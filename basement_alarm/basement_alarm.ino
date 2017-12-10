const int PIN_ON_BTN = 2; 
const int PIN_OFF_BTN = 3;
const int PIN_LED_RED = 4;
const int PIN_LED_GREEN = 5;
const int PIN_ALARM = 6;
const int PIN_PIR_SENSOR = 7;

const int STATE_ENABLING_GUARD = 0;
const int STATE_GUARD = 1;
const int STATE_ENABLING_ALARM = 2;
const int STATE_ALARM = 3;
const int STATE_HOLD = 4;

//PROD VALUES

const int ALARM_OFF_LEVEL = 0;
const int ALARM_ON_LEVEL = 255;

const unsigned long ENABLING_GUARD_STATE_TIME_MS = 30*1000l; // 30 seconds
const unsigned long ENABLE_ALARM_STATE_TIME_MS = 30*1000l; // 30 seconds
const unsigned long HOLD_STATE_TIME_MS = 2*60*60*1000l; // 2 hours
const unsigned long ALARM_STATE_TIME_MS = 1*60*1000l; // 5 minutes
const boolean DEBUG = false;

//END PROD VALUES

//DEBUG VALUES
/*const int ALARM_OFF_LEVEL = 255;
const int ALARM_ON_LEVEL = 180;

const unsigned long ENABLING_GUARD_STATE_TIME_MS = 10*1000l; // 30 seconds
const unsigned long ENABLE_ALARM_STATE_TIME_MS = 5*1000l; // 30 seconds
const unsigned long HOLD_STATE_TIME_MS = 30*1000l; // 2 hours
const unsigned long ALARM_STATE_TIME_MS = 10*1000l; // 5 minutes
const boolean DEBUG = true;
*/
//END DEBUG VALUES

const unsigned long BLINK_PERIOD_TIME_MS = 500l;




int onBtnValue = 0;
int offBtnValue = 0;
int pirValue = 0;
int state = STATE_ENABLING_GUARD;
unsigned long enablingGuardStateStartTime = 0;
unsigned long enablingAlarmStateStartTime = 0;
unsigned long holdStateStartTime = 0;
unsigned long alarmStateStartTime = 0;
boolean redIsOn = false;
boolean greenIsOn = false;
unsigned long redChangedTime = 0;
unsigned long greenChangedTime = 0;




void setup() { 
  pinMode(PIN_ON_BTN, INPUT);
  pinMode(PIN_OFF_BTN, INPUT);
  pinMode(PIN_LED_RED, OUTPUT);
  pinMode(PIN_LED_GREEN, OUTPUT);
  pinMode(PIN_ALARM, OUTPUT);
  pinMode(PIN_PIR_SENSOR, INPUT);
  
  initAlarm();
  changeToEnablingGuardState();
  if(DEBUG) {
    Serial.begin(9600);
  }
}

void initAlarm() {
    // clean up noise
    alarmOn();
    delay(100);
    alarmOff();
    delay(100);
}

void changeToEnablingGuardState() {
  if(DEBUG) {
    Serial.print("Changed to ENABLING GUARD state\r\n");
  }

  LEDsOff();
  alarmOff();
  enablingGuardStateStartTime = millis();
  state = STATE_ENABLING_GUARD;
}

void processEnablingGuardState() {
  blinkRedLED();
  if(offBtnValue == 1) {
    changeToHoldState();
    return;
  }
  unsigned long delta = getTimeDelta(enablingGuardStateStartTime, millis());
  if(delta > ENABLING_GUARD_STATE_TIME_MS) {
    changeToGuardState();
    return;
  }
}

void changeToHoldState() {
  if(DEBUG) {
    Serial.print("Changed to HOLD state\r\n");
  }

  LEDsOff();
  alarmOff();
  holdStateStartTime = millis();
  state = STATE_HOLD;
}

void processHoldState() {
  greenLEDOn();
  if(offBtnValue == 1) {
    changeToHoldState(); // reenable hold
    return;
  }
  unsigned long delta = getTimeDelta(holdStateStartTime, millis());
  if(onBtnValue == 1 || delta > HOLD_STATE_TIME_MS) {
    changeToEnablingGuardState();
    return;
  }
}
  
void changeToGuardState() {
  if(DEBUG) {
    Serial.print("Changed to GUARD state\r\n");
  }

  LEDsOff();
  alarmOff();
  state = STATE_GUARD;
}

