#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_NeoPixel.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ezButton.h>
#include <RTClib.h>

RTC_DS3231 rtc;

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
#define DHTTYPE DHT11
#define LED_FOG_PIN 4
#define LED_HEATER_PIN 16
#define defaultGround 0
//#define LED_RAIN_PIN 2

DHT dht(DHTPIN, DHTTYPE);
ezButton button(SW_PIN);

int hour = 0;
int minute = 0;
int seconds = 0;
int year = 2024;
int month = 12;
int day = 20;
int selection = 0; // 0 = hour, 1 = minute, 2 = confirm
int menuSelection = 0;  // Selection for modes or controls
int modesSelection = 0;  // Selection inside the modes screen
int counter = 0;
int randInt = 0;
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
DateTime now;
int lastStateCLK;
int currentStateCLK;
int lightCounter = 0;
const int optionsCount = 6;
String options[optionsCount] = {"Rain", "Fog", "Aurora ", "Day Cycle", "Controls", "Re-Set Time"};
bool isActive[optionsCount] = {rainEffectActive, fogEffectActive, northernLightsActive, dayCycleActive, false, false};


void setup() {
  Serial.begin(9600);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  if (! rtc.begin()) {
    Serial.println("RTC module is NOT found");
    Serial.flush();
    while (1);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

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
  //setAllLEDsToRed();

// Initialize effect pins
  pinMode(LED_FOG_PIN, OUTPUT);
  pinMode(LED_HEATER_PIN, OUTPUT);
  pinMode(defaultGround, OUTPUT);
  //pinMode(LED_RAIN_PIN, OUTPUT);
  digitalWrite(LED_HEATER_PIN, LOW);
  digitalWrite(LED_HEATER_PIN, HIGH);
  digitalWrite(defaultGround, LOW);
  
  digitalWrite(LED_FOG_PIN, HIGH);
  now = rtc.now();
  hour = 0;
  minute = 0;
  now = rtc.now();
  hour = now.hour();
  minute = now.minute();
  seconds = now.second();
  minute += 2;
  //dayCycleActive = false;
  
}

void loop() {
  //DateTime now = rtc.now();
  
  button.loop();
  unsigned long currentMillis = millis();

  // Rotary encoder handling
  currentStateCLK = digitalRead(CLK_PIN);
  if (currentStateCLK != lastStateCLK) {
    if(currentStateCLK == HIGH)
    {
      if (digitalRead(DT_PIN) == LOW) {
      if (!inControlScreen && !inModesScreen) {
        // Time setting mode
        // if (selection == 0) hour = (hour + 1) % 24;
        // else if (selection == 1) minute = (minute + 1) % 60;
      } 
      else if (inModesScreen) {
        // Mode selection navigation
        modesSelection = (modesSelection + 1) % 6;  // There are 5 mode options
      }
    } 
    else {
      if (!inControlScreen && !inModesScreen) 
      {
        //if (selection == 0) hour = (hour - 1 + 24) % 24;
        //else if (selection == 1) minute = (minute - 1 + 60) % 60;
      } 
      else if (inModesScreen)
      {
        
        if(modesSelection > -1)
        {
          modesSelection = (modesSelection -1 );
        }
        else
        {
          modesSelection = 5;
        }
      }
    }
  }
  else
  {
    if (digitalRead(DT_PIN) == LOW) {
      if (!inControlScreen && !inModesScreen) {
        // Time setting mode
        //if (selection == 0) hour = (hour - 1 + 24) % 24;
        //else if (selection == 1) minute = (minute - 1 + 60) % 60;
      } 
      else if (inModesScreen) {
        // Mode selection navigation
        if(modesSelection > -1)
        {
          modesSelection = (modesSelection -1 );
        }
        else
        {
          modesSelection = 5;
        }
      }
    } 
    else {
      if (!inControlScreen && !inModesScreen) 
      {
        //if (selection == 0) hour = (hour + 1) % 24;
        //else if (selection == 1) minute = (minute + 1) % 60;
      } 
      else if (inModesScreen)
      {
        /*
        if(modesSelection > -1)
        {
          modesSelection = (modesSelection -1 );
        }
        else
        {
          modesSelection = 5;
        }
        */
        modesSelection = (modesSelection + 1) % 6;  // There are 5 mode options

      }
    }
  }
}
  lastStateCLK = currentStateCLK;
  int lightValue1 = analogRead(LDR_PIN);
  int lightValue = map(lightValue1, 0, 4000, 100, 0);
  if(lightValue <= 30)
  {
    bassOff();
  }
  Serial.print("counter ");
  Serial.print(counter);
  Serial.print("     randInt ");
  Serial.println(randInt);
  if (northernLightsActive && counter == randInt) 
  {
    northernLightsEffect(lightValue, lightCounter);
    lightCounter++;
    randInt = random(500, 1001);
    counter = 0;
    //delay(random(200, 700));
  }
  if (dayCycleActive) 
  {
    setDayCycleEffect(lightValue, hour, minute);
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
  seconds ++;
  if(seconds/36 == 60)
  {
    seconds = 0;
    if(minute != 60)
    {
      minute++;
    }
    else
    {
      minute = 0;
      if(hour != 24)
      {
        hour++;
      }
      else
      {
        hour == 0;
      }
      
    }  
  }
  rtc.adjust(DateTime(year, month, day, hour, minute, seconds));
  
  // Serial.print(hour);
  // Serial.print(" : ");
  // Serial.print(minute);
  // Serial.print(" : ");
  // Serial.println(seconds/36);
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  
}

// Display the controls screen with sensor data
void showControlsScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightValue1 = analogRead(LDR_PIN);
  int lightValue = map(lightValue1, 0, 4000, 100, 0);

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

  display.setCursor(0, 40);
  display.print(hour);
  display.print(" : ");
  display.print(minute); 
  //display.print(" : ");
  //display.print(seconds/36); 
  

  display.display();
}

// Display the time-setting screen
void showTimeScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(20, 10);
  display.print("TIME");
  display.setCursor(20, 40);

  display.print((hour));
  display.print((" : "));
  display.print((minute));
  display.display();
}

