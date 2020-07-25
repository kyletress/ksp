#include <krpc.h>
#include <krpc/services/krpc.h>
#include <krpc/services/space_center.h>

#define SAS_LED 3
#define SAS_BUTTON 2

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

void setup() {
  krpc_error_t error;

  pinMode(SAS_LED, OUTPUT);
  digitalWrite(SAS_LED, LOW);

  pinMode(SAS_BUTTON, INPUT_PULLUP);

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
  sasLastButton = sasCurrentButton;
  krpc_SpaceCenter_Control_set_SAS(conn, control, sasState);
  digitalWrite(SAS_LED, sasState);
}
