#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>
#include <DHT.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Rotary encoder pins
#define CLK_PIN 25
#define DT_PIN 26
#define SW_PIN 27

// DHT22 and LDR sensor pins
#define DHT_PIN 19
#define LDR_PIN 13

#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

ezButton button(SW_PIN);
int hour = 0;
int minute = 0;
int selection = 0;  // 0 = hour, 1 = minute, 2 = confirm
int menuSelection = 0;  // 0 = Modes, 1 = Controls
bool blinkState = true;
bool menuBlinkState = true;
bool inControlScreen = false;  // New flag to handle the control screen

unsigned long previousMillis = 0;
unsigned long menuPreviousMillis = 0;
const long interval = 500; // Blink interval for time selection
const long menuInterval = 300; // Blink interval for menu selection

int lastStateCLK;
int currentStateCLK;

void setup() {
  Serial.begin(9600);  // Start serial communication for debugging
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }

  pinMode(CLK_PIN, INPUT);
  pinMode(DT_PIN, INPUT);
  button.setDebounceTime(50);

  lastStateCLK = digitalRead(CLK_PIN);

  dht.begin();  // Initialize DHT22 sensor
}

void loop() {
  button.loop();
  unsigned long currentMillis = millis();

  // Read the current state of CLK_PIN
  currentStateCLK = digitalRead(CLK_PIN);

  // Handle encoder rotation
  if (currentStateCLK != lastStateCLK && currentStateCLK == HIGH) {
    if (digitalRead(DT_PIN) == LOW) {
      if (selection == 0 && !inControlScreen) {
        hour = (hour + 1) % 24;
      } else if (selection == 1 && !inControlScreen) {
        minute = (minute + 1) % 60;
      } else if (inControlScreen) {
        menuSelection = (menuSelection + 1) % 2;  // Scroll between Modes and Controls
      }
    } else {
      if (selection == 0 && !inControlScreen) {
        hour = (hour - 1 + 24) % 24;
      } else if (selection == 1 && !inControlScreen) {
        minute = (minute - 1 + 60) % 60;
      } else if (inControlScreen) {
        menuSelection = (menuSelection - 1 + 2) % 2;  // Scroll between Modes and Controls
      }
    }
  }

  lastStateCLK = currentStateCLK;

  // Button press handling
  if (button.isPressed()) {
    if (selection < 2 && !inControlScreen) {
      selection++;  // Move to the next time selection step or confirm
    } else {
      if (!inControlScreen) {
        inControlScreen = true;  // Enter Modes/Controls menu
      } else if (menuSelection == 1) {
        showControlsScreen();  // Enter Controls screen
      }
    }
  }

  // Handle blinking for time setting
  if (!inControlScreen && currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    blinkState = !blinkState;
  }

  // Handle blinking for menu selection
  if (inControlScreen && currentMillis - menuPreviousMillis >= menuInterval) {
    menuPreviousMillis = currentMillis;
    menuBlinkState = !menuBlinkState;
  }

  // Display the appropriate screen
  if (!inControlScreen) {
    showTimeScreen();
  } else {
    showModesScreen();
  }
}

void showTimeScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  display.print("TIME?");

  display.setCursor(20, 40);
  if (selection == 0 && blinkState) {
    display.print("  :");
  } else {
    if (hour < 10) display.print('0');
    display.print(hour);
    display.print(":");
  }

  if (selection == 1 && blinkState) {
    display.print("  ");
  } else {
    if (minute < 10) display.print('0');
    display.print(minute);
  }

  display.display();
}

void showModesScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  // Display current time at the top of the Modes/Controls screen
  display.setCursor(0, 0);
  if (hour < 10) display.print('0');
  display.print(hour);
  display.print(":");
  if (minute < 10) display.print('0');
  display.print(minute);

  display.setTextSize(2);
  if (menuSelection == 0 && menuBlinkState) {
    display.setCursor(10, 20);
    display.print(" Modes");
  } else {
    display.setCursor(10, 20);
    display.print("Modes");
  }

  if (menuSelection == 1 && menuBlinkState) {
    display.setCursor(10, 40);
    display.print(" Controls");
  } else {
    display.setCursor(10, 40);
    display.print("Controls");
  }

  display.display();
}

void showControlsScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  // Read sensor data
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightLevel = analogRead(LDR_PIN);

  // Display sensor data
  display.setCursor(0, 0);
  display.print("Temp: ");
  display.print(temperature);
  display.print(" C");

  display.setCursor(0, 10);
  display.print("Humidity: ");
  display.print(humidity);
  display.print(" %");

  display.setCursor(0, 20);
  display.print("Light: ");
  display.print(lightLevel);

  display.display();
}
