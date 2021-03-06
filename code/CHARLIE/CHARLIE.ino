/*
  MR14 GAMMA
  Organization: MuTrac (McGill ASABE Tractor Pull Team), McGill University
  Author: Trevor Stanhope
  Date: 2013-05-04
  Summary: Beta Project for MR14
  - STEERING
  - DYNAMIC BALLAST
  - NO TEMPERATURE
  - KILLSWITCHES AND LOCKSWITCHES
  - SCREEN DISPLAYS ENGINE, WHEEL, FUEL AND BALLAST
  
  Contents:
  - Boot
    - Libraries
    - Declarations
    - Setup
  - Runtime
    - Loop
    - States
    - Functions
*/

/////////////// BOOT ///////////////
/* --- Libraries --- */
#include <SoftwareSerial.h> // needed for RFID module.
#include <Wire.h> // needed for LCD
#include <LiquidCrystal_I2C.h> // needed for LCD
#include "config.h" // needed for pin setup and global values
#include <DualVNH5019MotorShield.h> // needed for motors

/* --- Declarations --- */
// Spawn serial communication objects.
LiquidCrystal_I2C lcd(LCD_I2C_PORT, LCD_WIDTH, LCD_HEIGHT); // control object for LCD
DualVNH5019MotorShield motors; // M1 is steering actuator, M2 is ballast motor
// Create switch booleans (Locks, Kills, Triggers and Limits).
int KILL_HITCH = LOW;
int KILL_BUTTON = LOW;
int KILL_SEAT = LOW;
int LOCK_LEFTBRAKE = LOW;
int LOCK_RIGHTBRAKE = LOW;
int LOCK_CVT = LOW;
int LIMIT_FAR = LOW;
int LIMIT_NEAR = LOW;
int IGNITION = LOW;
// Create ballast and steering values.
int BALLAST_SPEED = 0;
volatile int STEERING_POSITION = 0;
volatile int ACTUATOR_POSITION = 0;
int ACTUATOR_FAULT = 0;
int MOTOR_FAULT = 0;
// Create sensor reading values and motor fault booleans.
volatile unsigned int SENSOR_FUEL = 0;
volatile unsigned int SENSOR_ENGINE = 0;
volatile unsigned int SENSOR_WHEEL = 0;
volatile unsigned int RATE_FUEL = 0;
volatile unsigned int RATE_ENGINE = 0;
volatile unsigned int RATE_WHEEL = 0;
// Create character buffers for LCD.
char BUFFER_FUEL[128];
char BUFFER_WHEEL[128];
char BUFFER_ENGINE[128];
char BUFFER_BALLAST[128];
// List of valid RFID keys.
int ANTOINE[] = {139, 151, 226, 117};
int TREVOR[] = {10, 227, 165, 221};

/* --- Setup --- */
// Begin MR14 initialization actions.
void setup() {
  /* --- Serial --- */
  Serial2.begin(BAUD); // RFID
  lcd.init();
  /* --- Initialize Ballast and Steering --- */
  pinMode(STEERING_A_PIN, INPUT);
  pinMode(STEERING_B_PIN, INPUT);
  digitalWrite(STEERING_A_PIN, HIGH); // turn on pullup resistor
  digitalWrite(STEERING_B_PIN, HIGH); // turn on pullup resistor
  motors.setM1Speed(0);
  motors.setM2Speed(0);
  /* --- Relays --- */
  pinMode(RELAY_1_PIN, OUTPUT);
  pinMode(RELAY_2_PIN, OUTPUT);
  pinMode(RELAY_3_PIN, OUTPUT);
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(RELAY_3_PIN, HIGH);
  /* --- Power Pins --- */
  pinMode(KILL_BUTTON_POWER, OUTPUT);
  digitalWrite(KILL_BUTTON_POWER, LOW);
  pinMode(KILL_SEAT_POWER, OUTPUT);
  digitalWrite(KILL_SEAT_POWER, LOW);
  pinMode(KILL_HITCH_POWER, OUTPUT);
  digitalWrite(KILL_HITCH_POWER, LOW);
  pinMode(LOCK_LEFTBRAKE_POWER, OUTPUT);
  digitalWrite(LOCK_LEFTBRAKE_POWER, LOW);
  pinMode(LOCK_RIGHTBRAKE_POWER, OUTPUT);
  digitalWrite(LOCK_RIGHTBRAKE_POWER, LOW);
  pinMode(IGNITION_POWER, OUTPUT);
  digitalWrite(IGNITION_POWER, LOW);
  /* --- Input Pins --- */
  pinMode(KILL_BUTTON_PIN, INPUT);
  digitalWrite(KILL_BUTTON_PIN, HIGH);
  pinMode(KILL_SEAT_PIN, INPUT);
  digitalWrite(KILL_SEAT_PIN, HIGH);
  pinMode(KILL_HITCH_PIN, INPUT);
  digitalWrite(KILL_HITCH_PIN, HIGH);
  pinMode(LOCK_LEFTBRAKE_PIN, INPUT);
  digitalWrite(LOCK_LEFTBRAKE_PIN, HIGH);
  pinMode(LOCK_RIGHTBRAKE_PIN, INPUT);
  digitalWrite(LOCK_RIGHTBRAKE_PIN, HIGH);
  pinMode(IGNITION_PIN, INPUT);
  digitalWrite(IGNITION_PIN, HIGH);
  /* --- Initialize Analog Pins --- */
  pinMode(ACTUATOR_FAULT_PIN, INPUT); 
  pinMode(MOTOR_FAULT_PIN, INPUT);
  pinMode(LOCK_CVT_PIN, INPUT);
  pinMode(BALLAST_SPEED_PIN, INPUT);
  /* --- Interrupt Pins --- */
  pinMode(SENSOR_FUEL_PIN, INPUT);
  pinMode(SENSOR_ENGINE_PIN, INPUT);
  pinMode(SENSOR_WHEEL_PIN, INPUT);
  /* --- Boot message --- */
  lcd.backlight();
  lcd.print("BOOTING...");
  delay(SHORT);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("MR14");
  lcd.setCursor(0,1);
  lcd.print("MCGILL UNIVERSITY");
  lcd.setCursor(0,2);
  lcd.print("BIORESOURCE ENG");
  delay(LONG);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("READ OWNERS MANUAL");
  lcd.setCursor(0,1);
  lcd.print("BEFORE OPERATION");
  delay(LONG);
}

