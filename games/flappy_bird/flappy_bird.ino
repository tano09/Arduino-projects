#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>

// Pin definitions
#define BUTTON_PIN 11
#define MATRIX_PIN 12

// Matrix configuration
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16

// Game configuration
#define GRAVITY -0.25
#define JUMP_STRENGTH .75
#define PIPE_SPEED 1
#define PIPE_GAP 4
#define PIPE_WIDTH 2

// Initialize the LED matrix
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_PIN,
  NEO_MATRIX_TOP + NEO_MATRIX_LEFT + NEO_MATRIX_ROWS + NEO_MATRIX_ZIGZAG,
  NEO_GRB + NEO_KHZ800);

// Game variables
float birdY = 8.0;
float birdVelocity = 0.0;
int birdX = 3;  // Back to fixed position
int pipeX = MATRIX_WIDTH;
int pipeGapY = 6;
int score = 0;
bool gameRunning = false;  // Start with game not running
bool buttonPressed = false;
bool lastButtonState = false;
unsigned long lastFrameTime = 0;
unsigned long lastPipeMove = 0;
const int frameDelay = 150;
const int pipeMoveDelay = 300;

// Colors
uint16_t birdColor = matrix.Color(255, 255, 0);    // Yellow
uint16_t pipeColor = matrix.Color(0, 255, 0);      // Green
uint16_t bgColor = matrix.Color(0, 0, 0);          // Black
uint16_t scoreColor = matrix.Color(255, 0, 0);     // Red

void setup() {
  Serial.begin(9600);
  Serial.println("Starting Flappy Bird...");
  
  // Initialize button
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize matrix
  Serial.println("Initializing matrix...");
  matrix.begin();
  matrix.setBrightness(30);  // Lowered brightness
  matrix.fillScreen(0);
  matrix.show();
  
  Serial.println("Flappy Bird Ready! Press button to start.");
  
  // Show the start screen and wait for button press
  showStartScreen();
}

void loop() {
  // Read button state
  bool currentButtonState = !digitalRead(BUTTON_PIN);
  buttonPressed = currentButtonState && !lastButtonState;
  lastButtonState = currentButtonState;
  
  if (buttonPressed) {
    Serial.println("Button pressed!");
    if (!gameRunning) {
      // Start the game
      resetGame();
    } else {
      // Jump during game
      birdVelocity = JUMP_STRENGTH;
    }
  }
  
  if (!gameRunning) {
    // Show a simple waiting pattern
    matrix.fillScreen(0);
    matrix.drawPixel(8, 8, matrix.Color(0, 255, 0));  // Green center dot
    matrix.show();
    delay(100);
    return;
  }-
  
  // Update game physics
  unsigned long currentTime = millis();
  if (currentTime - lastFrameTime >= frameDelay) {
    updateBird();
    
    if (currentTime - lastPipeMove >= pipeMoveDelay) {
      updatePipes();
      lastPipeMove = currentTime;
    }
    
    // Check collisions
    if (checkCollisions()) {
      gameOver();
      showStartScreen();
      return;
    }
    
    // Draw everything
    drawGame();
    
    lastFrameTime = currentTime;
  }
}

void updateBird() {
  // Apply gravity
  birdVelocity += GRAVITY;
  birdY += birdVelocity;
  
  // Keep bird in bounds
  if (birdY < 0) {
    birdY = 0;
    birdVelocity = 0;
  }
  if (birdY >= MATRIX_HEIGHT) {
    gameOver();
  }
}

void updatePipes() {
  pipeX -= PIPE_SPEED;
  
  // Reset pipe when it goes off screen
  if (pipeX < -PIPE_WIDTH) {
    pipeX = MATRIX_WIDTH;
    pipeGapY = random(1, MATRIX_HEIGHT - PIPE_GAP - 1);  // More random range
    score++;
    Serial.print("Score: ");
    Serial.println(score);
  }
}

bool checkCollisions() {
  // Check if bird hits ground or ceiling
  if (birdY <= 0 || birdY >= MATRIX_HEIGHT - 1) {
    return true;
  }
  
  // Check pipe collision
  if (birdX >= pipeX && birdX < pipeX + PIPE_WIDTH) {
    int birdIntY = (int)birdY;
    if (birdIntY < pipeGapY || birdIntY >= pipeGapY + PIPE_GAP) {
      return true;
    }
  }
  
  return false;
}

