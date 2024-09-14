void northernLightsEffect(int speed) {
  // Turn off all LEDs first
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0)); // Turn off
  }
  strip.show();

  // Gradually turn on LEDs with random green and blue mix for Northern Lights effect
  for (int i = 0; i < NUM_LEDS; i++) {
    // Generate a base green color with random blue value
    int green = random(100, 255);  // Green is the dominant color
    int blue = random(50, 150);    // Add a subtle random mix of blue

    // Set the color with more green and some random blue for northern lights effect
    strip.setPixelColor(i, strip.Color(0, green, blue));
    strip.show();
    
    delay(speed);  // Control speed of lighting up
  }
}
