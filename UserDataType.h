#ifndef UserDataType_h
#define UserDataType_h



//Interval between data records in microseconds. Cela definie la frÃ©quence d'aquisition. 
// 2000 microssecond pour 500 Hz
// 1000 pour 1Khz
// 500 pour 2 Khz

const uint8_t ADC_DIM = 2; // Nombre de voies Ã  lire
 // Used to set the frequency aquisition in SetFrequency. It will mutliplicate the next value
uint32_t LOG_INTERVAL_USEC = 1000; 

struct data_t {
  unsigned long time;
  unsigned short adc[2]; // On ne lit que 2 valeurs d'adc
  unsigned short hallSensor;
};

#endif  // UserDataType_h
