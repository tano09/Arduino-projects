// RGB Spectrum Cycling Program
// Red LED on pin 9, Green LED on pin 10, Blue LED on pin 11

// Pin definitions
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

// Variables for color cycling
int colorStep = 0;
int fadeSpeed = 5; // Delay between color changes (milliseconds)

void setup() {
  // Initialize pins as outputs
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  

}

void loop() {
  // Cycle through the RGB spectrum
  cycleRGBSpectrum();
  
  // Small delay for smooth transitions
  delay(fadeSpeed);
}

void cycleRGBSpectrum() {
  int red, green, blue;
  
  // Calculate RGB values based on current step in the spectrum
  // The spectrum is divided into 6 phases of 255 steps each (total 1530 steps)
  
  if (colorStep < 255) {
    // Phase 1: Red to Yellow (Red stays 255, Green increases)
    red = 255;
    green = colorStep;
    blue = 0;
  }
  else if (colorStep < 510) {
    // Phase 2: Yellow to Green (Red decreases, Green stays 255)
    red = 255 - (colorStep - 255);
    green = 255;
    blue = 0;
  }
  else if (colorStep < 765) {
    // Phase 3: Green to Cyan (Green stays 255, Blue increases)
    red = 0;
    green = 255;
    blue = colorStep - 510;
  }
  else if (colorStep < 1020) {
    // Phase 4: Cyan to Blue (Green decreases, Blue stays 255)
    red = 0;
    green = 255 - (colorStep - 765);
    blue = 255;
  }
  else if (colorStep < 1275) {
    // Phase 5: Blue to Magenta (Blue stays 255, Red increases)
    red = colorStep - 1020;
    green = 0;
    blue = 255;
  }
  else {
    // Phase 6: Magenta to Red (Blue decreases, Red stays 255)
    red = 255;
    green = 0;
    blue = 255 - (colorStep - 1275);
  }
  
  // Write PWM values to the LED pins
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);

  // Increment color step and reset when complete cycle is done
  colorStep++;
  if (colorStep >= 1530) {
    colorStep = 0;
  }
}

