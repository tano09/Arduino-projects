/*
  Project 2: LED Light Show Controller - with Hot/Cold Game!
  Description: Controls various LED light patterns using a joystick,
               potentiometer, and buttons. The RGB LED provides status feedback.
               Includes a new "Hot/Cold" game mode.
*/

// --- Pin Definitions ---
#define POT_PIN A0      // Potentiometer for speed/intensity control
#define JOY_X_PIN A1    // Joystick X-axis (used for Hot/Cold game)
#define JOY_Y_PIN A2    // Joystick Y-axis (not used in this version, but available)
#define JOY_SW_PIN 2    // Joystick Button (Digital Pin)

#define BUTTON1_PIN 3   // Button 1: Pause/Play
#define BUTTON2_PIN 4   // Button 2: Cycle Light Show Modes

// Single Color LEDs (ensure these are connected to your breadboard)
// Pins 5, 6, 7, 8 as per the circuit diagram.
const int singleLEDs[] = {5, 6, 7, 8};
const int NUM_SINGLE_LEDS = sizeof(singleLEDs) / sizeof(singleLEDs[0]);

// RGB LED Pins (Common Cathode assumed, connected to PWM pins for color mixing)
#define RGB_R_PIN 9
#define RGB_G_PIN 10
#define RGB_B_PIN 11

// --- Global Variables ---
int currentMode = 0; // 0: Off, 1: Knight Rider, 2: Random Blink, 3: Fading, 4: Hot/Cold Game
const int MAX_MODES = 5; // Total number of modes (0 to 4)

unsigned long previousMillis = 0; // For non-blocking timing
int patternDelay = 100;           // Default delay for patterns (milliseconds)

bool isPaused = false;            // State for pause/play functionality

// Variables for debouncing buttons
unsigned long lastButtonPressTime[3] = {0, 0, 0}; // For JOY_SW, BUTTON1, BUTTON2
const long debounceDelay = 50; // Milliseconds

// Variables for Knight Rider pattern
int knightRiderIndex = 0;
int knightRiderDirection = 1; // 1 for forward, -1 for backward

// Variables for Fading pattern
int fadeBrightness = 0;
int fadeAmount = 5; // How much to change brightness each step

// --- Hot/Cold Game Variables ---
int targetValue = 0;        // The hidden joystick X value to find
bool foundTarget = false;   // True if the player has found the target
unsigned long gameResetTime = 0; // When the game was won/reset
const long WIN_CELEBRATION_DURATION = 2000; // 2 seconds of celebration

// --- Setup Function ---
void setup() {
  Serial.begin(9600); // Initialize serial communication for debugging

  // Set pin modes for inputs
  pinMode(POT_PIN, INPUT);
  pinMode(JOY_X_PIN, INPUT);
  pinMode(JOY_Y_PIN, INPUT);
  pinMode(JOY_SW_PIN, INPUT_PULLUP); // Use internal pull-up resistor
  pinMode(BUTTON1_PIN, INPUT_PULLUP); // Use internal pull-up resistor
  pinMode(BUTTON2_PIN, INPUT_PULLUP); // Use internal pull-up resistor

  // Set pin modes for outputs (Single Color LEDs)
  for (int i = 0; i < NUM_SINGLE_LEDS; i++) {
    pinMode(singleLEDs[i], OUTPUT);
  }

  // Set pin modes for outputs (RGB LED)
  pinMode(RGB_R_PIN, OUTPUT);
  pinMode(RGB_G_PIN, OUTPUT);
  pinMode(RGB_B_PIN, OUTPUT);

  // Initial state: all LEDs off, RGB off
  allLEDsOff();
  setRGBColor(0, 0, 0); // Turn off RGB LED

  // Seed the random number generator (important for random patterns and game)
  randomSeed(analogRead(A3)); // Use an unconnected analog pin for random noise

  // Generate initial target for the game
  generateNewTarget();
}

// --- Helper Functions ---

/**
 * @brief Sets the color of the RGB LED.
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 */
void setRGBColor(int r, int g, int b) {
  analogWrite(RGB_R_PIN, r);
  analogWrite(RGB_G_PIN, g);
  analogWrite(RGB_B_PIN, b);
}

