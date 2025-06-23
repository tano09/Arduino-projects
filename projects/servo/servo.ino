// Potentiometer Controlled Servo Program
// Potentiometer on analog pin A0, Servo on digital pin 9

#include <Servo.h>

// Create servo object
Servo myServo;

// Pin definitions
const int potPin = A0;    // Potentiometer connected to analog pin A0
const int servoPin = 9;   // Servo connected to digital pin 9

// Variables
int potValue = 0;         // Variable to store potentiometer reading
int servoAngle = 0;       // Variable to store servo angle

void setup() {
  // Attach servo to pin
  myServo.attach(servoPin);
  
  // Initialize serial communication for debugging
  Serial.begin(9600);
  Serial.println("Potentiometer Servo Control Started");
  Serial.println("Turn the potentiometer to control the servo");
  
  // Set initial servo position to center
  myServo.write(90);
  delay(500);
}

void loop() {
  // Read potentiometer value (0-1023)
  potValue = analogRead(potPin);
  
  // Map potentiometer value to servo angle (0-180 degrees)
  servoAngle = map(potValue, 0, 1023, 0, 180);
  
  // Write angle to servo
  myServo.write(servoAngle);
  
  // Print values to serial monitor for debugging
  Serial.print("Pot Value: ");
  Serial.print(potValue);
  Serial.print(" | Servo Angle: ");
  Serial.print(servoAngle);
  Serial.println(" degrees");
  
  // Small delay for stability and to avoid overwhelming serial output
  delay(50);
}



