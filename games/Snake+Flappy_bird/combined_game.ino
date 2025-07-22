#include <Adafruit_NeoPixel.h>

// Pin definitions
#define BUTTON_PIN 12
#define MATRIX_PIN 11
#define JOYSTICK_XPIN A3
#define JOYSTICK_YPIN A4

// Matrix configuration
#define MATRIX_WIDTH 16
#define MATRIX_HEIGHT 16

// Game states
enum GameState {
  MENU,
  FLAPPY_BIRD,
  SNAKE,
  GAME_OVER_STATE
};

GameState currentGameState = MENU;
int selectedGame = 0; // 0 = Flappy Bird, 1 = Snake

// Create matrix object - ONLY Adafruit_NeoPixel like working test
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(256, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

// Shared variables
bool buttonPressed = false;
bool lastButtonState = false;

// ============== FLAPPY BIRD VARIABLES ==============
float birdY = 8.0;
float birdVelocity = 0.0;
int birdX = 3;
int pipeX = 16;
int pipeGapY = 6;
int flappyScore = 0;
bool flappyGameRunning = false;
unsigned long lastFlappyUpdate = 0;

// FLAPPY BIRD GAME SETTINGS - Easy to customize!
const float GRAVITY = -0.25;           // How fast bird falls (more negative = faster fall)
const float JUMP_STRENGTH = 0.75;     // How high bird jumps (higher = bigger jumps)
const int GAME_SPEED = 200;           // Game update speed in ms (lower = faster game)
const int PIPE_GAP_SIZE = 4;          // Height of gap in pipes (bigger = easier)
const int PIPE_WIDTH = 2;             // Width of pipes (1 or 2 pixels)

// ============== SNAKE VARIABLES - OPTIMIZED ==============
int snake[50];  // Reduced size to save memory
int snakeLength = 2;
int snakeHead = 135;
int apple = 100;
int snakeDirection = 1;
int lastDirection = 1;
int xpinval = 512, ypinval = 512;
unsigned long lastSnakeMove = 0;

// SNAKE GAME SETTINGS - Easy to customize!
const int SNAKE_SPEED = 600;          // Snake move speed in ms (lower = faster snake)
const int JOYSTICK_THRESHOLD = 200;   // How far to move joystick (lower = more sensitive)

// Helper functions - using only NeoPixel methods
void clearMatrix() {
  for (int i = 0; i < 256; i++) {
    matrix.setPixelColor(i, 0); // Black - using 0 instead of matrix.Color(0,0,0)
  }
}

// Convert X,Y to pixel index for 16x16 zigzag matrix
int xyToPixel(int x, int y) {
  if (x < 0 || x >= 16 || y < 0 || y >= 16) return -1;
  
  int pixel;
  if (y % 2 == 0) {
    // Even rows: left to right
    pixel = y * 16 + x;
  } else {
    // Odd rows: right to left
    pixel = y * 16 + (15 - x);
  }
  return pixel;
}

void setPixel(int x, int y, uint32_t color) {
  int pixel = xyToPixel(x, y);
  if (pixel >= 0) {
    matrix.setPixelColor(pixel, color);
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Combo Game - NeoPixel Only");
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize matrix exactly like working test
  matrix.begin();
  matrix.setBrightness(50);  // Reduced brightness from 255 to 50 (about 20%)
  matrix.show();
  
  // Quick working test pattern
  clearMatrix();
  matrix.show();
  delay(100);
  
  // Test first 5 pixels white like working version
  for(int i = 0; i < 5; i++) {
    matrix.setPixelColor(i, matrix.Color(255, 255, 255));
  }
  matrix.show();
  delay(500);
  
  clearMatrix();
  matrix.show();
  
  // Calibrate joystick
  for(int i = 0; i < 10; i++) {
    xpinval += analogRead(JOYSTICK_XPIN);
    ypinval += analogRead(JOYSTICK_YPIN);
    delay(10);
  }
  xpinval /= 10;
  ypinval /= 10;
  
  Serial.println("Ready!");
  currentGameState = MENU;
  showMenu();
}

void loop() {
  // Read button
  bool currentButtonState = !digitalRead(BUTTON_PIN);
  buttonPressed = currentButtonState && !lastButtonState;
  lastButtonState = currentButtonState;
  
  switch (currentGameState) {
    case MENU:
      handleMenu();
      break;
    case FLAPPY_BIRD:
      handleFlappyBird();
      break;
    case SNAKE:
      handleSnake();
      break;
    case GAME_OVER_STATE:
      handleGameOver();
      break;
  }
}

// ============== MENU FUNCTIONS ==============
void showMenu() {
  Serial.println("=== MENU ===");
  
  clearMatrix();
  
  // Top row white (pixels 0-15)
  for(int i = 0; i < 16; i++) {
    matrix.setPixelColor(i, matrix.Color(255, 255, 255));
  }
  
  // Flappy Bird section (rows 4-7, y=4 to y=7)
  uint32_t flappyBgColor = (selectedGame == 0) ? matrix.Color(50, 0, 0) : matrix.Color(0, 0, 0); // Red background if selected
  // Fill background
  for(int y = 4; y < 8; y++) {
    for(int x = 0; x < 16; x++) {
      setPixel(x, y, flappyBgColor);
    }
  }
  // Draw mini flappy bird scene
  setPixel(3, 5, matrix.Color(255, 255, 0)); // Yellow bird
  setPixel(3, 6, matrix.Color(255, 255, 0)); // Yellow bird (2 pixels tall)
  // Draw mini pipe
  setPixel(10, 4, matrix.Color(0, 255, 0)); // Green pipe top
  setPixel(10, 7, matrix.Color(0, 255, 0)); // Green pipe bottom
  setPixel(11, 4, matrix.Color(0, 255, 0)); // Green pipe top
  setPixel(11, 7, matrix.Color(0, 255, 0)); // Green pipe bottom
  
  // Snake section (rows 8-11, y=8 to y=11)
  uint32_t snakeBgColor = (selectedGame == 1) ? matrix.Color(50, 0, 0) : matrix.Color(0, 0, 0); // Red background if selected
  // Fill background
  for(int y = 8; y < 12; y++) {
    for(int x = 0; x < 16; x++) {
      setPixel(x, y, snakeBgColor);
    }
  }
  // Draw mini snake (3 segments)
  setPixel(5, 9, matrix.Color(0, 255, 0));  // Snake head
  setPixel(4, 9, matrix.Color(0, 255, 0));  // Snake body
  setPixel(3, 9, matrix.Color(0, 255, 0));  // Snake tail
  // Draw apple
  setPixel(8, 10, matrix.Color(255, 0, 0)); // Red apple
  
  // Bottom row blue (pixels 240-255)
  for(int i = 240; i < 256; i++) {
    matrix.setPixelColor(i, matrix.Color(0, 0, 255));
  }
  
  matrix.show();
  Serial.println("Menu shown - Game previews displayed");
}

void handleMenu() {
  int x = analogRead(JOYSTICK_XPIN);
  
  // Menu navigation uses X axis (original way that was working)
  if (x < (xpinval - 100)) {
    if (selectedGame != 0) {
      selectedGame = 0;
      showMenu();
      delay(200);
    }
  } else if (x > (xpinval + 100)) {
    if (selectedGame != 1) {
      selectedGame = 1;
      showMenu();
      delay(200);
    }
  }
  
  if (buttonPressed) {
    if (selectedGame == 0) {
      Serial.println("Starting Flappy Bird");
      currentGameState = FLAPPY_BIRD;
      resetFlappyBird();
    } else {
      Serial.println("Starting Snake");
      currentGameState = SNAKE;
      resetSnake();
    }
  }
}

// ============== FLAPPY BIRD FUNCTIONS ==============
void resetFlappyBird() {
  birdY = 8.0;
  birdVelocity = 0.0;
  pipeX = 16;
  pipeGapY = 6;
  flappyScore = 0;
  flappyGameRunning = true;
  lastFlappyUpdate = millis();
}

void handleFlappyBird() {
  if (buttonPressed && flappyGameRunning) {
    birdVelocity = JUMP_STRENGTH;  // Use customizable jump strength
  }
  
  if (millis() - lastFlappyUpdate > GAME_SPEED) {  // Use customizable game speed
    birdVelocity += GRAVITY;  // Use customizable gravity
    birdY += birdVelocity;
    
    if (birdY <= 0) { birdY = 0; flappyGameOver(); return; }
    if (birdY >= 15) { birdY = 15; flappyGameOver(); return; }
    
    pipeX--;
    if (pipeX < -PIPE_WIDTH) {  // Use customizable pipe width
      pipeX = 16;
      pipeGapY = random(2, 16 - PIPE_GAP_SIZE - 2);  // Use customizable gap size
      flappyScore++;
      Serial.print("Flappy score: ");
      Serial.println(flappyScore);
    }
    
    if (pipeX >= birdX - 1 && pipeX <= birdX + 1) {  // Better collision detection
      int birdIntY = (int)birdY;
      if (birdIntY < pipeGapY || birdIntY >= pipeGapY + PIPE_GAP_SIZE) {
        flappyGameOver();
        return;
      }
    }
    
    clearMatrix();
    setPixel(3, (int)birdY, matrix.Color(255, 255, 0));
    
    if (pipeX >= 0 && pipeX < 16) {
      for(int y = 0; y < pipeGapY; y++) {
        setPixel(pipeX, y, matrix.Color(0, 255, 0));
        if (pipeX + 1 < 16 && PIPE_WIDTH == 2) setPixel(pipeX + 1, y, matrix.Color(0, 255, 0));
      }
      for(int y = pipeGapY + PIPE_GAP_SIZE; y < 16; y++) {
        setPixel(pipeX, y, matrix.Color(0, 255, 0));
        if (pipeX + 1 < 16 && PIPE_WIDTH == 2) setPixel(pipeX + 1, y, matrix.Color(0, 255, 0));
      }
    }
    
    matrix.show();
    lastFlappyUpdate = millis();
  }
}

void flappyGameOver() {
  flappyGameRunning = false;
  Serial.print("Flappy final score: ");
  Serial.println(flappyScore);
  showNumber(flappyScore);
  delay(5000);  // Show score for 5 seconds instead of 3
  currentGameState = GAME_OVER_STATE;
}

// ============== SNAKE FUNCTIONS - CONVERTED TO NEOPIXEL ONLY ==============
void resetSnake() {
  snakeLength = 2;
  int startX = 8, startY = 8;
  snakeHead = xyToPixel(startX, startY);
  snake[0] = snakeHead;
  snake[1] = xyToPixel(startX - 1, startY);
  apple = random(0, 256);
  snakeDirection = 2;
  lastDirection = 2;
  lastSnakeMove = millis();
  drawSnake();
}

void handleSnake() {
  // Read joystick for direction - ONLY change direction, don't move multiple times
  int x = analogRead(JOYSTICK_XPIN);
  int y = analogRead(JOYSTICK_YPIN);
  
  // Change direction based on joystick - ROTATED 90° CLOCKWISE: Y-up=Right, Y-down=Left, X-left=Up, X-right=Down
  if (y > (ypinval + JOYSTICK_THRESHOLD) && lastDirection != 2) {  // Use customizable threshold
    snakeDirection = 1; // Left (joystick DOWN)
  } else if (y < (ypinval - JOYSTICK_THRESHOLD) && lastDirection != 1) {  // Use customizable threshold
    snakeDirection = 2; // Right (joystick UP)
  } else if (x < (xpinval - JOYSTICK_THRESHOLD) && lastDirection != 4) {  // Use customizable threshold
    snakeDirection = 3; // Up (joystick LEFT)
  } else if (x > (xpinval + JOYSTICK_THRESHOLD) && lastDirection != 3) {  // Use customizable threshold
    snakeDirection = 4; // Down (joystick RIGHT)
  }
  
  // Move snake at customizable speed
  if (millis() - lastSnakeMove > SNAKE_SPEED) {  // Use customizable snake speed
    moveSnake();
    drawSnake();
    lastSnakeMove = millis();
    lastDirection = snakeDirection;
  }
}

void moveSnake() {
  // Convert current head from pixel to X,Y coordinates
  int currentX, currentY;
  currentY = snakeHead / 16;  // Row
  if (currentY % 2 == 0) {
    currentX = snakeHead % 16;
  } else {
    currentX = 15 - (snakeHead % 16);
  }
  
  // Calculate new X,Y position
  int newX = currentX;
  int newY = currentY;
  
  switch(snakeDirection) {
    case 1: if (currentX == 0) { snakeGameOver(); return; } newX = currentX - 1; break;
    case 2: if (currentX == 15) { snakeGameOver(); return; } newX = currentX + 1; break;
    case 3: if (currentY == 0) { snakeGameOver(); return; } newY = currentY - 1; break;
    case 4: if (currentY == 15) { snakeGameOver(); return; } newY = currentY + 1; break;
  }
  
  // Convert new X,Y back to pixel index
  int newHead = xyToPixel(newX, newY);
  
  // Check collision with self
  for(int i = 0; i < snakeLength; i++) {
    if (snake[i] == newHead) {
      snakeGameOver();
      return;
    }
  }
  
  // Move body
  for(int i = snakeLength - 1; i > 0; i--) {
    snake[i] = snake[i - 1];
  }
  snake[0] = newHead;
  snakeHead = newHead;
  
  // Check apple
  if (snakeHead == apple && snakeLength < 49) {
    snakeLength++;
    apple = random(0, 256);
    Serial.print("Snake score: ");
    Serial.println(snakeLength - 2);
  }
}

void drawSnake() {
  clearMatrix();
  matrix.setPixelColor(apple, matrix.Color(255, 0, 0));
  for(int i = 0; i < snakeLength; i++) {
    matrix.setPixelColor(snake[i], matrix.Color(0, 255, 0));
  }
  matrix.show();
}

void snakeGameOver() {
  int finalScore = snakeLength - 2;
  Serial.print("Snake final score: ");
  Serial.println(finalScore);
  showNumber(finalScore);
  delay(5000);  // Show score for 5 seconds instead of 3
  currentGameState = GAME_OVER_STATE;
}

// ============== SCORE DISPLAY ==============
void showNumber(int number) {
  clearMatrix();
  
  Serial.print("Displaying score: ");
  Serial.println(number);
  
  // Ensure number is valid
  if (number < 0) number = 0;
  
  // Simple way to display score as a number (0-9)
  if (number < 10) {
    // Display single digit score in the center
    drawNumber(number, 6, 4, matrix.Color(255, 0, 0));
  } else {
    // For scores 10+, show tens digit and ones digit
    int tens = number / 10;
    int ones = number % 10;
    drawNumber(tens, 3, 4, matrix.Color(255, 0, 0));  // Tens digit on left
    drawNumber(ones, 9, 4, matrix.Color(255, 0, 0));  // Ones digit on right
  }
  
  matrix.show();
}

// Function to draw simple numbers 0-9 using a 3x5 pixel font
void drawNumber(int number, int startX, int startY, uint32_t color) {
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
        setPixel(startX + x, startY + y, color);
      }
    }
  }
}

void showScore(int score) {
  // Keep old function for compatibility
  showNumber(score);
}

// ============== GAME OVER ==============
void handleGameOver() {
  clearMatrix();
  

  // Return to menu
  currentGameState = MENU;
  selectedGame = 0;
  showMenu();
  Serial.println("Returning to menu");
}