/////////////// RUNTIME ///////////////
/*********** LOOP ***********/
/* --- Loop --- */
void loop() {
  interrupts();
  // Attempt to enter ON mode.
  on();
  // Disable all tractor functions then wait for reboot.
  off();
  // Wait for restart
  delay(MEDIUM);
}
/*********** STATES ***********/
/*
  Operation states for the MR14:
    on()
    standby()
    ignition()
    run()
    off()
*/
/* --- On --- */
// LOOP() --> ON()
// Tractor is on and scans continously for key.
void on() {
  // Display ON message.
  lcd.clear();
  lcd.print("ON");
  Serial.println("ON");
  delay(SHORT);
  //  While no Kills, contine to prompt for RFID key.
  while (nokill()) {
    // Display prompt message then test RFID serial.
    lcd.clear();
    lcd.print("SWIPE KEY CARD");
    Serial.println("SWIPE KEY CARD");
    delay(SHORT);
    Serial2.write(0x02);
    delay(SHORT);
    if (Serial2.available()) {
      // If key is good, enter STANDY mode.
      if (testKey()) {
        standby();
      }
      // Else if key is bad, display error message.
      else {
        lcd.clear();
        lcd.print("INCORRECT KEY");
        Serial.println("INCORRECT KEY");
        delay(SHORT);
      }
    }
  }
}

/* --- Standby --- */
// ON() && TESTKEY() && KILL() --> STANDY()
void standby() {
  // Enable Regulator and Fuel Solenoid and display STANDY message.
  lcd.clear();
  lcd.print("STANDBY");
  Serial.println("STANDBY");
  delay(SHORT);
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, HIGH);
  // If no Kills or Locks enabled, attempt ignition.
  while (nokill()) {
    while (nolock()) {
      if (start()) { // activate ignition if start button engaged
        ignition();
        run();
      }
    }
  }
}

/* --- Ignition --- */
// STANDBY() && START() && KILL() && LOCK() --> IGNITION()
void ignition() {
  // Enable Starter, disable Fuel Solenoid and display ignition message.
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, LOW);
  // Prepare LCD
  lcd.clear();
  lcd.print("IGNITION");
  // While ignition button is enaged, attempt ignition.
  while (start()) {
    delay(SHORT);
  }
}

