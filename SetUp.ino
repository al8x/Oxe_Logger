void setFrequency(){
  
  while (!middleButtonPressed){
    delay(10);
    ScreenChooseFrequency();
    
    if (leftButtonPressed){
      
      LOG_INTERVAL_USEC=(LOG_INTERVAL_USEC*2);
      leftButtonPressed=false;
      //We set the minimum 
      if (LOG_INTERVAL_USEC>32000){
        LOG_INTERVAL_USEC=32000;
      }

    }
    
    else if (rightButtonPressed){
      LOG_INTERVAL_USEC=(LOG_INTERVAL_USEC/2);
      rightButtonPressed=false;
      
      if (LOG_INTERVAL_USEC<1000){
        LOG_INTERVAL_USEC=1000;
      }
    }
  }
  // at the end, we set the final frequency by changing the value
  middleButtonPressed=false;
}

