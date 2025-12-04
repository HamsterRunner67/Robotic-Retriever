#include <Wire.h>
#include "Adafruit_TCS34725.h"

#include "AFMotor_R4.h"

#include <elapsedMillis.h>

// Motor Pins

// Left Motor
const int motor_AIN1 = 5;
const int motor_AIN2 = 6;
// Right Motor
const int motor_BIN1 = 10;
const int motor_BIN2 = 9;
const int motorSpeed = 120;  // was 90

AF_DCMotor LeftMotor(1);   // Create motor on M1
AF_DCMotor RightMotor(3);  // Create motor on M2

//Ultrasonic Sensors
const int trigPin1 = A3;  //front
const int echoPin1 = A2;
const int trigPin2 = A0;  //left
const int echoPin2 = A1;
const int trigPin3 = A4;  //right
const int echoPin3 = A5;

bool scannedRed = false;
bool neverScanned = true;

elapsedMillis timeSinceLastColorScan = 0;

#define MAXDIS 8
#define MINDIS 6

#define LEFTWHEELOFFSET 1
#define BATTERY_OFFSET 1

// Initialize the TCS34725 (your original settings)
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_600MS,
  TCS34725_GAIN_16X);

void setup() {
  Serial.begin(9600);

  //Start Color Sensor
  if (tcs.begin()) {
    Serial.println("TCS34725 found!");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1)
      ;  // Stop program if not found
  }

  //Ultrasonic Sensors
  pinMode(trigPin1, OUTPUT);
  pinMode(echoPin1, INPUT);
  pinMode(trigPin2, OUTPUT);
  pinMode(echoPin2, INPUT);
  pinMode(trigPin3, OUTPUT);
  pinMode(echoPin3, INPUT);

  //Left Motor
  pinMode(motor_AIN1, OUTPUT);
  pinMode(motor_BIN1, OUTPUT);
  //Right Motor
  pinMode(motor_AIN2, OUTPUT);
  pinMode(motor_BIN2, OUTPUT);

  //Makes sure motors are off
  digitalWrite(motor_AIN1, LOW);
  digitalWrite(motor_BIN2, LOW);
  digitalWrite(motor_AIN2, LOW);
  digitalWrite(motor_BIN2, LOW);

  //Set Motor Speed
  LeftMotor.setSpeed(motorSpeed);
  RightMotor.setSpeed(motorSpeed * .9);

  // L light
  pinMode(LED_BUILTIN, OUTPUT);
  blinkLight(500);
}

void loop() {
  long rightSensor, frontSensor, leftSensor;
  rightSensor = RightSensor();
  frontSensor = FrontSensor();
  leftSensor = LeftSensor();
  Serial.print("Right: ");
  Serial.println(rightSensor);
  Serial.print("Left: ");
  Serial.println(leftSensor);
  Serial.print("Front: ");
  Serial.println(frontSensor);
  delay(100);

  //Test Motor Functions
  //testMotorFunctions();


  stop();
  if (timeSinceLastColorScan > 1000) {
    scannedRed = scanForRed();
    timeSinceLastColorScan = 0;
  }

  if (scannedRed == true && neverScanned == true) {
    scanNFC();
    scannedRed = false;
    neverScanned = false;
  }
  delay(250);
  solveMaze();
  delay(250);
}

void blinkLight(int delayTime) {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(delayTime);
  digitalWrite(LED_BUILTIN, LOW);
  delay(delayTime);
}

//Motor Functions
void forward() {
  analogWrite(motor_AIN1, motorSpeed * LEFTWHEELOFFSET);
  analogWrite(motor_AIN2, 0);
  analogWrite(motor_BIN1, motorSpeed);
  analogWrite(motor_BIN2, 0);
}

// void forward() {
//   //Forward
//   LeftMotor.run(FORWARD);
//   RightMotor.run(FORWARD);
// }

void turn_left() {
  analogWrite(motor_AIN1, 0);
  analogWrite(motor_AIN2, motorSpeed * LEFTWHEELOFFSET);
  analogWrite(motor_BIN1, motorSpeed);
  analogWrite(motor_BIN2, 0);
}

// void turn_left() {
//   LeftMotor.run(FORWARD);
//   RightMotor.run(BACKWARD);
// }

void turn_right() {
  analogWrite(motor_AIN1, motorSpeed * LEFTWHEELOFFSET);
  analogWrite(motor_AIN2, 0);
  analogWrite(motor_BIN1, 0);
  analogWrite(motor_BIN2, motorSpeed);
}

// void turn_right() {
//   LeftMotor.run(BACKWARD);
//   RightMotor.run(FORWARD);
// }

void reverse() {
  analogWrite(motor_AIN1, 0);
  analogWrite(motor_AIN2, motorSpeed * LEFTWHEELOFFSET);
  analogWrite(motor_BIN1, 0);
  analogWrite(motor_BIN2, motorSpeed);
}

