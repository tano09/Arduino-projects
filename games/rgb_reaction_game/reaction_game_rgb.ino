int redPin = 11;
int greenPin = 10;
int bluePin = 9;
int sensorPin1 = 2;
int long ranDelay = 0;
int sensorValue = 0;
float realTime;
int targetColor = 0; // 0 = red, 1 = green, 2 = blue, 3 = yellow, 4 = purple, 5 = cyan, 6 = white
String colorNames[] = {"RED", "GREEN", "BLUE", "YELLOW", "PURPLE", "CYAN", "WHITE"};

void setup()
{
  Serial.begin(9600);
  pinMode(redPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(sensorPin1, INPUT);
  
  // Turn off all LEDs initially
  analogWrite(redPin, 0);
  analogWrite(bluePin, 0);
  analogWrite(greenPin, 0);
}

void loop(){
  // Game start - choose initial target color
  targetColor = random(7);
  
  // First round requires button press to start
  Serial.println("");
  Serial.println("");
  Serial.println("");
  Serial.print("Push the button to start game - Only press when ");
  Serial.print(colorNames[targetColor]);
  Serial.println(" light turns on!");
  
  while (digitalRead(sensorPin1)==   0) {
  }
  
  // Wait for button to be released before continuing
  while (digitalRead(sensorPin1) == 1) {
  }
  
  while (true) { // Keep playing until game over
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.print("Next round - Only press when ");
    Serial.print(colorNames[targetColor]);
    Serial.println(" light turns on!");
    
    delay(1000); // Short pause before starting countdown
    
    // Wait for button to be released before continuing (in case it's still pressed from previous round)
    while (digitalRead(sensorPin1) == 1) {
    }
    
    Serial.println("");
    Serial.println("");
    Serial.println("");
    Serial.println("Get Ready!");
    delay(1000);
    Serial.println("Get Set!");
    delay(1000);  ranDelay = random(5000);
    delay(ranDelay);
    
    // Check if button is being held down - if so, it's cheating!
    if (digitalRead(sensorPin1) == 1) {
      Serial.println("CHEATER! Button was held down!");
      Serial.println("Game Over! Try again...");
      delay(3000);
      return; // End this round, start fresh
    }
  
  Serial.println("Go!");
   
  realTime = millis();
  
  // Light up a random color (might be the target or a different color)
  int actualColor = random(7);
  
  // Turn off all LEDs first
  analogWrite(redPin, 0);
  analogWrite(bluePin, 0);
  analogWrite(greenPin, 0);
  
  // Light up the selected color with PWM for better color recognition
  if (actualColor == 0) {        // Red
    analogWrite(redPin, 255);
  } else if (actualColor == 1) { // Green
    analogWrite(greenPin, 255);
  } else if (actualColor == 2) { // Blue
    analogWrite(bluePin, 255);
  } else if (actualColor == 3) { // Yellow (Red + Green)
    analogWrite(redPin, 200);
    analogWrite(greenPin, 180);
  } else if (actualColor == 4) { // Purple (Red + Blue)
    analogWrite(redPin, 200);
    analogWrite(bluePin, 150);
  } else if (actualColor == 5) { // Cyan (Green + Blue)
    analogWrite(greenPin, 200);
    analogWrite(bluePin, 200);
  } else {                       // White (Red + Green + Blue)
    analogWrite(redPin, 150);
    analogWrite(greenPin, 150);
    analogWrite(bluePin, 150);
  }
 
  // Wait for button press OR timeout (3 seconds)
  unsigned long startTime = millis();
  bool buttonPressed = false;
  
  while (millis() - startTime < 3000) { // 3 second timeout
    if (digitalRead(sensorPin1) == 1) {
      buttonPressed = true;
      break;
    }
  }
  
  // Turn off all LEDs
  analogWrite(redPin, 0);
  analogWrite(bluePin, 0);
  analogWrite(greenPin, 0);
  
  // Check the result
  if (actualColor == targetColor) {
    if (buttonPressed) {
      Serial.println("Correct! Your time was");
      realTime = millis() - realTime;
      Serial.print(realTime/1000,2);
      Serial.println(" seconds");
      Serial.println("Well done! You won the game!");
      delay(3000);
      return; // This will exit the entire loop() function and restart from beginning
    } else {
      Serial.println("Too slow! You should have pressed for that color!");
      Serial.println("Game Over! Try again...");
      delay(3000);
      return; // End game
    }
  } else {
    if (buttonPressed) {
      Serial.print("Wrong! The light was ");
      Serial.print(colorNames[actualColor]);
      Serial.print(" but you should only press for ");
      Serial.println(colorNames[targetColor]);
      Serial.println("Game Over! Try again...");
      delay(3000);
      return; // End game
    } else {
      Serial.print("Good! You correctly didn't press for ");
      Serial.println(colorNames[actualColor]);
      Serial.println("Well done! Starting next round...");
      delay(2000);
      continue; // Continue with same target color
    }
  }
  } // End of while loop
}

