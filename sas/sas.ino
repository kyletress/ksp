#include <krpc.h>
#include <krpc/services/krpc.h>
#include <krpc/services/space_center.h>

#define SAS_LED 3
#define SAS_BUTTON 2
#define HEADING_ENABLED_LED 4
#define HEADING_DISABLED_LED 5
//#define MANEUVER_ENABLED_LED 4
//#define MANEUVER_DISABLED_LED 5
//#define PROGRADE_ENABLED_LED 4
//#define PROGRADE_DISABLED_LED 5
//#define RETROGRADE_ENABLED_LED 4
//#define RETROGRADE_DISABLED_LED 5
#define TARGET_ENABLED_LED 6
#define TARGET_DISABLED_LED 7
#define ANTITARGET_ENABLED_LED 8
#define ANTITARGET_DISABLED_LED 9
#define RADIALIN_ENABLED_LED 10
#define RADIALIN_DISABLED_LED 11
#define RADIALOUT_ENABLED_LED 12
#define RADIALOUT_DISABLED_LED 13

boolean sasLastButton = LOW; // previous SAS button state
boolean sasCurrentButton = LOW; // current SAS button state
boolean sasState = false; // the present state of SAS

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
boolean debounce(boolean last) {
  boolean current = digitalRead(SAS_BUTTON);
  if(last != current) {
    delay(5);
    current = digitalRead(SAS_BUTTON);
  }
  return current;
}

// make the SAS row all red 
void disableSasLeds(){
  digitalWrite(HEADING_ENABLED_LED, LOW);
  digitalWrite(HEADING_DISABLED_LED, HIGH);
  digitalWrite(TARGET_ENABLED_LED, LOW);
  digitalWrite(TARGET_DISABLED_LED, HIGH);
  digitalWrite(ANTITARGET_ENABLED_LED, LOW);
  digitalWrite(ANTITARGET_DISABLED_LED, HIGH);
  digitalWrite(RADIALIN_ENABLED_LED, LOW);
  digitalWrite(RADIALIN_DISABLED_LED, HIGH);
  digitalWrite(RADIALOUT_ENABLED_LED, LOW);
  digitalWrite(RADIALOUT_DISABLED_LED, HIGH);
}

void cutTheLights() {
  digitalWrite(HEADING_ENABLED_LED, LOW);
  digitalWrite(HEADING_DISABLED_LED, LOW);
  digitalWrite(TARGET_ENABLED_LED, LOW);
  digitalWrite(TARGET_DISABLED_LED, LOW);
  digitalWrite(ANTITARGET_ENABLED_LED, LOW);
  digitalWrite(ANTITARGET_DISABLED_LED, LOW);
  digitalWrite(RADIALIN_ENABLED_LED, LOW);
  digitalWrite(RADIALIN_DISABLED_LED, LOW);
  digitalWrite(RADIALOUT_ENABLED_LED, LOW);
  digitalWrite(RADIALOUT_DISABLED_LED, LOW);
}

void setup() {
  krpc_error_t error;

  pinMode(SAS_LED, OUTPUT);
  pinMode(SAS_BUTTON, INPUT_PULLUP);
  pinMode(HEADING_ENABLED_LED, OUTPUT);
  pinMode(HEADING_DISABLED_LED, OUTPUT);
  
  digitalWrite(SAS_LED, LOW);
  digitalWrite(HEADING_ENABLED_LED, LOW);
  digitalWrite(HEADING_DISABLED_LED, LOW);

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

  sasCurrentButton = debounce(sasLastButton);
  if (sasLastButton == HIGH && sasCurrentButton == LOW) { // button pressed
    sasState = !sasState; // toggle the sasState variable
  }

  if (sasLastButton == LOW && sasCurrentButton == HIGH) {
    cutTheLights();
  }
  
  sasLastButton = sasCurrentButton;
  krpc_SpaceCenter_Control_set_SAS(conn, control, sasState);
  digitalWrite(SAS_LED, sasState);
  
  krpc_SpaceCenter_SASMode_t sasMode;
  krpc_SpaceCenter_Control_SASMode(conn, &sasMode, control);
  
  if (sasState) {
    switch (sasMode) {
    case KRPC_SPACECENTER_SASMODE_STABILITYASSIST:
      disableSasLeds();
      digitalWrite(HEADING_ENABLED_LED, HIGH);
      digitalWrite(HEADING_DISABLED_LED, LOW);
      break;
     case KRPC_SPACECENTER_SASMODE_RADIAL:
      disableSasLeds();
      digitalWrite(RADIALIN_ENABLED_LED, HIGH);
      digitalWrite(RADIALIN_DISABLED_LED, LOW);
      break;
     case KRPC_SPACECENTER_SASMODE_ANTIRADIAL:
      disableSasLeds();
      digitalWrite(RADIALOUT_ENABLED_LED, HIGH);
      digitalWrite(RADIALOUT_DISABLED_LED, LOW);
      break;
     case KRPC_SPACECENTER_SASMODE_TARGET:
      disableSasLeds();
      digitalWrite(TARGET_ENABLED_LED, HIGH);
      digitalWrite(TARGET_DISABLED_LED, LOW);
      break;
     case KRPC_SPACECENTER_SASMODE_ANTITARGET:
      disableSasLeds();
      digitalWrite(ANTITARGET_ENABLED_LED, HIGH);
      digitalWrite(ANTITARGET_DISABLED_LED, LOW);
      break;
    default:
      break;
    }
  }
}
