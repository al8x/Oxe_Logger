//**************************************************************************************
//------------------------------- SD SAVING FUNCTIONS ----------------------------------
//**************************************************************************************
// Conatain function :
//      - logData
//      - CheckOverrun
//      - binaryToCsv


//==============================================================================
// Acquire a data record.
void acquireData(data_t* data) {
  data->time = micros();
  for (int i = 0; i < ADC_DIM; i++) {
    data->adc[i] = readADC(i);
  }
}

// Print a data record.
void printData(Print* pr, data_t* data) {
  pr->print(data->time);
  for (int i = 0; i < ADC_DIM; i++) {
    pr->write(',');
    pr->print(data->adc[i]);
  }
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {
  pr->print(F("time"));
  for (int i = 0; i < ADC_DIM; i++) {
    pr->print(F(",adc"));
    pr->print(i);
  }
  pr->println();
}

//---------------------------------------------------------

// Convert binary file to csv file.
void binaryToCsv() {
  uint8_t lastPct = 0;
  block_t block;
  uint32_t t0 = millis();
  uint32_t syncCluster = 0;
  SdFile csvFile;
  char csvName[13];

  if (!binFile.isOpen()) {
    SentenceOnScreen("Error in binToCSV");
    return;
  }
  binFile.rewind();
  // Create a new csvFile.
  strcpy(csvName, binName);
  strcpy(&csvName[BASE_NAME_SIZE + 3], "csv");

  if (!csvFile.open(csvName, O_WRITE | O_CREAT | O_TRUNC)) {
    error("open csvFile failed");
  }
  
  printHeader(&csvFile);
  uint32_t tPct = millis();
  while (binFile.read(&block, 512) == 512) {
    uint16_t i;
    if (block.count == 0) {
      break;
    }
    if (block.overrun) {
      csvFile.print(F("OVERRUN,"));
      csvFile.println(block.overrun);
    }
    for (i = 0; i < block.count; i++) {
      printData(&csvFile, &block.data[i]);
    }
    if (csvFile.curCluster() != syncCluster) {
      csvFile.sync();
      syncCluster = csvFile.curCluster();
    }
    if ((millis() - tPct) > 300) {
      uint8_t pct = binFile.curPosition()/(binFile.fileSize()/100);
      if (pct != lastPct) {
        tPct = millis();
        lastPct = pct;
        DisplayProgressBinToCsv(csvName, pct);
      }
    }
  }
  csvFile.close();
}


//------------------------------------------------------------------------------
// read data file and check for overruns

int checkOverrun() {
  bool headerPrinted = false;
  block_t block;
  uint32_t bgnBlock, endBlock;
  uint32_t bn = 0;

  if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
    error("contiguousRange failed");
  }
  binFile.rewind();

  while (binFile.read(&block, 512) == 512) {
    if (block.count == 0) {
      break;
    }
    if (block.overrun) {
      if (!headerPrinted) {
        //Serial.println();
        //Serial.println(F("Overruns:"));
        //Serial.println(F("fileBlockNumber,sdBlockNumber,overrunCount"));
        headerPrinted = true;
      }
    }
    bn++;
  }
  if (!headerPrinted) {
    return false;
  } else {
    return true;
  }
}

//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
//__________________________________________________________________________  LOGDATA