// void reverse() {
//   LeftMotor.run(BACKWARD);
//   RightMotor.run(BACKWARD);
// }

void stop() {
  analogWrite(motor_AIN1, 0);
  analogWrite(motor_AIN2, 0);
  analogWrite(motor_BIN1, 0);
  analogWrite(motor_BIN2, 0);
}

// void stop() {
//   LeftMotor.run(BRAKE);
//   RightMotor.run(BRAKE);
// }

void testMotorFunctions() {
  forward();
  delay(500);
  stop();
  delay(500);
  turn_left();
  delay(500);
  stop();
  delay(500);
  turn_right();
  delay(500);
  stop();
  delay(500);
}

long pulseUltrasonicSensor(int trigPin, int echoPin) {
  long duration;
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  return duration;
}

long microsecondsToCentimeters(long microseconds) {
  return microseconds / 29 / 2;
}

long microsecondsToInches(long microseconds) {
  return microseconds / 74 / 2;
}

long FrontSensor() {
  long duration;
  duration = microsecondsToCentimeters(pulseUltrasonicSensor(trigPin1, echoPin1));
  delay(50);
  return (duration);  // convert the distance  to centimeters.
}

long RightSensor() {
  long duration;
  duration = microsecondsToCentimeters(pulseUltrasonicSensor(trigPin3, echoPin3));
  delay(50);
  return (duration);  // convert the distance  to centimeters.
}

long LeftSensor() {
  long duration;
  duration = microsecondsToCentimeters(pulseUltrasonicSensor(trigPin2, echoPin2));
  delay(50);
  return (duration);  // convert the distance  to centimeters.
}


/* Maze Code For Refrence*/
void solveMaze() {
  if (FrontSensor() > MINDIS && RightSensor() > MINDIS && LeftSensor() > MINDIS) {
    forward();
    delay(200);
  } else if (FrontSensor() < MAXDIS + 1 && RightSensor() < MAXDIS + 1 && LeftSensor() < MAXDIS + 1)  // obstacle infront of all 3  sides
  {
    reverse();
    delay(500);
    turn_left();
    delay(400);  // Changed based on battery
    // forward();
    // delay(250);
    if ((LeftSensor()) > (RightSensor()))
      turn_left();
    else
      turn_right();
    delay(200);
  } else if (FrontSensor() < MAXDIS && RightSensor() < MAXDIS && LeftSensor() > MAXDIS)  // obstacle on right and front sides
  {
    turn_left();
    delay(300);                                                                          // turn left  side
  } else if (FrontSensor() < MAXDIS && RightSensor() > MAXDIS && LeftSensor() < MAXDIS)  // obstacle on left and front sides
  {
    turn_right();
    delay(300);                                                                          // turn right side
  } else if (FrontSensor() < MAXDIS && RightSensor() > MINDIS && LeftSensor() > MINDIS)  // obstacle  on front sides
  {
    turn_left();
    delay(400);
    //stop();
    forward();                                                                           // then turn right  //********************
  } else if (FrontSensor() > MINDIS && RightSensor() > MINDIS && LeftSensor() < MINDIS)  // obstacle on left sides
  {
    turn_right();  // then turn  right and then forward
    delay(400);
    forward();
    delay(200);
    //stop();
  } else if (FrontSensor() > MINDIS && RightSensor() < MINDIS && LeftSensor() > MINDIS)  // obstacle on right sides
  {
    turn_left();  // then turn left and then right
    delay(400);
    forward();
    delay(200);
    //stop();
  } else {
    forward();
    delay(200);
  }
}

// Checks for red color
bool scanForRed() {
  float r, g, b;
  float redRatio, greenRatio, blueRatio;
  float r_weight = 120;
  float g_weight = 80;
  float b_weight = 45;

  float redThreshold = 0.05;
  tcs.getRGB(&r, &g, &b);

  float sum = r + g + b;
  // Uses offset weights to detect red better
  if (sum > 0.0f) {
    redRatio = constrain(r - r_weight, 0, 256) / sum;
    greenRatio = constrain(g - g_weight, 0, 256) / sum;
    blueRatio = constrain(b - b_weight, 0, 256) / sum;
  }

  Serial.println("Scanning For Color");
  Serial.print("R: ");
  Serial.print(redRatio, 2);
  Serial.print(" G: ");
  Serial.print(greenRatio, 2);
  Serial.print(" B: ");
  Serial.println(blueRatio, 2);

  if (redRatio > redThreshold) {
    Serial.println("RED!");
    return true;
  } else {
    return false;
  }
}

// Put robot in the position to scan nfc tag
void scanNFC() {
  turn_left();
  delay(400);
  stop();
  int numTurns = 10;
  for (int i = 0; i < numTurns; i++) {
    reverse();
    delay(150);
    stop();
    delay(150);
  }
  forward();
  delay(250);
}