/**
 * @brief Turns off all single color LEDs.
 */
void allLEDsOff() {
  for (int i = 0; i < NUM_SINGLE_LEDS; i++) {
    digitalWrite(singleLEDs[i], LOW);
  }
}

/**
 * @brief Reads a debounced button press.
 * @param pin The digital pin the button is connected to.
 * @param buttonIndex An index (0, 1, or 2) to store the last press time for this button.
 * @return True if a confirmed button press occurred, false otherwise.
 */
bool readDebouncedButton(int pin, int buttonIndex) {
  bool buttonState = digitalRead(pin); // Read the current state of the button

  // If the button is pressed (LOW because of INPUT_PULLUP)
  if (buttonState == LOW) {
    // Check if enough time has passed since the last confirmed press
    if (millis() - lastButtonPressTime[buttonIndex] > debounceDelay) {
      lastButtonPressTime[buttonIndex] = millis(); // Update the last press time
      return true; // Return true for a confirmed press
    }
  }
  return false; // Button not pressed or still debouncing
}

/**
 * @brief Generates a new random target for the Hot/Cold game.
 */
void generateNewTarget() {
  targetValue = random(100, 924); // Target between 100 and 923 to avoid joystick extremes
  foundTarget = false;
  Serial.print("New Target: ");
  Serial.println(targetValue);
  allLEDsOff(); // Clear LEDs for new game
}

/**
 * @brief Provides visual feedback for the Hot/Cold game based on distance to target.
 * @param distance The absolute difference between joystick value and target.
 */
void updateHotColdFeedback(int distance) {
  // RGB LED Feedback
  if (distance < 10) { // Very Hot (within 10 units)
    setRGBColor(255, 0, 0); // Red
  } else if (distance < 50) { // Warm (within 50 units)
    setRGBColor(255, 0, 255); // Magenta
  } else { // Cold (more than 50 units away)
    setRGBColor(0, 0, 255); // Blue
  }

  // Single LED Bar Graph Feedback (more LEDs = hotter)
  allLEDsOff(); // Start by turning all off
  // Map distance (0-1023) to number of LEDs to light (NUM_SINGLE_LEDS down to 0)
  // Invert the mapping: small distance (hot) -> many LEDs, large distance (cold) -> few LEDs
  int numLedsToLight = map(distance, 0, 1023, NUM_SINGLE_LEDS, 0);
  numLedsToLight = constrain(numLedsToLight, 0, NUM_SINGLE_LEDS); // Ensure within bounds

  for (int i = 0; i < numLedsToLight; i++) {
    digitalWrite(singleLEDs[i], HIGH);
  }
}