void processGuardState() {
  blinkGreenLED();
  if(offBtnValue == 1) {
    changeToHoldState(); // reenable hold
    return;
  }
  if(pirValue == 1) {
    changeToEnablingAlarmState(); // reenable alarm state
    return;
  }
}

void changeToAlarmState() {
  if(DEBUG) {
    Serial.print("Changed to ALARM state\r\n");
  }

  if(state != STATE_ALARM) {
    LEDsOff();
  }
  
  alarmOn();
  alarmStateStartTime = millis();
  state = STATE_ALARM;
}

void processAlarmState() {
  redLEDOn();
  if(offBtnValue == 1) {
    changeToHoldState();
    return;
  }
  alarmOn();
  if(pirValue == 1) {
    changeToAlarmState(); // reenable alarm state
    return;
  }
  unsigned long delta = getTimeDelta(alarmStateStartTime, millis());
  if(delta > ALARM_STATE_TIME_MS) {
    changeToGuardState();
    return;
  }
}

void changeToEnablingAlarmState() {
  if(DEBUG) {
    Serial.print("Changed to ENABLING ALARM state\r\n");
  }

  LEDsOff();
  alarmOff();
  enablingAlarmStateStartTime = millis();
  state = STATE_ENABLING_ALARM;
}

void processEnablingAlarmState() {
  blinkRedLED();
  if(offBtnValue == 1) {
    changeToHoldState();
    return;
  }
  unsigned long delta = getTimeDelta(enablingAlarmStateStartTime, millis());
  if(delta > ENABLE_ALARM_STATE_TIME_MS) {
    changeToAlarmState();
    return;
  }
}

void loop(){
//  for(int i=0;i<256;i++) {
//    analogWrite(PIN_ALARM, i);
//    delay(100);
//  }
//  returnl0
  onBtnValue = digitalRead(PIN_ON_BTN);
  offBtnValue = digitalRead(PIN_OFF_BTN);
  pirValue = digitalRead(PIN_PIR_SENSOR);
  //if(onBtnValue == 1) {
  //alarmOn();
  //}
  
  switch(state) {
     case STATE_ENABLING_GUARD: processEnablingGuardState();
           break;
     case STATE_GUARD: processGuardState();
           break;
     case STATE_ENABLING_ALARM: processEnablingAlarmState();
           break;     
     case STATE_ALARM: processAlarmState();
           break;
     case STATE_HOLD: processHoldState();
           break;
     default: processGuardState();
  }
  delay(50);

}

unsigned long getTimeDelta(unsigned long prevTime, unsigned long currentTime) {
  if(prevTime < currentTime) {
    unsigned long deltaToEnd = 0xfffffffful - prevTime;
    
    return currentTime + deltaToEnd;
  } else {
    return currentTime - prevTime;
  }
}

void LEDsOff() {
  digitalWrite(PIN_LED_RED, 0);
  digitalWrite(PIN_LED_GREEN, 0);
  redIsOn = false;
  greenIsOn = false;
  redChangedTime = millis();
  greenChangedTime = millis();
}

void redLEDOn() {
  digitalWrite(PIN_LED_RED, 1);
  redIsOn = true;
  redChangedTime = millis();
}

void greenLEDOn() {
  digitalWrite(PIN_LED_GREEN, 1);
  greenIsOn = true;
  greenChangedTime = millis();
}

void blinkRedLED() {
  unsigned long delta = getTimeDelta(redChangedTime, millis());
  if(delta > BLINK_PERIOD_TIME_MS) {
    if(redIsOn) {
      digitalWrite(PIN_LED_RED, 0);
    } else {
      digitalWrite(PIN_LED_RED, 1);
    }
    redIsOn = ~redIsOn;
    redChangedTime = millis();
  }
}

void blinkGreenLED() {
  unsigned long delta = getTimeDelta(greenChangedTime, millis());
  if(delta > BLINK_PERIOD_TIME_MS) {
    if(greenIsOn) {
      digitalWrite(PIN_LED_GREEN, 0);
    } else {
      digitalWrite(PIN_LED_GREEN, 1);
    }
    greenIsOn = ~greenIsOn;
    greenChangedTime = millis();
  }
}

void alarmOn() {
  analogWrite(PIN_ALARM, ALARM_ON_LEVEL);
}

void alarmOff() {
  analogWrite(PIN_ALARM, ALARM_OFF_LEVEL);
}

