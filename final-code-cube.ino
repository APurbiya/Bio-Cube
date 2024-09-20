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

// Sensor and effect pins
#define DHTPIN 19
#define LDR_PIN 12
#define DHTTYPE DHT22
#define LED_FOG_PIN 4
#define LED_HEATER_PIN 0
#define LED_RAIN_PIN 2

DHT dht(DHTPIN, DHTTYPE);

ezButton button(SW_PIN);
int hour = 0;
int minute = 0;
int selection = 0; // 0 = hour, 1 = minute, 2 = confirm
int menuSelection = 0;  // Selection for modes or controls
int modesSelection = 0;  // Selection inside the modes screen
bool blinkState = true;
bool menuBlinkState = true;
bool inControlScreen = false;
bool inModesScreen = false;

// Flags for toggling effects
bool rainEffectActive = false;
bool fogEffectActive = false;
bool northernLightsActive = false;

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
  pinMode(LED_RAIN_PIN, OUTPUT);
}

void loop() {
  button.loop();
  unsigned long currentMillis = millis();

  // Rotary encoder handling
  currentStateCLK = digitalRead(CLK_PIN);
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if (digitalRead(DT_PIN) == LOW) {
      if (!inControlScreen && !inModesScreen) {
        if (selection == 0) hour = (hour + 1) % 24;
        else if (selection == 1) minute = (minute + 1) % 60;
      } else if (inModesScreen) {
        modesSelection = (modesSelection + 1) % 5;  // There are 5 mode options
      } else {
        menuSelection = (menuSelection + 1) % 2;  // Modes or Controls
      }
    } else {
      if (!inControlScreen && !inModesScreen) {
        if (selection == 0) hour = (hour - 1 + 24) % 24;
        else if (selection == 1) minute = (minute - 1 + 60) % 60;
      } else if (inModesScreen) {
        modesSelection = (modesSelection - 1 + 5) % 5;
      } else {
        menuSelection = (menuSelection - 1 + 2) % 2;
      }
    }
  }
  lastStateCLK = currentStateCLK;

  // Button press handling
  if (button.isPressed()) {
    Serial.println("button press");
    if (!inControlScreen && !inModesScreen) {
      if (selection == 2) {
        inControlScreen = true;
        selection = 0;
      } else {
        selection++;
      }
    } else if (inModesScreen) {
      toggleMode();  // Toggle the selected mode
    } else {
      if (menuSelection == 1) {
        inControlScreen = false;
        showControlsScreen();
      } else {
        inModesScreen = true;
      }
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

  // Show respective screens
  if (!inControlScreen && !inModesScreen) showTimeScreen();
  else if (inModesScreen) showModesScreen();
  else showControlsScreen();
}

// Display the time-setting screen
void showTimeScreen() {
  Serial.println("inTime Screen");
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  display.print("TIME?");
  display.setCursor(20, 40);

  if (selection == 0 && blinkState) display.print("  :");
  else display.print((hour < 10 ? "0" : "") + String(hour) + ":");

  if (selection == 1 && blinkState) display.print("  ");
  else display.print((minute < 10 ? "0" : "") + String(minute));

  display.display();
}

// Display the modes screen with scrolling
void showModesScreen() {
  Serial.println("inMode Screen");
  const int optionsCount = 5;  // Total number of options
  String options[optionsCount] = {"Rain", "Fog", "Northern Lights", "Day Cycle", "Controls"};
  bool isActive[optionsCount] = {rainEffectActive, fogEffectActive, northernLightsActive, false, false};

  // Determine which subset of options to display based on the current selection
  int start = modesSelection / 3 * 3;
  int end = min(start + 3, optionsCount);

  display.clearDisplay();
  display.setTextSize(2);

  // Loop to display the visible subset of options
  for (int i = start; i < end; i++) {
    display.setCursor(10, (i - start) * 20 + 10);
    if (modesSelection == i && menuBlinkState) display.print("> ");
    display.print(options[i]);
    if (isActive[i]) display.print(" [X]");
  }

  display.display();
}

// Display the controls screen with sensor data or error messages
// Display the controls screen with sensor data or error messages
void showControlsScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightValue = analogRead(LDR_PIN);

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
  if (lightValue < 0) {
    display.print("Light: Error");
  } else {
    display.print("Light: "); 
    display.print(lightValue);
  }

  // Serial printing of light intensity values
  Serial.print("Light Intensity: ");
  Serial.println(lightValue);

  display.display();
}


// Toggle the selected mode (Rain, Fog, Northern Lights)
void toggleMode() {
  switch (modesSelection) {
    case 0:  // Rain
      rainEffectActive = !rainEffectActive;
      if (rainEffectActive) runRainEffect();
      else digitalWrite(LED_RAIN_PIN, LOW);
      break;
    case 1:  // Fog
      fogEffectActive = !fogEffectActive;
      if (fogEffectActive) runFogEffect();
      else digitalWrite(LED_FOG_PIN, LOW);
      break;
    case 2:  // Northern Lights
      northernLightsActive = !northernLightsActive;
      if (northernLightsActive) northernLightsEffect(50);
      else setAllLEDsToRed();
      break;
  }
}

// Run the rain effect
void runRainEffect() {
  digitalWrite(LED_RAIN_PIN, HIGH);
}

// Run the fog effect
void runFogEffect() {
  digitalWrite(LED_FOG_PIN, HIGH);
}

// Set all LEDs to red
void setAllLEDsToRed() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
}

// Northern lights effect
void northernLightsEffect(int speed) {
  uint32_t colors[] = {strip.Color(0, 255, 0), strip.Color(0, 0, 255), strip.Color(255, 0, 255)};
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, colors[random(0, 3)]);
    strip.show();
    delay(speed);
  }
}