void logData() {
  uint32_t bgnBlock, endBlock;

  // Allocate extra buffer space.
  block_t block[BUFFER_BLOCK_COUNT];
  block_t* curBlock = 0;
  
  // Find unused file name.
  if (BASE_NAME_SIZE > 6) {
    error("FILE_BASE_NAME too long");
  }
  while (sd.exists(binName)) {
    if (binName[BASE_NAME_SIZE + 1] != '9') {
      binName[BASE_NAME_SIZE + 1]++;
    } else {
      binName[BASE_NAME_SIZE + 1] = '0';
      if (binName[BASE_NAME_SIZE] == '9') {
        error("Can't create file name");
      }
      binName[BASE_NAME_SIZE]++;
    }
  }
  // Delete old tmp file.
  if (sd.exists(TMP_FILE_NAME)) {
    display.println("Deleting Tmp Files");
    display.display();
    if (!sd.remove(TMP_FILE_NAME)) {
      error("Can't remove tmp file");
    }
  }
  // Create new file
  display.println("Creating new file");
  display.display();
  binFile.close();
  
  if (!binFile.createContiguous(sd.vwd(),
                                TMP_FILE_NAME, 512 * FILE_BLOCK_COUNT)) {
    error("createContiguous failed");
  }
  // Get the address of the file on the SD.
  if (!binFile.contiguousRange(&bgnBlock, &endBlock)) {
    error("contiguousRange failed");
  }
  // Use SdFat's internal buffer.
  uint8_t* cache = (uint8_t*)sd.vol()->cacheClear();
  if (cache == 0) {
    error("cacheClear failed");
  }

  // Flash erase all data in the file.
  display.println("Erasing all data");
  display.display();
  uint32_t bgnErase = bgnBlock;
  uint32_t endErase;
  while (bgnErase < endBlock) {
    endErase = bgnErase + ERASE_SIZE;
    if (endErase > endBlock) {
      endErase = endBlock;
    }
    if (!sd.card()->erase(bgnErase, endErase)) {
      error("erase failed");
    }
    bgnErase = endErase + 1;
  }
  // Start a multiple block write.
  if (!sd.card()->writeStart(bgnBlock, FILE_BLOCK_COUNT)) {
    error("writeBegin failed");
  }
  // Initialize queues.
  emptyHead = emptyTail = 0;
  fullHead = fullTail = 0;

  // Use SdFat buffer for one block.
  emptyQueue[emptyHead] = (block_t*)cache;
  emptyHead = queueNext(emptyHead);

  // Put rest of buffers in the empty queue.
  for (uint8_t i = 0; i < BUFFER_BLOCK_COUNT; i++) {
    emptyQueue[emptyHead] = &block[i];
    emptyHead = queueNext(emptyHead);
  }
  
  display.clearDisplay();
  display.display();
  display.setCursor(0,0); //We put back the origin of the screen to the top left and corner
  Aquisition(); //Show the aquisition screen sying that we are logging
  
  delay(10);
  uint32_t bn = 0;
  uint32_t t0 = millis();
  uint32_t t1 = t0;
  uint32_t overrun = 0;
  uint32_t overrunTotal = 0;
  uint32_t count = 0;
  uint32_t maxLatency = 0;
  int32_t diff;
  // Start at a multiple of interval.
  uint32_t logTime = micros()/LOG_INTERVAL_USEC + 1;
  logTime *= LOG_INTERVAL_USEC;
  bool closeFile = false;
  while (1) {
    // Time for next data record.
    logTime += LOG_INTERVAL_USEC;
    if (rightButtonPressed) { //Serial.available()
      rightButtonPressed=false;
      closeFile = true;
    }

    if (closeFile) {
      if (curBlock != 0 && curBlock->count >= 0) {
        // Put buffer in full queue.
        fullQueue[fullHead] = curBlock;
        fullHead = queueNext(fullHead);
        curBlock = 0;
      }
    } else {
      if (curBlock == 0 && emptyTail != emptyHead) {
        curBlock = emptyQueue[emptyTail];
        emptyTail = queueNext(emptyTail);
        curBlock->count = 0;
        curBlock->overrun = overrun;
        overrun = 0;
      }
      do {
        diff = logTime - micros();
      } while(diff > 0);
      if (diff < -10) {
        error("LOG_INTERVAL_USEC too small");
      }
      if (curBlock == 0) {
        overrun++;
      } else {
        acquireData(&curBlock->data[curBlock->count++]);
        magnetDetected = 0; // After we saved everything, we set back the value of detected magnet to 0 to record next passage of magnet
        
        if (curBlock->count == DATA_DIM) {
          fullQueue[fullHead] = curBlock;
          fullHead = queueNext(fullHead);
          curBlock = 0;
        }
      }
    }

    if (fullHead == fullTail) {
      // Exit loop if done.
      if (closeFile) {
        break;
      }
    } else if (!sd.card()->isBusy()) {
      // Get address of block to write.
      block_t* pBlock = fullQueue[fullTail];
      fullTail = queueNext(fullTail);
      // Write block to SD.
      uint32_t usec = micros();
      if (!sd.card()->writeData((uint8_t*)pBlock)) {
        error("write data failed");
      }
      usec = micros() - usec;
      t1 = millis();
      if (usec > maxLatency) {
        maxLatency = usec;
      }
      count += pBlock->count;

      // Add overruns and possibly light LED.
      if (pBlock->overrun) {
        overrunTotal += pBlock->overrun;
        if (ERROR_LED_PIN >= 0) {
          digitalWrite(ERROR_LED_PIN, HIGH);
        }
      }
      // Move block to empty queue.
      emptyQueue[emptyHead] = pBlock;
      emptyHead = queueNext(emptyHead);
      bn++;
      if (bn == FILE_BLOCK_COUNT) {
        // File full so stop
        break;
      }
    }
  }
  if (!sd.card()->writeStop()) {
    error("writeStop failed");
  }
  
  // Truncate file if recording stopped early.
  if (bn != FILE_BLOCK_COUNT) {
    //SentenceOnScreen("Truncating file");
    if (!binFile.truncate(512L * bn)) {
      //error("Can't truncate file");
    }
  }
  
  if (!binFile.rename(sd.vwd(), binName)) {
    error("Can't rename file");
  }

  //------------------End routine of the programm 
   int Errors=checkOverrun();
   DataAquariedPreview();
   binaryToCsv();            // converting the file recored to CSV file
   rightButtonPressed=false;
    
    while(!rightButtonPressed){ // We show the data until the user skip the screen
      ShowSavingStats(binName,maxLatency,t1,t0,count,overrunTotal,Errors);
      } //DisplayOn screen stats of the results
      delay(100);
      rightButtonPressed=false;  // After each button action, we put it back to its original false position
}

//------------------------------------------------------------------------------
