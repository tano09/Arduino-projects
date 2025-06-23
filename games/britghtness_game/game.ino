const int potPin = A0;       // Potentiometer connected to A0
const int ledPin1 = 5;       // LED to match brightness connected to pin 5
const int ledPin2 = 6;       // LED controlled by potentiometer connected to pin 6
const int buttonPin = 4;     // Start button connected to pin 4

bool gameStarted = false;    // Flag to track if the game has started
bool guessSubmitted = false; // Flag to track if the guess has been submitted
unsigned long lastButtonPress = 0; // Timestamp for debounce

void setup() {
  pinMode(potPin, INPUT);
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); // Button with pull-up resistor

  Serial.begin(9600); // For debugging
}

void loop() {
  // Check if the button is pressed
  if (digitalRead(buttonPin) == LOW) {
    unsigned long currentTime = millis();
    if (currentTime - lastButtonPress > 300) { // Debounce delay
      if (!gameStarted) {
        gameStarted = true;
        Serial.println("Game started! Match the brightness.");
      } else {
        guessSubmitted = true;
        Serial.println("Guess submitted!");
      }
      lastButtonPress = currentTime;
    }
  }

  if (gameStarted) {
    // Generate random brightness for LED1
    static bool initialized = false;
    static int targetBrightness = 0;

    if (!initialized) {
      targetBrightness = random(0, 256); // Random brightness between 0 and 255
      analogWrite(ledPin1, targetBrightness);
      initialized = true;
    }

    // Read potentiometer value and adjust LED2 brightness
    int potValue = analogRead(potPin);
    Serial.print("Potentiometer Value: ");
    Serial.println(potValue); // Debug potentiometer value
    int led2Brightness = map(potValue, 0, 1023, 0, 255);
    analogWrite(ledPin2, led2Brightness);

    // Check if the guess is submitted
    if (guessSubmitted) {
      if (abs(led2Brightness - targetBrightness) <= 25) { // Allow small margin of error
        Serial.println("Brightness matched! You win!");
        
        // Blink LEDs to indicate win
        for (int i = 0; i < 5; i++) { // Blink 5 times
          digitalWrite(ledPin1, HIGH);
          digitalWrite(ledPin2, HIGH);
          delay(200);
          digitalWrite(ledPin1, LOW);
          digitalWrite(ledPin2, LOW);
          delay(200);
        }

        // Reset game
        gameStarted = false;
        guessSubmitted = false;
        initialized = false;
        delay(2000); // Pause before restarting
      } else {
        Serial.println("Brightness did not match. Try again!");
        guessSubmitted = false; // Allow another guess
      }
    }
  }
}