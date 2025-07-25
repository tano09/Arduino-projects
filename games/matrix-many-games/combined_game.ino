#include <Adafruit_NeoPixel.h>

// Pin definitions
#define BUTTON_PIN 11
#define MATRIX_PIN 12
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
  PONG,
  GAME_OVER_STATE
};

GameState currentGameState = MENU;
int selectedGame = 0; // 0 = Flappy Bird, 1 = Snake, 2 = Pong

// Create matrix object - ONLY Adafruit_NeoPixel like working test
Adafruit_NeoPixel matrix = Adafruit_NeoPixel(256, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

// Shared variables
bool buttonPressed = false;
bool lastButtonState = false;
unsigned long lastButtonTime = 0;  // For button debouncing
const unsigned long BUTTON_DEBOUNCE_DELAY = 50;  // 50ms debounce delay



//GAME SETTINGS
//MODIFY THESE SETTINGS TO CUSTOMIZE YOUR GAME HERE
// ============== CONTROL SETTINGS ==============
const bool INVERT_MENU_X_AXIS = true;      // Set to false for normal menu X-axis, true for inverted menu X-axis
const bool INVERT_GAME_X_AXIS = true;     // Set to false for normal game X-axis, true for inverted game X-axis
const bool INVERT_SNAKE_Y_AXIS = true;    // Set to false for normal snake Y-axis, true for inverted snake Y-axis
const bool INVERT_PONG_Y_AXIS = true;     // Set to false for normal pong Y-axis, true for inverted pong Y-axis
const int JOYSTICK_THRESHOLD = 75;   // How far to move joystick (lower = more sensitive)

//================= FLAPPY BIRD GAME SETTINGS ==============
const float GRAVITY = -0.25;           // How fast bird falls (more negative = faster fall)
const float JUMP_STRENGTH = 0.75;     // How high bird jumps (higher = bigger jumps)
const int GAME_SPEED = 200;           // Game update speed in ms (lower = faster game)
const int PIPE_GAP_SIZE = 4;          // Height of gap in pipes (bigger = easier)
const int PIPE_WIDTH = 2;             // Width of pipes (1 or 2 pixels)
// ================ SNAKE GAME SETTINGS ==============
const int SNAKE_SPEED = 600;          // Snake move speed in ms (lower = faster snake)
// ================ PONG GAME SETTINGS ==============
const int PONG_SPEED = 100;           // Game update speed in ms (lower = faster)
const float BALL_SPEED_INCREASE = 1.05; // Ball speeds up by this factor when hit
const int PADDLE_SIZE = 4;            // Height of paddle in pixels
const int PADDLE_MOVE_DELAY = 50;     // Paddle movement delay in ms (lower = more responsive)
//brightness
const int BRIGHTNESS = 15; // brightness (0-255)

//end game settings


// ============== FLAPPY BIRD VARIABLES ==============
float birdY = 8.0;
float birdVelocity = 0.0;
int birdX = 3;
int pipeX = 16;
int pipeGapY = 6;
int flappyScore = 0;
bool flappyGameRunning = false;
unsigned long lastFlappyUpdate = 0;
// ============== SNAKE VARIABLES - OPTIMIZED ==============
int snake[50];  // Reduced size to save memory
int snakeLength = 2;
int snakeHead = 135;
int apple = 100;
int snakeDirection = 2;
int lastDirection = 1;
int xpinval = 512, ypinval = 512;
unsigned long lastSnakeMove = 0;
// ============== PONG VARIABLES ==============
float ballX = 8.0, ballY = 8.0;
float ballVelX = 0.8, ballVelY = 0.6;
int paddleY = 6;  // Paddle position (3 pixels tall)
int pongScore = 0;
bool pongGameRunning = false;
unsigned long lastPongUpdate = 0;
unsigned long lastPaddleMove = 0;  // For finer paddle control





// Helper functions - using only NeoPixel methods
void clearMatrix() {
  for (int i = 0; i < 256; i++) {
    matrix.setPixelColor(i, 0); // Black - using 0 instead of matrix.Color(0,0,0)
  }
}

// Convert X,Y to pixel index for 16x16 zigzag matrix - ROTATED 90° CLOCKWISE
int xyToPixel(int x, int y) {
  // Rotate coordinates 90° clockwise: new_x = y, new_y = 15-x
  int rotated_x = y;
  int rotated_y = 15 - x;
  
  if (rotated_x < 0 || rotated_x >= 16 || rotated_y < 0 || rotated_y >= 16) return -1;
  
  int pixel;
  if (rotated_y % 2 == 0) {
    // Even rows: left to right
    pixel = rotated_y * 16 + rotated_x;
  } else {
    // Odd rows: right to left
    pixel = rotated_y * 16 + (15 - rotated_x);
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
  
  // Initialize pins
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  // Initialize matrix
  matrix.begin();
  matrix.setBrightness(BRIGHTNESS);
  matrix.show();
  
  // Calibrate joystick
  for(int i = 0; i < 10; i++) {
    xpinval += analogRead(JOYSTICK_XPIN);
    ypinval += analogRead(JOYSTICK_YPIN);
    delay(10);
  }
  xpinval /= 10;
  ypinval /= 10;
  
  currentGameState = MENU;
  showMenu();
}

void loop() {
  // Read button with debouncing
  bool currentButtonState = !digitalRead(BUTTON_PIN);
  
  if (currentButtonState != lastButtonState) {
    lastButtonTime = millis();
    lastButtonState = currentButtonState;
  }
  
  if ((millis() - lastButtonTime) > BUTTON_DEBOUNCE_DELAY) {
    if (currentButtonState && !buttonPressed) {
      buttonPressed = true;
    } else if (!currentButtonState) {
      buttonPressed = false;
    }
  }
  
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
    case PONG:
      handlePong();
      break;
    case GAME_OVER_STATE:
      handleGameOver();
      break;
  }
}

// ============== MENU FUNCTIONS ==============
void showMenu() {
  clearMatrix();
  
  // Left column white
  for(int y = 0; y < 16; y++) {
    setPixel(0, y, matrix.Color(255, 255, 255));
  }
  
  // Flappy Bird section
  uint32_t flappyBgColor = (selectedGame == 0) ? matrix.Color(50, 0, 0) : matrix.Color(0, 0, 0);
  for(int x = 2; x < 6; x++) {
    for(int y = 0; y < 16; y++) {
      setPixel(x, y, flappyBgColor);
    }
  }
  setPixel(3, 3, matrix.Color(255, 255, 0));
  setPixel(4, 3, matrix.Color(255, 255, 0));
  setPixel(2, 8, matrix.Color(0, 255, 0));
  setPixel(5, 8, matrix.Color(0, 255, 0));
  setPixel(2, 9, matrix.Color(0, 255, 0));
  setPixel(5, 9, matrix.Color(0, 255, 0));
  
  // Snake section
  uint32_t snakeBgColor = (selectedGame == 1) ? matrix.Color(50, 0, 0) : matrix.Color(0, 0, 0);
  for(int x = 6; x < 10; x++) {
    for(int y = 0; y < 16; y++) {
      setPixel(x, y, snakeBgColor);
    }
  }
  setPixel(7, 5, matrix.Color(0, 255, 0));
  setPixel(7, 4, matrix.Color(0, 255, 0));
  setPixel(7, 3, matrix.Color(0, 255, 0));
  setPixel(8, 8, matrix.Color(255, 0, 0));
  
  // Pong section
  uint32_t pongBgColor = (selectedGame == 2) ? matrix.Color(50, 0, 0) : matrix.Color(0, 0, 0);
  for(int x = 10; x < 14; x++) {
    for(int y = 0; y < 16; y++) {
      setPixel(x, y, pongBgColor);
    }
  }
  setPixel(10, 6, matrix.Color(0, 255, 255));
  setPixel(10, 7, matrix.Color(0, 255, 255));
  setPixel(10, 8, matrix.Color(0, 255, 255));
  setPixel(12, 4, matrix.Color(255, 255, 255));
  
  // Right column blue
  for(int y = 0; y < 16; y++) {
    setPixel(15, y, matrix.Color(0, 0, 255));
  }
  
  matrix.show();
}

void handleMenu() {
  int x = analogRead(JOYSTICK_XPIN);
  
  // Menu navigation with separate X-axis inversion for menu only
  if (INVERT_MENU_X_AXIS) {
    // Inverted menu controls
    if (x < (xpinval - JOYSTICK_THRESHOLD)) {
      if (selectedGame > 0) {
        selectedGame--;
        showMenu();
        delay(200);
      }
    } else if (x > (xpinval + JOYSTICK_THRESHOLD)) {
      if (selectedGame < 2) {
        selectedGame++;
        showMenu();
        delay(200);
      }
    }
  } else {
    // Normal menu controls
    if (x > (xpinval + JOYSTICK_THRESHOLD)) {
      if (selectedGame > 0) {
        selectedGame--;
        showMenu();
        delay(200);
      }
    } else if (x < (xpinval - JOYSTICK_THRESHOLD)) {
      if (selectedGame < 2) {
        selectedGame++;
        showMenu();
        delay(200);
      }
    }
  }
  
  if (buttonPressed) {
    if (selectedGame == 0) {
      currentGameState = FLAPPY_BIRD;
      resetFlappyBird();
    } else if (selectedGame == 1) {
      currentGameState = SNAKE;
      resetSnake();
    } else if (selectedGame == 2) {
      currentGameState = PONG;
      resetPong();
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
    if (pipeX < -PIPE_WIDTH) {
      pipeX = 16;
      pipeGapY = random(2, 16 - PIPE_GAP_SIZE - 2);
      flappyScore++;
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
  showNumber(flappyScore);
  delay(3000);
  currentGameState = GAME_OVER_STATE;
}

// ============== SNAKE FUNCTIONS - CONVERTED TO NEOPIXEL ONLY ==============
void resetSnake() {
  snakeLength = 2;
  int startX = 8, startY = 8;
  snakeHead = xyToPixel(startX, startY);
  snake[0] = snakeHead;
  snake[1] = xyToPixel(startX - 1, startY);
  spawnNewApple();
  snakeDirection = 2;
  lastDirection = 2;
  lastSnakeMove = millis();
  drawSnake();
}

void handleSnake() {
  int x = analogRead(JOYSTICK_XPIN);
  int y = analogRead(JOYSTICK_YPIN);
  
  // Change direction based on joystick with separate game axis inversions
  // Y-axis controls (with inversion option)
  if ((INVERT_SNAKE_Y_AXIS && y < (ypinval - JOYSTICK_THRESHOLD)) || (!INVERT_SNAKE_Y_AXIS && y > (ypinval + JOYSTICK_THRESHOLD))) {
    if (lastDirection != 4) snakeDirection = 4; // Left
  } else if ((INVERT_SNAKE_Y_AXIS && y > (ypinval + JOYSTICK_THRESHOLD)) || (!INVERT_SNAKE_Y_AXIS && y < (ypinval - JOYSTICK_THRESHOLD))) {
    if (lastDirection != 3) snakeDirection = 3; // Right
  }
  // X-axis controls (with inversion option)
  else if ((INVERT_GAME_X_AXIS && x < (xpinval - JOYSTICK_THRESHOLD)) || (!INVERT_GAME_X_AXIS && x > (xpinval + JOYSTICK_THRESHOLD))) {
    if (lastDirection != 1) snakeDirection = 1; // Up
  } else if ((INVERT_GAME_X_AXIS && x > (xpinval + JOYSTICK_THRESHOLD)) || (!INVERT_GAME_X_AXIS && x < (xpinval - JOYSTICK_THRESHOLD))) {
    if (lastDirection != 2) snakeDirection = 2; // Down
  }
  
  // Move snake at customizable speed
  if (millis() - lastSnakeMove > SNAKE_SPEED) {
    moveSnake();
    drawSnake();
    lastSnakeMove = millis();
    lastDirection = snakeDirection;
  }
}

void moveSnake() {
  // Convert current head from pixel to X,Y coordinates (accounting for rotation)
  int currentX, currentY;
  
  // Reverse the rotation: if rotated_x = y and rotated_y = 15-x, then x = 15-rotated_y and y = rotated_x
  // First, get the rotated coordinates from pixel index
  int rotated_y = snakeHead / 16;  // Row
  int rotated_x;
  if (rotated_y % 2 == 0) {
    rotated_x = snakeHead % 16;
  } else {
    rotated_x = 15 - (snakeHead % 16);
  }
  
  // Convert back to original coordinates
  currentX = 15 - rotated_y;
  currentY = rotated_x;
  
  // Calculate new X,Y position
  int newX = currentX;
  int newY = currentY;
  
  switch(snakeDirection) {
    case 1: if (currentX <= 0) { snakeGameOver(); return; } newX = currentX - 1; break;
    case 2: if (currentX >= 15) { snakeGameOver(); return; } newX = currentX + 1; break;
    case 3: if (currentY <= 0) { snakeGameOver(); return; } newY = currentY - 1; break;
    case 4: if (currentY >= 15) { snakeGameOver(); return; } newY = currentY + 1; break;
  }
  
  // Convert new X,Y back to pixel index (using rotated xyToPixel)
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
    spawnNewApple();
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
  showNumber(finalScore);
  delay(3000);
  currentGameState = GAME_OVER_STATE;
}

// Function to spawn apple in empty location
void spawnNewApple() {
  bool validPosition = false;
  int attempts = 0;
  
  while (!validPosition && attempts < 100) {
    apple = random(0, 256);
    validPosition = true;
    
    // Check if apple position conflicts with any snake segment
    for (int i = 0; i < snakeLength; i++) {
      if (snake[i] == apple) {
        validPosition = false;
        break;
      }
    }
    attempts++;
  }
  
  // Fallback: if we can't find a good spot after 100 tries, use random anyway
  if (!validPosition) {
    apple = random(0, 256);
  }
}

// ============== PONG FUNCTIONS ==============
void resetPong() {
  ballX = 8.0;
  ballY = 8.0;
  ballVelX = 0.8;
  ballVelY = 0.6;
  paddleY = 6;  // Center paddle (3 pixels tall, so 6,7,8)
  pongScore = 0;
  pongGameRunning = true;
  lastPongUpdate = millis();
  lastPaddleMove = millis();  // Reset paddle movement timer
}

void handlePong() {
  int x = analogRead(JOYSTICK_XPIN);
  int y = analogRead(JOYSTICK_YPIN);
  
  // Paddle control with movement delay and separate game axis inversions
  if (millis() - lastPaddleMove > PADDLE_MOVE_DELAY) {
    bool paddleMoved = false;
    
    // Move paddle based on joystick with separate game axis inversions
    // X-axis controls (with inversion option)
    if ((INVERT_GAME_X_AXIS && x < (xpinval - JOYSTICK_THRESHOLD)) || (!INVERT_GAME_X_AXIS && x > (xpinval + JOYSTICK_THRESHOLD))) {
      if (paddleY > 0) {
        paddleY--;
        paddleMoved = true;
      }
    } else if ((INVERT_GAME_X_AXIS && x > (xpinval + JOYSTICK_THRESHOLD)) || (!INVERT_GAME_X_AXIS && x < (xpinval - JOYSTICK_THRESHOLD))) {
      if (paddleY < (16 - PADDLE_SIZE)) {
        paddleY++;
        paddleMoved = true;
      }
    }
    // Y-axis controls (with inversion option)
    else if ((INVERT_PONG_Y_AXIS && y > (ypinval + JOYSTICK_THRESHOLD)) || (!INVERT_PONG_Y_AXIS && y < (ypinval - JOYSTICK_THRESHOLD))) {
      if (paddleY > 0) {
        paddleY--;
        paddleMoved = true;
      }
    } else if ((INVERT_PONG_Y_AXIS && y < (ypinval - JOYSTICK_THRESHOLD)) || (!INVERT_PONG_Y_AXIS && y > (ypinval + JOYSTICK_THRESHOLD))) {
      if (paddleY < (16 - PADDLE_SIZE)) {
        paddleY++;
        paddleMoved = true;
      }
    }
    
    if (paddleMoved) {
      lastPaddleMove = millis();
    }
  }
  
  if (millis() - lastPongUpdate > PONG_SPEED) {
    updatePong();
    drawPong();
    lastPongUpdate = millis();
  }
}

void updatePong() {
  // Move ball
  ballX += ballVelX;
  ballY += ballVelY;
  
  // Ball collision with top/bottom walls
  if (ballY <= 0 || ballY >= 15) {
    ballVelY = -ballVelY;
    ballY = constrain(ballY, 0, 15);
  }
  
  // Ball collision with right wall (bounces back)
  if (ballX >= 15) {
    ballVelX = -ballVelX;
    ballX = 15;
  }
  
  // Ball collision with paddle (left side)
  if (ballX <= 1 && ballVelX < 0) {
    // Check if ball hits paddle
    int ballIntY = (int)ballY;
    if (ballIntY >= paddleY && ballIntY < paddleY + PADDLE_SIZE) {
      ballVelX = -ballVelX * BALL_SPEED_INCREASE;
      ballVelY *= BALL_SPEED_INCREASE;
      ballX = 1;
      pongScore++;
    } else {
      // Missed paddle - game over
      pongGameOver();
      return;
    }
  }
  
  // Ball went off left side
  if (ballX < 0) {
    pongGameOver();
    return;
  }
}

void drawPong() {
  clearMatrix();
  
  // Draw paddle (left side, cyan)
  for(int i = 0; i < PADDLE_SIZE; i++) {
    setPixel(0, paddleY + i, matrix.Color(0, 255, 255));
  }
  
  // Draw ball (white)
  setPixel((int)ballX, (int)ballY, matrix.Color(255, 255, 255));
  
  matrix.show();
}

void pongGameOver() {
  pongGameRunning = false;
  showNumber(pongScore);
  delay(3000);
  currentGameState = GAME_OVER_STATE;
}

// ============== SCORE DISPLAY ==============
void showNumber(int number) {
  clearMatrix();
  
  if (number < 0) number = 0;
  
  if (number < 10) {
    // Single digit: display in center
    drawNumber(number, 6, 4, matrix.Color(255, 0, 0));
  } else if (number < 100) {
    // Two digits: display side by side
    int tens = number / 10;
    int ones = number % 10;
    drawNumber(tens, 3, 4, matrix.Color(255, 0, 0));  // Tens digit on left
    drawNumber(ones, 9, 4, matrix.Color(255, 0, 0));  // Ones digit on right
  } else {
    // Three digits: display compactly
    int hundreds = number / 100;
    int tens = (number / 10) % 10;
    int ones = number % 10;
    drawNumber(hundreds, 1, 4, matrix.Color(255, 0, 0));  // Hundreds digit on far left
    drawNumber(tens, 6, 4, matrix.Color(255, 0, 0));      // Tens digit in center
    drawNumber(ones, 11, 4, matrix.Color(255, 0, 0));     // Ones digit on far right
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
  currentGameState = MENU;
  selectedGame = 0;
  showMenu();
}
