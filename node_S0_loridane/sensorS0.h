//sensor and sensor variables init here

#define WH_PER_IMP 1.25
#define SENSPIN 12

unsigned int S0pulses = 0; // is long in case we use very long sending intervals (up to 49 days possible)
unsigned long int lastIRtime = 0;
bool attachedIR = false; //is interrupt attached?
unsigned long int IRinterval = 1000000000; //how long since the last interrupt?
float watthours = 0;
float wattsmean = 0;
bool newreading = false; //has there been an interrupt lately?

//what to do at interrupt
void IRAM_ATTR ISR() {
  IRinterval = millis() - lastIRtime;
  lastIRtime = millis();
  attachedIR = false;
  newreading = true;
  S0pulses++;
}


// executed in main setup()
bool sensorSetup() {
  pinMode(SENSPIN, INPUT_PULLDOWN);
  attachedIR = true;
  return true;
}

//executed to get the sensor values
float sensorReadWatthours() {
  float reading = S0pulses * WH_PER_IMP; //1.25 Wh per impulse
  return reading;
}
float sensorReadWatts() {
  float reading;
  if(IRinterval != 1000000000){
    reading = WH_PER_IMP * 3600000 / IRinterval;
  }else{
    reading = 0;
  }
  return reading;
}

//executed after the node has sent the sensorvalue
void sensorAfterSent() {
  S0pulses = 0;
  watthours = 0;
  wattsmean = 0;
}