/* --- Run --- */
// STANDBY() && START() && KILL() && LOCK() --> RUN()
void run() {
  // Enable engine RUN mode
  digitalWrite(RELAY_1_PIN, LOW);
  digitalWrite(RELAY_2_PIN, LOW);
  digitalWrite(RELAY_3_PIN, HIGH);
  // Display RUNNING message
  lcd.clear();
  lcd.print("RUNNING");
  delay(MEDIUM);
  lcd.clear();
  // Attach interrupt functions to interrupt pins.
  attachInterrupt(1, fuel, RISING); // Pin 3
  attachInterrupt(2, engine, RISING); // Pin 5
  attachInterrupt(3, wheel, RISING); // Pin 6
  attachInterrupt(4, encoder, CHANGE); // Pin 18
  // While the engine is running, monitor all sensors and controls. 
  while (nokill()) {
    // Ballast and steering
    ballast();
    steering();
    // Reset interrupt counters, then wait for next interval.
    SENSOR_FUEL = 0;
    SENSOR_ENGINE = 0;
    SENSOR_WHEEL = 0;
    delay(SHORTER); // pulses per 300ms = L/hour
    // Calculate rates
    RATE_FUEL = SENSOR_FUEL;
    RATE_ENGINE = SENSOR_ENGINE;
    RATE_WHEEL = SENSOR_WHEEL;
    // Package current sensor readings as character buffers, then display them.
    sprintf(BUFFER_FUEL, "FUEL (L/H):   %d    ", RATE_FUEL);
    sprintf(BUFFER_ENGINE, "ENGINE (RPM): %d    ", RATE_ENGINE);
    sprintf(BUFFER_WHEEL, "WHEEL (RPM):  %d    ", RATE_WHEEL);
    sprintf(BUFFER_BALLAST, "BALLAST (RPM):  %d    ", BALLAST_SPEED);
    lcd.setCursor(0,0); 
    lcd.print(BUFFER_FUEL);
    lcd.setCursor(0,1);
    lcd.print(BUFFER_ENGINE);
    lcd.setCursor(0,2);
    lcd.print(BUFFER_WHEEL);
    lcd.setCursor(20,1);
    lcd.print(BUFFER_BALLAST);
  }
}

/* --- Off --- */
// LOOP() --> OFF()
void off() {
  // Engine Position 1
  digitalWrite(RELAY_1_PIN, HIGH);
  digitalWrite(RELAY_2_PIN, HIGH);
  digitalWrite(RELAY_3_PIN, HIGH);
  // Display OFF message
  lcd.clear();
  lcd.print("OFF");
}

/*********** FUNCTIONS ***********/
/*
  Secondary tasks executed by state functions:
    kill()
    lock()
    testKey()
    ballast()
    steering()
*/

/* --- Kill Switches --- */
// Tractor will not operate if any of the killswitches are engaged
boolean nokill() {
  // Get kill switch states
  KILL_SEAT = digitalRead(KILL_SEAT_PIN);
  KILL_BUTTON = digitalRead(KILL_BUTTON_PIN);
  KILL_HITCH = digitalRead(KILL_HITCH_PIN);
  // If driver is off of seat
  if (KILL_SEAT) {
    delay(SHORT);
    if (digitalRead(KILL_SEAT_PIN)) {
      // Engine Position 1
      digitalWrite(RELAY_1_PIN, HIGH);
      digitalWrite(RELAY_2_PIN, HIGH);
      digitalWrite(RELAY_3_PIN, HIGH);
      // Error message
      lcd.clear();
      lcd.print("DRIVER NOT ON SEAT");
      delay(SHORT);
      return false;
    }
    else {
      return true;
    }

  }
  // If hitch is removed
  else if (KILL_HITCH) {
    // Engine Position 1
    digitalWrite(RELAY_1_PIN, HIGH);
    digitalWrite(RELAY_2_PIN, HIGH);
    digitalWrite(RELAY_3_PIN, HIGH);
    // Error message
    lcd.clear();
    lcd.print("HITCH DETACHED");
    delay(SHORT);
    return false;
  }
  // If Kill button is pressed
  else if (KILL_BUTTON) {
     // Engine Position 1
    digitalWrite(RELAY_1_PIN, HIGH);
    digitalWrite(RELAY_2_PIN, HIGH);
    digitalWrite(RELAY_3_PIN, HIGH);
    // Error message
    lcd.clear();
    lcd.print("KILL BUTTON PRESSED");
    delay(SHORT);
    return false;
  }
  // If all kill switches not engaged
  else {
    return true;
  }
}

/* --- Lock Switches --- */
// MR14 will not allow ignition if any of the locks are engaged
boolean nolock() {
  // get Brake switches
  LOCK_LEFTBRAKE = digitalRead(LOCK_LEFTBRAKE_PIN);
  LOCK_RIGHTBRAKE = digitalRead(LOCK_RIGHTBRAKE_PIN);
  // If brakes are not pressed
  if (LOCK_LEFTBRAKE || LOCK_RIGHTBRAKE) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("BRAKES NOT ENGAGED");
    delay(SHORT);
    return false;
  }
  // get CVT state
  LOCK_CVT = analogRead(LOCK_CVT_PIN);

  // If CVT guard is removed
  if (LOCK_CVT > 512) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("CVT GUARD OFF");
    delay(SHORT);
    lcd.clear();
    return false;
  }
  
  // If all locks are proper
  else {
    return true;
  }
  
}

/* --- Start Ignition --- */
boolean start() {
  IGNITION = digitalRead(IGNITION_PIN);
  if (IGNITION) {
    return false;
  }
  else {
    return true;
  }
}

