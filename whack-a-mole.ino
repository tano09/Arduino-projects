/* Whack-a-Mole Game
This sketch is a modified version of the Simon Says game to create a Whack-a-Mole game.

Hardware needed:
4x pushbuttons
1x Blue LED
1x Yellow LED
1x Red LED
1x Green LED
4x 1k resistors
4x 10k resistors
10x jumpers
*/

const int MAX_ROUNDS = 10; // Number of rounds in the game
const int MOLE_DELAY = 1000; // Time the mole stays lit (in milliseconds)
const int BUTTON_DELAY = 200; // Debounce delay for buttons

int moleLEDs[] = {2, 3, 4, 5}; // LED pins for moles
int moleButtons[] = {A3, A2, A1, A0}; // Button pins for moles
int score = 0; // Player's score
int currentRound = 0; // Current round

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

  // Start serial communication for debugging
  Serial.begin(9600);
  Serial.println("Whack-a-Mole Game Started!");
}

void loop() {
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

  // Reset the game
  score = 0;
  currentRound = 0;
}