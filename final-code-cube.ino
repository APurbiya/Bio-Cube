#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>

// OLED settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Rotary encoder pins
#define CLK_PIN 25
#define DT_PIN 26
#define SW_PIN 27

// LED Strip settings
#define LED_PIN_BASE 5
#define LED_PIN_TOP 2
#define NUM_LEDS 29
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN_BASE, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel stripTop = Adafruit_NeoPixel(9, LED_PIN_TOP, NEO_GRB + NEO_KHZ800);

// Sensor and effect pins
#define DHTPIN 19
#define LDR_PIN 12
#define DHTTYPE DHT22
#define LED_FOG_PIN 4
#define LED_HEATER_PIN 0
#define defaultGround 16
//#define LED_RAIN_PIN 2

DHT dht(DHTPIN, DHTTYPE);
ezButton button(SW_PIN);

int hour = 0;
int minute = 0;
int selection = 0; // 0 = hour, 1 = minute, 2 = confirm
int menuSelection = 0;  // Selection for modes or controls
int modesSelection = 0;  // Selection inside the modes screen
int counter = 0;
bool blinkState = true;
bool menuBlinkState = true;
bool inControlScreen = false;
bool inModesScreen = false;
int lightValue = analogRead(LDR_PIN);

// Flags for toggling effects
bool rainEffectActive = false;
bool fogEffectActive = false;
bool northernLightsActive = false;
bool dayCycleActive = false;

unsigned long previousMillis = 0;
unsigned long menuPreviousMillis = 0;
const long interval = 500;
const long menuInterval = 300;

int lastStateCLK;
int currentStateCLK;

void setup() {
  Serial.begin(9600);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  // Initialize Rotary encoder and button
  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  button.setDebounceTime(50);
  lastStateCLK = digitalRead(CLK_PIN);

  // Initialize DHT sensor
  dht.begin();
  
  // Initialize LED strip
  strip.begin();
  strip.show();
  setAllLEDsToRed();

  // Initialize effect pins
  pinMode(LED_FOG_PIN, OUTPUT);
  pinMode(LED_HEATER_PIN, OUTPUT);
  pinMode(defaultGround, OUTPUT);
  //pinMode(LED_RAIN_PIN, OUTPUT);
  digitalWrite(LED_HEATER_PIN, LOW);
  digitalWrite(defaultGround, LOW);
  
  digitalWrite(LED_FOG_PIN, HIGH);
}

void loop() {
  button.loop();
  unsigned long currentMillis = millis();

  // Rotary encoder handling
  currentStateCLK = digitalRead(CLK_PIN);
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if (digitalRead(DT_PIN) == LOW) {
      if (!inControlScreen && !inModesScreen) {
        // Time setting mode
        if (selection == 0) hour = (hour + 1) % 24;
        else if (selection == 1) minute = (minute + 1) % 60;
      } else if (inModesScreen) {
        // Mode selection navigation
        modesSelection = (modesSelection + 1) % 5;  // There are 5 mode options
      }
    } else {
      if (!inControlScreen && !inModesScreen) {
        if (selection == 0) hour = (hour - 1 + 24) % 24;
        else if (selection == 1) minute = (minute - 1 + 60) % 60;
      } else if (inModesScreen) {
        modesSelection = (modesSelection - 1 + 5) % 5;
      }
    }
  }
  lastStateCLK = currentStateCLK;
  lightValue = analogRead(LDR_PIN);
  if (northernLightsActive && counter == 900) 
  {
    northernLightsEffect(random(25, 100));
    //delay(random(200, 700));
  }
  if (dayCycleActive) 
  {
    setDayCycleEffect(lightValue);
    //delay(random(200, 700));
  }
  // Button press handling
  if (button.isPressed()) {
    if (!inControlScreen && !inModesScreen) {
      // Time-setting screen logic
      if (selection == 2) {
        inModesScreen = true;  // Enter main menu
        selection = 0;         // Reset the selection index
      } else {
        selection++;  // Move to the next field (hour, minute, confirm)
      }
    } else if (inModesScreen) {
      // Toggle the selected mode (rain, fog, etc.)
      toggleMode();  
    } else if (inControlScreen) {
      // Exit the control screen when button is pressed in control mode
      inControlScreen = false;
      inModesScreen = true;
    }
  }

  // Blinking control for time and menu
  if (!inControlScreen && !inModesScreen && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    blinkState = !blinkState;
  }
  if ((inControlScreen || inModesScreen) && currentMillis - menuPreviousMillis >= menuInterval) {
    menuPreviousMillis = currentMillis;
    menuBlinkState = !menuBlinkState;
  }

  // Display the respective screen based on the current mode
  if (!inControlScreen && !inModesScreen) {
    showTimeScreen();
  } else if (inModesScreen) {
    showModesScreen();
  } else if (inControlScreen) {
    showControlsScreen();
  }

  counter++;
}

// Display the controls screen with sensor data
void showControlsScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  lightValue = analogRead(LDR_PIN);

  display.setCursor(0, 0);
  if (isnan(temp)) {
    display.print("Temp: Error");
  } else {
    display.print("Temp: "); 
    display.print(temp); 
    display.print(" C");
  }

  display.setCursor(0, 10);
  if (isnan(humidity)) {
    display.print("Humidity: Error");
  } else {
    display.print("Humidity: "); 
    display.print(humidity); 
    display.print(" %");
  }

  display.setCursor(0, 20);
  display.print("Light: "); 
  display.print(lightValue);

  display.display();
}