/* --- Test Key --- */
// Tests if RFID key is valid
boolean testKey() {
  // Get RFID key
  int temp[KEYLENGTH]; // length of key is 4;
  for (int i = 0; i < KEYLENGTH; i++) {
    temp[i] = Serial2.read();
  }
  // Antoine's key
  if (temp[0] == 139) {
    for (int j = 0; j < KEYLENGTH; j++) {
      if (ANTOINE[j] == temp[j]) { // compare key to valid key
        continue;
      }
      else {
        return false;
      }
    }
    lcd.clear();
    lcd.print("HELLO ANTOINE");
    delay(MEDIUM);
  }
  // Trevor's key
  else if (temp[0] == 10) {
    for (int j = 0; j < KEYLENGTH; j++) {
      if (TREVOR[j] == temp[j]) { // compare key to valid key
        continue;
      }
      else {
        return false;
      }
    }
    lcd.clear();
    lcd.print("HELLO TREVOR");
    delay(MEDIUM);
  }
  // Bad key
  else {
    return false;
  }
  // Reset key
  temp[4] = 0;
  // If key passed, return true
  return true;
}

/* --- Ballast --- */
int ballast() {
  // Get Ballast control input and well as far/near limit switches.
  BALLAST_SPEED = analogRead(BALLAST_SPEED_PIN);
  LIMIT_NEAR = digitalRead(LIMIT_BALLAST_NEAR_PIN);
  LIMIT_FAR = digitalRead(LIMIT_BALLAST_FAR_PIN);
  MOTOR_FAULT = digitalRead(MOTOR_FAULT_PIN);
  // If motor is stable, enable motor.
  if (!MOTOR_FAULT) {
    // If limits are not engaged, set the proper motor speed.
    if (!LIMIT_NEAR || !LIMIT_FAR) {
      if (BALLAST_SPEED < 575) {
        motors.setM2Speed(-MOTOR_HIGH); // SPEED -2
      }
      else if ((BALLAST_SPEED > 575) && (BALLAST_SPEED < 650)) {
        motors.setM2Speed(-MOTOR_LOW); // SPEED -1
      }
      else if ((BALLAST_SPEED > 650) && (BALLAST_SPEED < 750)) {
        motors.setM2Speed(0); // SPEED 0
      }
      else if ((BALLAST_SPEED > 750) && (BALLAST_SPEED < 825)) {
        motors.setM2Speed(MOTOR_LOW); // SPEED 1
      }
      else if (BALLAST_SPEED > 825) {
        motors.setM2Speed(MOTOR_HIGH); // SPEED 2
      }
      else {
        motors.setM2Speed(0); // SPEED 0
      }
    }
    else {
      motors.setM2Speed(0);
    }
  }
  
  // If motor fault, disable motor.
  else {
    motors.setM2Speed(0);
  }
  // Return
  return BALLAST_SPEED;
}

/* --- Steering --- */
void steering() {
  // Get actuator fault reading.
  ACTUATOR_FAULT = digitalRead(ACTUATOR_FAULT_PIN);
  // If actuator is stable, enable actuator.
  if (!ACTUATOR_FAULT) {
    // Until actuator is left of steering wheel, adjust right.
    if (ACTUATOR_POSITION < STEERING_POSITION) {
      motors.setM1Speed(ACTUATOR_MEDIUM);
      ACTUATOR_POSITION++;
    }
    // Until actuator is right of steering wheel, adjust left.
    else if (ACTUATOR_POSITION > STEERING_POSITION) {
      motors.setM1Speed(-ACTUATOR_MEDIUM);
      ACTUATOR_POSITION--;
    }
    // Otherwise, disable actuator.
    else {
      motors.setM1Speed(0);
    }
  }
  // Otherwise, disable motor;
  else {
    motors.setM1Speed(0);
  }
}

/*********** INTERRUPTS ***********/
/*
    fuel()
    engine()
    wheel()
    encoder()
*/
/* --- Fuel --- */
void fuel() { 
  SENSOR_FUEL++;
}

/* --- Engine --- */
void engine() {
  SENSOR_ENGINE++;
}

/* --- Wheel --- */
void wheel() {
  SENSOR_WHEEL++;
}

/* --- Encoder --- */
void encoder() {
  // Get the current encoder position and limit if oversteered.
  if (digitalRead(STEERING_A_PIN) == digitalRead(STEERING_B_PIN)) {
    STEERING_POSITION++;
    if (STEERING_POSITION > 440) {
      STEERING_POSITION = 440; // limit right steer to 440
    }
  }
  else {
    STEERING_POSITION--;
    if (STEERING_POSITION < -440) {
    STEERING_POSITION = -440; // limit left steer to -440
    }
  }
}