// Display the modes screen with scrolling
void showModesScreen() {
   const int optionsCount = 6;
  String options[optionsCount] = {"Rain", "Fog", "Aurora ", "Day Cycle", "Controls", "Re-Set Time"};
  bool isActive[optionsCount] = {rainEffectActive, fogEffectActive, northernLightsActive, dayCycleActive, false, false};
    //isActive[6] = {true, false, true, true, false, false};

    
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
      //rainEffectActive = !rainEffectActive;
      if(rainEffectActive)
      {
        Serial.println("Active");
        digitalWrite(LED_HEATER_PIN, HIGH);
        rainEffectActive = !rainEffectActive;
        modesSelection++;
      }
      else
      {
        Serial.println("Not Active");
        //runFogEffect(false);
        //fogEffectActive = !fogEffectActive;
        digitalWrite(LED_HEATER_PIN, LOW);   // Turn fogger ON
        rainEffectActive = !rainEffectActive;
        modesSelection--;
        
      }
      //digitalWrite(LED_HEATER_PIN, rainEffectActive ? HIGH : LOW);
      break;
    case 1:  // Fog
      
      if(fogEffectActive)
      {
        Serial.println("Active");
        //runFogEffect(true);
        //fogEffectActive = !fogEffectActive;
        digitalWrite(LED_FOG_PIN, LOW);   // Turn fogger ON
        delay(1000);                      // Keep it ON for 1 second
        digitalWrite(LED_FOG_PIN, HIGH);
        fogEffectActive = !fogEffectActive;
        modesSelection++;
        modesSelection++;
        
      }
      else
      {
        Serial.println("Not Active");
        //runFogEffect(false);
        //fogEffectActive = !fogEffectActive;
        digitalWrite(LED_FOG_PIN, LOW);   // Turn fogger ON
        delay(500);                       // Keep it ON for 0.5 second
        digitalWrite(LED_FOG_PIN, HIGH);  // Turn fogger OFF for 0.5 second
        delay(500);
        digitalWrite(LED_FOG_PIN, LOW);   // Turn fogger ON for 0.5 second again
        delay(500);
        digitalWrite(LED_FOG_PIN, HIGH);  // Turn fogger OFF
        fogEffectActive = !fogEffectActive;
        modesSelection--;
        modesSelection--;
        
      }
      //digitalWrite(LED_FOG_PIN, fogEffectActive ? HIGH : LOW);
      break;
    case 2:  // Northern Lights
      northernLightsActive = !northernLightsActive;
      if (northernLightsActive) 
      {
        northernLightsEffect(50,1);
        counter = -50;
      }
      else setAllLEDsToRed();
      break;
    case 3:  // Day Cycle
      
      if (dayCycleActive) 
      {
        
        setDayCycleEffect(0, hour, 0);
        dayCycleActive = !dayCycleActive;
        modesSelection++;
      }
      else 
      {
        topOff();
        dayCycleActive = !dayCycleActive;
        modesSelection--;
      }
      
      break;
    case 4:  // Controls Screen
      inModesScreen = false;
      inControlScreen = true;
      break;
    case 5:
      inControlScreen = false;
      inModesScreen = false;
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
      // Turn fogger OFF
  }
  else if(state == false) {
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
void northernLightsEffect(int brightness, int counter) {
  // Shift colors up by one pixel
  for (int i = strip.numPixels() - 1; i > 0; i--) {
    uint32_t color = strip.getPixelColor(i - 1); // Get color of the previous pixel
    strip.setPixelColor(i, color);              // Set it to the current pixel
  }

  // Assign a new random color to pixel 0
  strip.setPixelColor(0, strip.Color(random(50, 200), random(0, 255), random(50, 255)));

  // Set brightness and display the changes
  strip.setBrightness(brightness);
  strip.show();
}


// Placeholder for Day Cycle effect
void setDayCycleEffect(int brightness, int hour, int minute)
{
  if(hour >= 6 && hour < 8) // Sunrise
  {
    stripTop.setPixelColor(0, stripTop.Color(255, 170, 0));  // Right most - Sunrise (orange)
    stripTop.setPixelColor(1, stripTop.Color(255, 190, 50)); // Slightly lighter orange
    stripTop.setPixelColor(2, stripTop.Color(255, 210, 100)); // Transition towards yellow
    stripTop.setPixelColor(3, stripTop.Color(255, 255, 150)); // Brighter yellow
    stripTop.setPixelColor(4, stripTop.Color(255, 255, 150)); // Soft yellow
    stripTop.setPixelColor(5, stripTop.Color(255, 255, 207)); // Bright white
    stripTop.setPixelColor(6, stripTop.Color(255, 255, 207)); // Bright white
    stripTop.setPixelColor(7, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(8, stripTop.Color(100, 100, 100)); // Bright white
  }
  else if(hour >= 8 && hour < 10) // Mid-morning
  {
    stripTop.setPixelColor(0, stripTop.Color(255, 255, 255)); // Right most - Bright white
    stripTop.setPixelColor(1, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(2, stripTop.Color(255, 255, 207)); // Soft yellow
    stripTop.setPixelColor(3, stripTop.Color(255, 255, 150)); // Brighter yellow
    stripTop.setPixelColor(4, stripTop.Color(255, 210, 100)); // Transition towards yellow
    stripTop.setPixelColor(5, stripTop.Color(255, 190, 50));  // Lighter orange
    stripTop.setPixelColor(6, stripTop.Color(255, 170, 0));   // Fading to orange
    stripTop.setPixelColor(7, stripTop.Color(0, 0, 0));       // Dark - no light
    stripTop.setPixelColor(8, stripTop.Color(0, 0, 0));       // Dark - no light
  }
  else if(hour >= 10 && hour < 12) // Late morning
  {
    stripTop.setPixelColor(0, stripTop.Color(255, 255, 255)); // Right most - Bright white
    stripTop.setPixelColor(1, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(2, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(3, stripTop.Color(255, 255, 207)); // Soft yellow
    stripTop.setPixelColor(4, stripTop.Color(255, 255, 150)); // Brighter yellow
    stripTop.setPixelColor(5, stripTop.Color(255, 210, 100)); // Transition towards yellow
    stripTop.setPixelColor(6, stripTop.Color(255, 190, 50));  // Lighter orange
    stripTop.setPixelColor(7, stripTop.Color(0, 0, 0));       // Dark - no light
    stripTop.setPixelColor(8, stripTop.Color(0, 0, 0));       // Dark - no light
  }
  else if(hour >= 12 && hour < 14) // Midday
  {
    stripTop.setPixelColor(0, stripTop.Color(255, 255, 255)); // Right most - Bright white
    stripTop.setPixelColor(1, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(2, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(3, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(4, stripTop.Color(255, 255, 207)); // Soft yellow
    stripTop.setPixelColor(5, stripTop.Color(255, 255, 150)); // Brighter yellow
    stripTop.setPixelColor(6, stripTop.Color(255, 210, 100)); // Transition towards yellow
    stripTop.setPixelColor(7, stripTop.Color(0, 0, 0));       // Dark - no light
    stripTop.setPixelColor(8, stripTop.Color(0, 0, 0));       // Dark - no light
  }
  else if(hour >= 14 && hour < 16) // Early afternoon
  {
    stripTop.setPixelColor(0, stripTop.Color(255, 255, 255)); // Right most - Bright white
    stripTop.setPixelColor(1, stripTop.Color(255, 255, 255)); // Bright white
    stripTop.setPixelColor(2, stripTop.Color(255, 255, 207)); // Soft yellow
    stripTop.setPixelColor(3, stripTop.Color(255, 255, 150)); // Brighter yellow
    stripTop.setPixelColor(4, stripTop.Color(255, 210, 100)); // Transition towards yellow
    stripTop.setPixelColor(5, stripTop.Color(255, 190, 50));  // Lighter orange
    stripTop.setPixelColor(6, stripTop.Color(255, 170, 0));   // Fading to orange
    stripTop.setPixelColor(7, stripTop.Color(0, 0, 0));       // Dark - no light
    stripTop.setPixelColor(8, stripTop.Color(0, 0, 0));       // Dark - no light
  }
  else if(hour >= 16 && hour < 18) // Late afternoon
  {
    stripTop.setPixelColor(0, stripTop.Color(255, 255, 207)); // Right most - Soft yellow
    stripTop.setPixelColor(1, stripTop.Color(255, 255, 150)); // Brighter yellow
    stripTop.setPixelColor(2, stripTop.Color(255, 210, 100)); // Transition towards yellow
    stripTop.setPixelColor(3, stripTop.Color(255, 190, 50));  // Lighter orange
    stripTop.setPixelColor(4, stripTop.Color(255, 170, 0));   // Fading to orange
    stripTop.setPixelColor(5, stripTop.Color(255, 120, 0));   // Darker orange
    stripTop.setPixelColor(6, stripTop.Color(255, 70, 0));    // Dark orange
    stripTop.setPixelColor(7, stripTop.Color(0, 0, 0));       // Dark - no light
    stripTop.setPixelColor(8, stripTop.Color(0, 0, 0));       // Dark - no light
  }
  else if(hour == 18) // Sunset
  {
    stripTop.setPixelColor(0, stripTop.Color(255, 120, 0));   // Right most - Dark orange
    stripTop.setPixelColor(1, stripTop.Color(255, 70, 0));    // Darker orange
    stripTop.setPixelColor(2, stripTop.Color(255, 70, 0));    // Same orange
    stripTop.setPixelColor(3, stripTop.Color(255, 120, 0));   // Transition orange
    stripTop.setPixelColor(4, stripTop.Color(255, 170, 0));   // Fading orange
    stripTop.setPixelColor(5, stripTop.Color(255, 190, 50));  // Lighter orange
    stripTop.setPixelColor(6, stripTop.Color(255, 210, 100)); // Transition towards yellow
    stripTop.setPixelColor(7, stripTop.Color(255, 255, 150)); // Brighter yellow
    stripTop.setPixelColor(8, stripTop.Color(255, 255, 207)); // Soft yellow
  }
  else if(hour < 6 || hour > 18) // Night
  {
    for (int i = 0; i < 9; i++) {
      stripTop.setPixelColor(i, stripTop.Color(0, 0, 0)); // Turn off all LEDs
    }
    bassOff();
  }

  stripTop.setBrightness(brightness);
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
  for (int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}
void topOff() {
  for (int i = 0; i < stripTop.numPixels(); i++) {
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
  strip.show();
}