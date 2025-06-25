/* Whack-a-Mole Game
This sketch is a modified version of the Simon Says game to create a Whack-a-Mole game.

Hardware needed:
5x pushbuttons (4 mole buttons + 1 start button)
1x Blue LED
1x Yellow LED
1x Red LED
1x Green LED
5x 1k resistors
5x 10k resistors
12x jumpers
*/

const int MAX_ROUNDS = 5; // Number of rounds in the game
const int MOLE_DELAY = 1000; // Time the mole stays lit (in milliseconds)
const int BUTTON_DELAY = 200; // Debounce delay for buttons
const int START_BUTTON = A4; // Start button pin

int moleLEDs[] = {2, 3, 4, 5}; // LED pins for moles
int moleButtons[] = {A3, A2, A1, A0}; // Button pins for moles
int score = 0; // Player's score
int currentRound = 0; // Current round
bool gameActive = false; // Game state

void setup() {
  // Initialize LED pins as outputs
  for (int i = 0; i < 4; i++) {
    pinMode(moleLEDs[i], OUTPUT);
    digitalWrite(moleLEDs[i], LOW);
  }

  // Initialize button pins as inputs with pull-up resistors
  for (int i = 0; i < 4; i++) {
    pinMode(moleButtons[i], INPUT_PULLUP);
  }
  
  // Initialize start button pin as input with pull-up resistor
  pinMode(START_BUTTON, INPUT_PULLUP);

  // Start serial communication for debugging
  Serial.begin(9600);
  Serial.println("Whack-a-Mole Game Ready!");
  Serial.println("Press the start button to begin...");
}

void loop() {
  // Check if game is not active, wait for start button
  if (!gameActive) {
    if (digitalRead(START_BUTTON) == LOW) {
      // Start button pressed
      delay(BUTTON_DELAY); // Debounce delay
      gameActive = true;
      score = 0;
      currentRound = 0;
      Serial.println("Game Started!");
      delay(500); // Brief pause before first round
    }
    return;
  }
  
  // Game is active
  if (currentRound < MAX_ROUNDS) {
    playRound(); // Play a round of the game
  } else {
    endGame(); // End the game when all rounds are completed
  }
}

void playRound() {
  // Light up a random mole (LED)
  int moleIndex = random(0, 4); // Randomly select a mole
  digitalWrite(moleLEDs[moleIndex], HIGH);
  Serial.print("Mole lit: ");
  Serial.println(moleLEDs[moleIndex]);

  // Wait for the player to press the correct button
  unsigned long startTime = millis();
  while (millis() - startTime < MOLE_DELAY) {
    for (int i = 0; i < 4; i++) {
      if (digitalRead(moleButtons[i]) == LOW) { // Button pressed
        if (i == moleIndex) { // Correct button
          Serial.println("Hit!");
          score++;
          digitalWrite(moleLEDs[moleIndex], LOW); // Turn off the mole
          delay(BUTTON_DELAY); // Debounce delay
          return;
        } else { // Wrong button
          Serial.println("Miss!");
          digitalWrite(moleLEDs[moleIndex], LOW); // Turn off the mole
          delay(BUTTON_DELAY); // Debounce delay
          return;
        }
      }
    }
  }

  // If no button is pressed within the time limit
  Serial.println("Missed!");
  digitalWrite(moleLEDs[moleIndex], LOW); // Turn off the mole
  currentRound++;
}


void endGame() {
  Serial.println("Game Over!");
  Serial.print("Your score: ");
  Serial.println(score);



  // Blink all LEDs to indicate the game is over
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
      digitalWrite(moleLEDs[j], HIGH);
    }
    delay(500);
    for (int j = 0; j < 4; j++) {
      digitalWrite(moleLEDs[j], LOW);
    }
    delay(500);
  }
  // Display score using LED flashes
  displayScore();

  // Reset the game
  score = 0;
  currentRound = 0;
  gameActive = false; // Set game to inactive state
  Serial.println("Press the start button to play again...");
}

void displayScore() {
  delay(1000); // Pause before showing score
 
  
  // Flash the first LED (pin 2) a number of times equal to the score
  for (int i = 0; i < score; i++) {
    digitalWrite(moleLEDs[0], HIGH); // Light up first LED
    delay(300);
    digitalWrite(moleLEDs[0], LOW);  // Turn off first LED
    delay(300);
  }
  
  delay(1000); // Pause after showing score
}