// Display the time-setting screen
void showTimeScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  display.print("SET TIME");
  display.setCursor(20, 40);

  if (selection == 0 && blinkState) display.print("  :");
  else display.print((hour < 10 ? "0" : "") + String(hour) + ":");

  if (selection == 1 && blinkState) display.print("  ");
  else display.print((minute < 10 ? "0" : "") + String(minute));

  display.display();
}

// Display the modes screen with scrolling
void showModesScreen() {
    const int optionsCount = 5;
    String options[optionsCount] = {"Rain", "Fog", "Northern Lights", "Day Cycle", "Controls"};
    bool isActive[optionsCount] = {rainEffectActive, fogEffectActive, northernLightsActive, dayCycleActive, false};

    display.clearDisplay();
    display.setTextSize(1);

    for (int i = 0; i < optionsCount; i++) {
        display.setCursor(10, i * 10);
        if (modesSelection == i && menuBlinkState) display.print("> ");
        display.print(options[i]);
        if (isActive[i]) display.print(" [X]");
    }

    display.display();
}

// Toggle the selected mode (Rain, Fog, Northern Lights, Day Cycle)
void toggleMode() {
  switch (modesSelection) {
    case 0:  // Rain
      rainEffectActive = !rainEffectActive;
      if(rainEffectActive)
      {
        runRainEffect(true);
      }
      else
      {
        runRainEffect(false);
      }
      //digitalWrite(LED_HEATER_PIN, rainEffectActive ? HIGH : LOW);
      break;
    case 1:  // Fog
      fogEffectActive = !fogEffectActive;
      if(fogEffectActive)
      {
        runFogEffect(true);
      }
      else
      {
        runFogEffect(false);
      }
      //digitalWrite(LED_FOG_PIN, fogEffectActive ? HIGH : LOW);
      break;
    case 2:  // Northern Lights
      northernLightsActive = !northernLightsActive;
      if (northernLightsActive) northernLightsEffect(50);
      else setAllLEDsToRed();
      break;
    case 3:  // Day Cycle
      dayCycleActive = !dayCycleActive;
      if (dayCycleActive) setDayCycleEffect(0);
      else bassOff();
      break;
    case 4:  // Controls Screen
      inModesScreen = false;
      inControlScreen = true;
      break;
  }
}

// Run the rain effect
void runRainEffect(bool state) {
  digitalWrite(LED_HEATER_PIN, state ? HIGH : LOW);
}

// Run the fog effect
void runFogEffect(bool state) {
  
  if(state == true) {
    // Corresponds to serial input '1'
    digitalWrite(LED_FOG_PIN, LOW);   // Turn fogger ON
    delay(1000);                      // Keep it ON for 1 second
    digitalWrite(LED_FOG_PIN, HIGH);  // Turn fogger OFF
  }
  else {
    // Corresponds to serial input '0'
    digitalWrite(LED_FOG_PIN, LOW);   // Turn fogger ON
    delay(500);                       // Keep it ON for 0.5 second
    digitalWrite(LED_FOG_PIN, HIGH);  // Turn fogger OFF for 0.5 second
    delay(500);
    digitalWrite(LED_FOG_PIN, LOW);   // Turn fogger ON for 0.5 second again
    delay(500);
    digitalWrite(LED_FOG_PIN, HIGH);  // Turn fogger OFF
  }
}

// Simulate the Northern Lights effect
void northernLightsEffect(int brightness) {
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(random(0, 255), random(0, 255), random(0, 255)));
  }
  strip.setBrightness(brightness);
  strip.show();
}

// Placeholder for Day Cycle effect
void setDayCycleEffect(int poss) {
  //poss is the intensity of light, 0 means its brightest, and 4000 means its really dark like darkest!
  if(poss <= 3000)
  {
    for (int i = 0; i < stripTop.numPixels(); i++) 
    {
      stripTop.setPixelColor(i, stripTop.Color(255, 255, 0));  // Yellow to simulate daylight
    }
  }
  if(poss < 3000 && poss <= 2500)
  {
    for (int i = 0; i < stripTop.numPixels() - 3; i++) 
    {
      stripTop.setPixelColor(i, stripTop.Color(100, 255, 0));  // Yellow to simulate daylight
    }
    for (int i = stripTop.numPixels() - 3; i < stripTop.numPixels(); i++) 
    {
      stripTop.setPixelColor(i, stripTop.Color(100, 20, 0));  // Yellow to simulate daylight
    }
  }
  if(poss > 2500)
  {
    for (int i = 0; i < stripTop.numPixels(); i++) 
    {
      stripTop.setPixelColor(i, stripTop.Color(200, 20, 20));  // Yellow to simulate daylight
    }
  }
  
  
  stripTop.show();
}

// Set all LEDs to red (default)
void setAllLEDsToRed() {
  for (int i = 0; i < strip.numPixels(); i+= 2) {
    strip.setPixelColor(i, strip.Color(255, 255, 220));
  }
  strip.show();
}

void bassOff() {
  for (int i = 0; i < stripTop.numPixels(); i++) {
    stripTop.setPixelColor(i, stripTop.Color(0, 0, 0));
  }
  stripTop.show();
}
