int readADC(int channel) {
  byte adcPrimaryRegister = 0b00000110;      // Sets default Primary ADC Address register B00000110, This is a default address setting, the third LSB is set to one to start the ADC, the second LSB is to set the ADC to single ended mode, the LSB is for D2 address bit, for this ADC its a "Don't Care" bit.
  byte adcPrimaryRegisterMask = 0b00000111;  // b00000111 Isolates the three LSB.  
  byte adcPrimaryByteMask = 0b00001111;      // b00001111 isolates the 4 LSB for the value returned. 
  byte adcPrimaryConfig = adcPrimaryRegister & adcPrimaryRegisterMask; // ensures the adc register is limited to the mask and assembles the configuration byte to send to ADC.
  byte adcSecondaryConfig = channel << 6;
  noInterrupts(); // disable interupts to prepare to send address data to the ADC.

  digitalWrite(SD_CS_PIN,HIGH);
  digitalWrite(CS_3204,HIGH); // take the Chip Select pin low to select the ADC.
  digitalWrite(CS_3204,LOW); // take the Chip Select pin low to select the ADC.
  SPI.setDataMode(SPI_MODE1); //Obligé pour que sa marche bien ! NE PAS OUBLIER DE changer la valeur dans le fichier SdFatConfig #define SD_SPI_CONFIGURATION 1

  
  SPI.transfer(adcPrimaryConfig); //  send in the primary configuration address byte to the ADC.  
  byte adcPrimaryByte = SPI.transfer(adcSecondaryConfig); // read the primary byte, also sending in the secondary address byte.  
  byte adcSecondaryByte = SPI.transfer(0x00); // read the secondary byte, also sending 0 as this doesn't matter.
   
  digitalWrite(CS_3204,HIGH); // take the Chip Select pin high to de-select the ADC.
  digitalWrite(SD_CS_PIN,HIGH);
  digitalWrite(SD_CS_PIN,LOW);
   
  interrupts(); // Enable interupts.
  adcPrimaryByte &= adcPrimaryByteMask; // Limits the value of the primary byte to the 4 LSB:
  int digitalValue = (adcPrimaryByte << 8) | adcSecondaryByte; // Shifts the 4 LSB of the primary byte to become the 4 MSB of the 12 bit digital value, this is then ORed to the secondary byte value that holds the 8 LSB of the digital value.
  //float value = (float(digitalValue) * 2.048) / 4096.000; // The digital value is converted to an analogue voltage using a VREF of 2.048V.
  return digitalValue; // Returns the value from the function
}

int Read3202(int CHANNEL){
  int msb;
  int lsb;
    int commandBytes = B10100000 ;              // channel 0      
    if (CHANNEL==1) commandBytes = B11100000 ;  // channel 1 

    digitalWrite(SD_CS_PIN,HIGH);
    digitalWrite(CS_3204,HIGH); // take the Chip Select pin low to select the ADC.
    digitalWrite(CS_3204,LOW); // take the Chip Select pin low to select the ADC.
    
    SPI.setDataMode(SPI_MODE1); //Obligé pour que sa marche bien ! NE PAS OUBLIER DE changer la valeur dans le fichier SdFatConfig #define SD_SPI_CONFIGURATION 1
    
    SPI.transfer (B00000001);                   // Start bit
    msb = SPI.transfer(commandBytes) ;
    msb = msb & B00001111;
    lsb = SPI.transfer(0x00);

    digitalWrite(CS_3204,HIGH); // take the Chip Select pin high to de-select the ADC.
    digitalWrite(SD_CS_PIN,HIGH);
    digitalWrite(SD_CS_PIN,LOW);
    
  return ((int) msb) <<8 | lsb;
}