void drawGame() {
  matrix.fillScreen(bgColor);
  
  // Draw bird
  matrix.drawPixel(birdX, (int)birdY, birdColor);
  
  // Draw pipes
  if (pipeX >= 0 && pipeX < MATRIX_WIDTH) {
    // Top pipe
    for (int x = pipeX; x < pipeX + PIPE_WIDTH && x < MATRIX_WIDTH; x++) {
      for (int y = 0; y < pipeGapY; y++) {
        if (x >= 0) {
          matrix.drawPixel(x, y, pipeColor);
        }
      }
    }
    
    // Bottom pipe
    for (int x = pipeX; x < pipeX + PIPE_WIDTH && x < MATRIX_WIDTH; x++) {
      for (int y = pipeGapY + PIPE_GAP; y < MATRIX_HEIGHT; y++) {
        if (x >= 0) {
          matrix.drawPixel(x, y, pipeColor);
        }
      }
    }
  }
  

  
  matrix.show();
}

void gameOver() {
  gameRunning = false;
  
  Serial.print("Game Over! Final Score: ");
  Serial.println(score);
  
  // Flash the screen red with lower brightness
  for (int i = 0; i < 3; i++) {
    matrix.fillScreen(matrix.Color(100, 0, 0));  // Dimmer red
    matrix.show();
    delay(200);
    matrix.fillScreen(bgColor);
    matrix.show();
    delay(200);
  }
  
  showGameOverScreen();
}

void resetGame() {
  Serial.println("Resetting game...");
  birdY = 8.0;
  birdVelocity = 0.0;
  pipeX = MATRIX_WIDTH;
  pipeGapY = random(1, MATRIX_HEIGHT - PIPE_GAP - 1);  // Random starting position too
  score = 0;
  gameRunning = true;
  lastFrameTime = millis();
  lastPipeMove = millis();
  
  Serial.println("Game Reset! Press button to jump.");
}

void showStartScreen() {
  matrix.fillScreen(bgColor);
  
  // Draw a simple bird icon
  matrix.drawPixel(7, 7, birdColor);
  matrix.drawPixel(8, 7, birdColor);
  matrix.drawPixel(7, 8, birdColor);
  matrix.drawPixel(8, 8, birdColor);
  
  // Draw some decorative elements
  for (int i = 0; i < 5; i++) {
    matrix.drawPixel(i, 0, matrix.Color(0, 0, 255));
    matrix.drawPixel(15-i, 15, matrix.Color(0, 0, 255));
  }
  
  matrix.show();
  
  // Wait for button press to start
  while (digitalRead(BUTTON_PIN) == HIGH) {
    delay(50);
  }
  while (digitalRead(BUTTON_PIN) == LOW) {
    delay(50);
  }
  
  resetGame();
}

void showGameOverScreen() {
  matrix.fillScreen(bgColor);
  
  // Simple way to display score as a number (0-9)
  if (score < 10) {
    // Display single digit score in the center
    drawNumber(score, 6, 4, scoreColor);
  } else {
    // For scores 10+, show tens digit and ones digit
    int tens = score / 10;
    int ones = score % 10;
    drawNumber(tens, 3, 4, scoreColor);  // Tens digit on left
    drawNumber(ones, 9, 4, scoreColor);  // Ones digit on right
  }
  
  matrix.show();
  
  // Keep the score displayed for 5 seconds
  delay(5000);
}

// Function to draw simple numbers 0-9 using a 3x5 pixel font
void drawNumber(int number, int startX, int startY, uint16_t color) {
  // Define 3x5 patterns for digits 0-9 (flipped vertically)
  bool patterns[10][5][3] = {
    // 0
    {{1,1,1},
     {1,0,1},
     {1,0,1},
     {1,0,1},
     {1,1,1}},
    // 1
    {{1,1,1},
     {0,1,0},
     {0,1,0},
     {1,1,0},
     {0,1,0}},
    // 2
    {{1,1,1},
     {1,0,0},
     {1,1,1},
     {0,0,1},
     {1,1,1}},
    // 3
    {{1,1,1},
     {0,0,1},
     {1,1,1},
     {0,0,1},
     {1,1,1}},
    // 4
    {{0,0,1},
     {0,0,1},
     {1,1,1},
     {1,0,1},
     {1,0,1}},
    // 5
    {{1,1,1},
     {0,0,1},
     {1,1,1},
     {1,0,0},
     {1,1,1}},
    // 6
    {{1,1,1},
     {1,0,1},
     {1,1,1},
     {1,0,0},
     {1,1,1}},
    // 7
    {{0,1,0},
     {0,1,0},
     {0,0,1},
     {0,0,1},
     {1,1,1}},
    // 8
    {{1,1,1},
     {1,0,1},
     {1,1,1},
     {1,0,1},
     {1,1,1}},
    // 9
    {{1,1,1},
     {0,0,1},
     {1,1,1},
     {1,0,1},
     {1,1,1}}
  };
  
  // Draw the pattern
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 3; x++) {
      if (patterns[number][y][x]) {
        matrix.drawPixel(startX + x, startY + y, color);
      }
    }
  }
}
