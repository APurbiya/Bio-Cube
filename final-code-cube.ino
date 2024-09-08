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
#define LED_PIN 5
#define NUM_LEDS 29
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Sensor and effect pins
#define DHTPIN 19
#define LDR_PIN 13
#define DHTTYPE DHT22
#define LED_FOG_PIN 4 
#define LED_HEATER_PIN 0 
#define LED_RAIN_PIN 2 

DHT dht(DHTPIN, DHTTYPE);

ezButton button(SW_PIN);
int hour = 0;
int minute = 0;
int selection = 0;  // 0 = hour, 1 = minute, 2 = confirm
int menuSelection = 0;  // 0 = Modes, 1 = Controls
int modesSelection = 0;  // 0 = Rain, 1 = Fog, 2 = Northern Lights
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
        modesSelection = (modesSelection + 1) % 3;  
      } else {
        menuSelection = (menuSelection + 1) % 2;  
      }
    } else {
      if (!inControlScreen && !inModesScreen) {
        if (selection == 0) hour = (hour - 1 + 24) % 24;
        else if (selection == 1) minute = (minute - 1 + 60) % 60;
      } else if (inModesScreen) {
        modesSelection = (modesSelection - 1 + 3) % 3;  
      } else {
        menuSelection = (menuSelection - 1 + 2) % 2;  
      }
    }
  }
  lastStateCLK = currentStateCLK;

  // Button press handling
  if (button.isPressed()) {
    if (!inControlScreen && !inModesScreen) {
      if (selection == 2) {  
        inControlScreen = true;  
        selection = 0;  
      } else {
        selection++;
      }
    } else if (inModesScreen) {
      toggleMode();  // Toggle the mode selected
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

void showTimeScreen() {
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

void showModesScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  
  display.setCursor(10, 10);
  if (modesSelection == 0 && menuBlinkState) display.print("> ");
  display.print("Rain");
  if (rainEffectActive) display.print(" [X]");
  
  display.setCursor(10, 30);
  if (modesSelection == 1 && menuBlinkState) display.print("> ");
  display.print("Fog");
  if (fogEffectActive) display.print(" [X]");
  
  display.setCursor(10, 50);
  if (modesSelection == 2 && menuBlinkState) display.print("> ");
  display.print("Northern Lights");
  if (northernLightsActive) display.print(" [X]");
  
  display.display();
}

void showControlsScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightValue = analogRead(LDR_PIN);

  display.setCursor(0, 0);
  display.print("Temp: "); display.print(temp); display.print(" C");
  display.setCursor(0, 10);
  display.print("Humidity: "); display.print(humidity); display.print(" %");
  display.setCursor(0, 20);
  display.print("Light: "); display.print(lightValue);

  display.display();
}

// Toggle selected mode
void toggleMode() {
  switch (modesSelection) {
    case 0: 
      rainEffectActive = !rainEffectActive;
      if (rainEffectActive) runRainEffect();
      else digitalWrite(LED_RAIN_PIN, LOW);
      break;
    case 1: 
      fogEffectActive = !fogEffectActive;
      if (fogEffectActive) runFogEffect();
      else digitalWrite(LED_FOG_PIN, LOW);
      break;
    case 2: 
      northernLightsActive = !northernLightsActive;
      if (northernLightsActive) northernLightsEffect(50);
      else setAllLEDsToRed();  // Reset to red if effect is turned off
      break;
  }
}

// Runs the rain effect
void runRainEffect() {
  digitalWrite(LED_RAIN_PIN, HIGH);
}

// Runs the fog effect
void runFogEffect() {
  digitalWrite(LED_FOG_PIN, HIGH);
}

// Sets all LEDs to red
void setAllLEDsToRed() {
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.Color(255, 0, 0));
  }
  strip.show();
}

// Northern lights effect
void northernLightsEffect(int speed) {
  uint32_t colors[] = {
    strip.Color(0, 255, 100), 
    strip.Color(0, 0, 255),
    strip.Color(75, 0, 130),
    strip.Color(238, 130, 238)
  };
  
  for (int i = 0; i < NUM_LEDS; i++) {
    for (int colorIndex = 0; colorIndex < 4; colorIndex++) {
      strip.setPixelColor(i, colors[colorIndex]);
      strip.show();
      delay(speed);
    }
  }
}