// --- Main Loop ---
void loop() {
  unsigned long currentMillis = millis(); // Get current time

  // 1. Read Potentiometer for pattern speed/intensity (still active for other modes)
  int potValue = analogRead(POT_PIN);
  patternDelay = map(potValue, 0, 1023, 50, 1000);

  // 2. Handle Button 1 (Pause/Play)
  if (readDebouncedButton(BUTTON1_PIN, 0)) {
    isPaused = !isPaused; // Toggle pause state
    if (isPaused) {
      setRGBColor(255, 100, 0); // Orange for paused
      allLEDsOff(); // Turn off LEDs when paused
    } else {
      setRGBColor(0, 255, 0); // Green for playing
      previousMillis = currentMillis; // Reset previousMillis to avoid immediate pattern jump
    }
    Serial.print("Pause/Play Toggled. isPaused: ");
    Serial.println(isPaused ? "True" : "False");
  }

  // 3. Handle Button 2 (Cycle Light Show Modes)
  if (readDebouncedButton(BUTTON2_PIN, 1)) {
    currentMode = (currentMode + 1) % MAX_MODES; // Cycle to the next mode
    allLEDsOff(); // Turn off all single LEDs when changing mode
    setRGBColor(0, 0, 255); // Flash Blue for mode change
    delay(200); // Brief delay to show the blue flash
    setRGBColor(0, 0, 0); // Turn off RGB after flash
    previousMillis = currentMillis; // Reset timer for the new mode
    knightRiderIndex = 0; // Reset Knight Rider pattern
    knightRiderDirection = 1;
    fadeBrightness = 0; // Reset fade pattern
    foundTarget = false; // Reset game state if entering Hot/Cold mode
    if (currentMode == 4) { // If entering Hot/Cold game mode
      generateNewTarget();
    }
    Serial.print("Mode Changed to: ");
    Serial.println(currentMode);
  }

  // 4. Handle Joystick Button (Reset to Mode 0 / Standby)
  if (readDebouncedButton(JOY_SW_PIN, 2)) {
    currentMode = 0; // Go to standby mode
    allLEDsOff();
    setRGBColor(255, 0, 0); // Flash Red for reset
    delay(500); // Longer delay to emphasize reset
    setRGBColor(0, 0, 0); // Turn off RGB
    isPaused = false; // Ensure not paused after reset
    previousMillis = currentMillis; // Reset timer
    knightRiderIndex = 0; // Reset pattern variables
    knightRiderDirection = 1;
    fadeBrightness = 0;
    foundTarget = false; // Reset game state
    Serial.println("Reset to Mode 0 (Standby)");
  }

  // If paused, do nothing further in the loop for patterns/game
  if (isPaused) {
    return;
  }

  // 5. Run the current light show mode or game
  switch (currentMode) {
    case 0: // Mode 0: All Off / Standby
      allLEDsOff();
      setRGBColor(0, 0, 0); // Keep RGB off
      break;

    case 1: // Mode 1: Knight Rider (Chaser)
      if (currentMillis - previousMillis >= patternDelay) {
        previousMillis = currentMillis;

        allLEDsOff();
        digitalWrite(singleLEDs[knightRiderIndex], HIGH);

        knightRiderIndex += knightRiderDirection;
        if (knightRiderIndex >= NUM_SINGLE_LEDS || knightRiderIndex < 0) {
          knightRiderDirection *= -1;
          knightRiderIndex += knightRiderDirection;
        }
        setRGBColor(0, 255, 0); // Green for active pattern
      }
      break;

    case 2: // Mode 2: Random Blink
      if (currentMillis - previousMillis >= patternDelay) {
        previousMillis = currentMillis;

        allLEDsOff();
        int randomLED = random(NUM_SINGLE_LEDS);
        digitalWrite(singleLEDs[randomLED], HIGH);
        setRGBColor(255, 255, 0); // Yellow for active pattern
      }
      break;

    case 3: // Mode 3: Fading (using the first single LED, assuming it's PWM capable)
      if (currentMillis - previousMillis >= patternDelay / 10) { // Faster update for smooth fade
        previousMillis = currentMillis;

        analogWrite(singleLEDs[0], fadeBrightness);

        fadeBrightness += fadeAmount;
        if (fadeBrightness <= 0 || fadeBrightness >= 255) {
          fadeAmount *= -1;
          fadeBrightness = constrain(fadeBrightness, 0, 255);
        }
        setRGBColor(0, 255, 255); // Cyan for active pattern
      }
      break;

    case 4: // Mode 4: Hot/Cold Game
      if (!foundTarget) {
        int joyXValue = analogRead(JOY_X_PIN);
        int distance = abs(joyXValue - targetValue); // Calculate distance from target

        updateHotColdFeedback(distance); // Update LEDs based on distance

        // Check if target is found (within a very small range)
        if (distance < 5) { // Adjust this threshold for difficulty (smaller = harder)
          foundTarget = true;
          gameResetTime = currentMillis; // Record time of win
          Serial.println("Target Found! New game in 2 seconds...");
        }
      } else {
        // Celebration animation when target is found
        setRGBColor(0, 255, 0); // Solid Green for win
        // Blink all single LEDs
        if (currentMillis - previousMillis >= 100) { // Blink every 100ms
          previousMillis = currentMillis;
          for (int i = 0; i < NUM_SINGLE_LEDS; i++) {
            digitalWrite(singleLEDs[i], !digitalRead(singleLEDs[i])); // Toggle LED state
          }
        }

        // After celebration, reset for a new game
        if (currentMillis - gameResetTime >= WIN_CELEBRATION_DURATION) {
          generateNewTarget(); // Generate a new target
          allLEDsOff(); // Ensure LEDs are off before new game starts
          setRGBColor(0, 0, 0); // Turn off RGB
        }
      }
      break;
  }
}
