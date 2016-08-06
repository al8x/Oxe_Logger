//-------------------------------------------------------TIME

//Something add to setup
    // set the Time library to use Teensy 3.0's RTC to keep time
    // setSyncProvider(getTeensy3Time);

void printDigits(int digits){
  // utility function for digital clock display: prints preceding colon and leading 0
  display.print(":");
  if(digits < 10)
    display.print('0');
  display.print(digits);
}

void digitalClockDisplay() {
  // digital clock display of the time
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.print(hour());
  printDigits(minute());
  printDigits(second());
  display.print(" ");
  display.print(day());
  display.print(" ");
  display.print(month());
  display.print(" ");
  display.print(year()); 
  display.println();
  display.setCursor(0,0);
  display.display();
  display.clearDisplay(); 
}

time_t getTeensy3Time()
{
  return Teensy3Clock.get();
}
