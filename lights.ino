#include <Adafruit_NeoPixel.h>

// Define the pin and number of LEDs
#define LED_PIN 5
#define NUM_LEDS 29

// Initialize the Adafruit NeoPixel library
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  // Initialize the NeoPixel strip
  strip.begin();
  strip.show();  // Initialize all pixels to 'off'
}

void loop() {
  // Call the Northern Lights effect
  northernLightsEffect(50);  // Adjust the speed (delay in ms)
}

void northernLightsEffect(int speed) {
  // Turn off all LEDs first
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));  // Turn off
  }
  strip.show();

  // Gradually turn on LEDs with random green, blue, and a bit of red for Northern Lights effect
  for (int i = 0; i < NUM_LEDS; i++) {
    // Generate a base green color with random blue and a bit of random red
    int red = random(10, 100);      // Add a subtle amount of red (less dominant)
    int green = random(100, 255);  // Green is the dominant color
    int blue = random(50, 150);    // Add a subtle random mix of blue

    // Set the color with random red, green, and blue for northern lights effect
    strip.setPixelColor(i, strip.Color(red, green, blue));
    strip.show();

    delay(speed);  // Control speed of lighting up
  }

  // Optionally, add a fading effect or other animations after the lights are fully on
}
