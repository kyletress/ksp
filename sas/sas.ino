#include <krpc.h>
#include <krpc/services/krpc.h>
#include <krpc/services/space_center.h>

#define SAS_LED 3
#define SAS_BUTTON 2
#define HEADING_GREEN 4
#define HEADING_RED 5
#define NORMAL_GREEN 6
#define NORMAL_RED 7
#define STABILITY_ASSIST 9
#define NORMAL 8

boolean sasLastButton = LOW; // previous SAS button state
boolean sasCurrentButton = LOW; // current SAS button state
boolean sasState = false; // the present state of SAS
boolean stabilityAssistLastButton = LOW;
boolean stabilityAssistCurrentButton = LOW;
boolean normalLastButton = LOW;
boolean normalCurrentButton = LOW;

HardwareSerial *conn;
krpc_SpaceCenter_Control_t instance;
krpc_SpaceCenter_Vessel_t vessel;
krpc_SpaceCenter_Control_t control;

void blink_led(int count) {
  delay(1000);
  for (int i = 0; i < count; i++) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(200);
    digitalWrite(LED_BUILTIN, LOW);
    delay(200);
  }
  delay(1000);  
}

// Button debounce 
boolean debounce(int pin, boolean last) {
  boolean current = digitalRead(pin);
  if(last != current) {
    delay(5);
    current = digitalRead(pin);
  }
  return current;
}

// make the SAS row all red 
void disableSasLeds(){
  digitalWrite(HEADING_GREEN, LOW);
  digitalWrite(HEADING_RED, HIGH);
  digitalWrite(NORMAL_GREEN, LOW);
  digitalWrite(NORMAL_RED, HIGH);
}

void cutTheLights() {
  digitalWrite(HEADING_GREEN, LOW);
  digitalWrite(HEADING_RED, LOW);
  digitalWrite(NORMAL_GREEN, LOW);
  digitalWrite(NORMAL_RED, LOW);
}

void setup() {
  krpc_error_t error;

  pinMode(SAS_LED, OUTPUT);
  pinMode(SAS_BUTTON, INPUT_PULLUP);
  pinMode(HEADING_GREEN, OUTPUT);
  pinMode(HEADING_RED, OUTPUT);
  pinMode(NORMAL_GREEN, OUTPUT);
  pinMode(NORMAL_RED, OUTPUT);
  
  digitalWrite(SAS_LED, LOW);
  digitalWrite(HEADING_GREEN, LOW);
  digitalWrite(HEADING_RED, LOW);
  digitalWrite(NORMAL_GREEN, LOW);
  digitalWrite(NORMAL_RED, LOW);

  conn = &Serial;
  delay(1000);

  // set up server communication
  do {
    // Open the serial port connection
    error = krpc_open(&conn, NULL);
    if (error != KRPC_OK) {
      delay(100);
    }
  } while (error != KRPC_OK);

  // Set up communication with the server
  do {
    error = krpc_connect(conn, "Kyle's KSP Controller");
    if (error != KRPC_OK) {
      blink_led(-(int) error);
      delay(100);
    }
  } while (error != KRPC_OK);

  do {
    error = krpc_SpaceCenter_ActiveVessel(conn, &vessel);
    if (error != KRPC_OK) {
      delay(100);
    }
  } while (error != KRPC_OK);

  do {
    error = krpc_SpaceCenter_Vessel_Control(conn, &control, vessel);
    if (error != KRPC_OK) {
      delay(100);
      blink_led(- (int)error);
    }
  } while (error != KRPC_OK);  
}

void loop() {
  krpc_error_t error;

  do {
    error = krpc_SpaceCenter_Control_SAS(conn, &sasState, control);
    if (error != KRPC_OK) {
      delay(100);
    }
  } while (error != KRPC_OK);

  sasCurrentButton = debounce(SAS_BUTTON, sasLastButton);
  if (sasLastButton == HIGH && sasCurrentButton == LOW) { // button pressed
    sasState = !sasState; // toggle the sasState variable
  }

  sasLastButton = sasCurrentButton;
  krpc_SpaceCenter_Control_set_SAS(conn, control, sasState);
  digitalWrite(SAS_LED, sasState);
  
  // sas buttons NOT WORKING
  // stability assist 
  stabilityAssistCurrentButton = debounce(STABILITY_ASSIST, stabilityAssistLastButton);
  if (stabilityAssistLastButton == HIGH && stabilityAssistCurrentButton == LOW) { // button pressed
    krpc_SpaceCenter_Control_set_SASMode(conn, control, KRPC_SPACECENTER_SASMODE_STABILITYASSIST);
  }
  stabilityAssistLastButton = stabilityAssistCurrentButton;
 
  // normal 
  normalCurrentButton = debounce(NORMAL, normalLastButton);
  if (normalLastButton == HIGH && normalCurrentButton == LOW) { // button pressed
    krpc_SpaceCenter_Control_set_SASMode(conn, control, KRPC_SPACECENTER_SASMODE_NORMAL);
  }
  normalLastButton = normalCurrentButton;

 
  krpc_SpaceCenter_SASMode_t sasMode;
  krpc_SpaceCenter_Control_SASMode(conn, &sasMode, control);
  
  
  if (sasState) {
    switch (sasMode) {
    case KRPC_SPACECENTER_SASMODE_STABILITYASSIST:
      digitalWrite(HEADING_GREEN, HIGH);
      digitalWrite(HEADING_RED, LOW);
      digitalWrite(NORMAL_GREEN, LOW);
      digitalWrite(NORMAL_RED, HIGH);
      break;
     case KRPC_SPACECENTER_SASMODE_NORMAL:
      digitalWrite(NORMAL_GREEN, HIGH);
      digitalWrite(NORMAL_RED, LOW);
      digitalWrite(HEADING_GREEN, LOW);
      digitalWrite(HEADING_RED, HIGH);
      break;
    default:
      break;
    }
  } else {
    cutTheLights();
  }
}
