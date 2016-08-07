
//**************************************************************************************
//------------------------------- DISPLAY FUNCTIONS ------------------------------------
//**************************************************************************************
void PasDeCarteSD(){
  //Si pas de carte SD, on ne fait rien
  // Ecran de pret à mesurer
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("");
    display.println("   NO SD");
    display.setTextSize(1);
    display.println("Please, shutdown the logger and insert or verify your sd card");
    display.println("");
    display.println("");
    display.setCursor(0,0);
    display.display();
    display.clearDisplay();  
  }


void ScreenChooseFrequency(){
  // Ecran ou on règle la fréquence d'aquisition
  // Ecran de pret à mesurer
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("  Frequency settings ");
    display.setTextSize(2);
    display.println("");
    display.print("  ");
    display.print(1000000/LOG_INTERVAL_USEC);
    display.print(" Hz");
    display.println("");

    
    display.drawLine(0, 50, 120, 50, WHITE);
    
    display.setTextSize(1);
    display.println("");
    display.println("");
    
    display.print("Less     OK     More");
    
    display.setCursor(0,0);
    display.display();
    display.clearDisplay();
  
}

void SentenceOnScreen(char Sentence[]){
  // Show for one second a sentence that need to be displayed
   display.setTextSize(1);
   display.setTextColor(WHITE);
   display.println(Sentence);
   display.setCursor(0,0);
   display.display();
   display.clearDisplay();
   delay(1000);
}
void Aquisition(){
  // Ecran de pret à mesurer
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("");
    display.println("  LOGGING");
    display.setTextSize(1);
    display.println("    Press to stop");
    display.println("");
    display.println("");
    display.drawLine(0, 50, 120, 50, WHITE);
    display.print(" *        *     Stop ");
    display.setCursor(0,0);
    display.display();
    display.clearDisplay();
}

void HomeScreen(){
  // Screen data and help to calibrate the sensors
  int temporarValue= readADC(0);
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print("   Voie 1   ");
  display.println(temporarValue);
  display.println("");
  display.println("");
  display.fillRect(0, 12, ((temporarValue*128)/4096), 6, WHITE);
  
  if (temporarValue>2048){
    display.drawLine(64, 12, 64, 17, BLACK);
  }
  else{
    display.drawLine(64, 12, 64, 17, WHITE);
  }

  //--------------First part of the screen
  
  temporarValue= readADC(1);
  display.fillRect(0, 22, ((temporarValue*128)/4096), 6, WHITE);
  
  if (temporarValue>2048){
    display.drawLine(64, 22, 64, 27, BLACK);
  }
  else{
    display.drawLine(64, 22, 64, 27, WHITE);
  }
  
  display.println("");
  display.print("   Voie 2   ");
  display.println(temporarValue);
  display.println("");
  display.println("");
  display.print("Back      *     Start");
 
  display.setCursor(0,0);
  display.drawLine(0, 50, 128, 50, WHITE);
  display.display();
  display.clearDisplay();
}
void OnEstPret(){
  // Ecran de pret à mesurer
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.println("   OXELO ");
    display.println("DATALOGGER");
    
    display.drawLine(0, 50, 120, 50, WHITE);
    display.setTextSize(1);
    display.println("");
    display.print("FAT");
    display.print(int(sd.vol()->fatType()));
    
    display.print("  ");
    uint32_t volFree = sd.vol()->freeClusterCount();
    float fs = 0.000000512*volFree*sd.vol()->blocksPerCluster();
    display.print(fs);
    display.print("/");
    display.print(sd.card()->cardSize()*512E-9);
    display.print(" GB");
    display.println("");
    display.println("");
    
    display.print("Format   Set     OK");
    
    display.setCursor(0,0);
    display.display();
    display.clearDisplay();
}

void ErrorOnScreen (const __FlashStringHelper* msg){
  // Ecran de pret à mesurer
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("");
    display.println(msg);
    display.setCursor(0,0);
    display.display();
    display.clearDisplay();
}

void DisplayProgressBinToCsv(char CsvName[], int percentageDone){
   display.setTextSize(2);
   display.setTextColor(WHITE);
   display.println("Saving as");
   display.println(CsvName);
   display.print("   ");
   display.print(percentageDone);
   display.println('%');
   display.setCursor(0,0);
   display.display();
   display.clearDisplay();
}

void ShowSavingStats(char binName[],int maxLatency,int t1, int t0,int count,int overrunTotal,int Errors){
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(F("Saved as: "));
  display.println(binName);
  //display.print(F("Max block write usec: "));
  //display.println(maxLatency);
  display.print(F("Record time:"));
  display.println(0.001*(t1 - t0), 3);
  display.print(F("Sample count: "));
  display.println(count);
  display.print(F("Samples/sec: "));
  display.println((1000.0)*count/(t1-t0));
  display.print(F("Overruns: "));
  display.println(overrunTotal);
  
  if (!Errors){display.println("No errors found");}
  else{display.println("Done");};
  display.println("");
  display.print(" *        *        OK");
 
  display.setCursor(0,0);
  display.drawLine(0, 50, 128, 50, WHITE);
  display.display();
  display.clearDisplay();
}


 //TO DO !!!:
void DataAquariedPreview() {
  //Affcieh un 
  
  int dataToScreen[128]; // Tableau contenant les donnés affichés sur l'écran de 128 pixels de largeur
  long tempsZero;
  int VoieAafficher=0;
  long Temps;
  
  block_t block;
  if (!binFile.isOpen()) {
    display.println();
    display.println(F("No current binary file"));
    return;
  }
  binFile.rewind();
  delay(1000);
  while (binFile.read(&block , 512) == 512) {
    if (block.count == 0) {
      break;
    }
    if (block.overrun) {
      display.print(F("OVERRUN,"));
    }
    tempsZero=((&block.data[0])->time);
    for (uint16_t i = 0; i < block.count; i++) { // Lecture de la valeur suivante
      
      for (uint16_t j = 0; j < 127; j++){ //décalage de toute les valeurs du tableau vers la gauche
        dataToScreen[j]=dataToScreen[j+1];
      }
      dataToScreen[127]=((&block.data[i])->adc[VoieAafficher]);// On met la valeur suisvante dans la derniere position du tableau
      
      Temps=(((&block.data[i])->time)-tempsZero);
      
      AfficheGraph(dataToScreen,Temps);
    }
  }
}
void AfficheGraph(int dataToScreen[],int temps){
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.print(temps/1000);
  for (uint16_t j = 0; j < 128; j++){ 
    display.drawPixel(j, dataToScreen[j]*64/4096, WHITE);
  }
  
  display.setCursor(0,0);
  display.drawLine(0, 32, 128, 32, WHITE);
  display.display();
  display.clearDisplay();
}

