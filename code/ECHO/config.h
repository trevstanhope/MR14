/*
  config.h
  Author: Trevor Stanhope
  Date: 2013-05-14
  Summary: Configuration Library for MR14
*/

/*********** INPUT/OUTPUT ***********/
/* --- PWM PINS --- */
// PWM 1: SENSORS
#define SENSOR_FUEL_PIN 3
#define SENSOR_ENGINE_PIN 5
#define SENSOR_WHEEL_PIN 18
#define ACTUATOR_FAULT_PIN 6
#define MOTOR_FAULT_PIN 12

/* --- COMMUNICATION PINS --- */
// COMMUNICATION 1: RFID STEERING AND LCD
#define RFID_TX_PIN 16
#define RFID_RX_PIN 17
#define STEERING_A_PIN 19
#define STEERING_B_PIN 20

/* --- START DIGITAL PINS --- */
// DIGITAL 1: KILL, TRIGGER AND LOCK SWITCHES
#define IGNITION_POWER 22
#define IGNITION_PIN 23
#define KILL_BUTTON_POWER 24
#define KILL_BUTTON_PIN 25
#define KILL_HITCH_POWER 26
#define KILL_HITCH_PIN 27
#define KILL_SEAT_POWER 28
#define KILL_SEAT_PIN 29
#define LOCK_LEFTBRAKE_POWER 30
#define LOCK_LEFTBRAKE_PIN 31
#define LOCK_RIGHTBRAKE_POWER 32
#define LOCK_RIGHTBRAKE_PIN 33
#define LIMIT_NEAR_POWER 34
#define LIMIT_NEAR_PIN 35
#define LIMIT_FAR_POWER 36
#define LIMIT_FAR_PIN 37

// DIGITAL 2: ENGINE RELAYS
#define RELAY_1_PIN 39 // pin to disable ENGINE STOP
#define RELAY_2_PIN 43 // pin to enable REGULATION AND FUEL SOLENOID
#define RELAY_3_PIN 45 // pin to enable STARTER

/* --- START ANALOG PINS --- */
// ANALOG 1: POTENTIOMETERS
#define ACTUATOR_CURRENT_PIN 0
#define MOTOR_CURRENT_PIN 1
#define BALLAST_SPEED_PIN 2
#define LOCK_CVT_PIN 3

/*********** VALUES ***********/
/* --- CONSTANTS --- */
#define KEYLENGTH 4
#define BAUD 9600
#define LCD_WIDTH 20
#define LCD_HEIGHT 4
#define PRECISION 2
#define DIGITS 4
#define CHARS 8

/* --- TIME --- */
#define SHORTEST 2
#define SHORTER 250
#define SHORT 500
#define MEDIUM 1000
#define LONG 2000
#define LONGER 5000

/* --- SERIAL COMMANDS --- */
#define LCD_I2C_PORT 0x3F
#define RFID_CARD 0x02
#define RFID_READ 0x03
#define RFID_WRITE 0x04

// MOTOR CONFIGURATIONS
#define REVERSE -100
#define OFF 0
#define SLOW 50
#define MODERATE 100
#define FAST 200
