#include <DHT.h>
#include <DHT_U.h>
#include "BluetoothSerial.h" 

#define AO_PIN 34
#define DHTPIN 26
#define DHTTYPE DHT22

int i = 0;
int j = 10000;
int incoming;
int led_pin_1 = 4;
int led_pin_2 = 0;
int led_pin_3 = 2;

DHT dht(DHTPIN, DHT22);
BluetoothSerial ESP_BT; 

void setup() {
  // Start DHT sensor
  dht.begin();
  delay(2000);

  // Serial communication and Bluetooth setup
  Serial.begin(9600);
  ESP_BT.begin("ESP32_Control");

  // Set attenuation for analog reading
  analogSetAttenuation(ADC_11db);

  // Initialize LED pins
  pinMode(led_pin_1, OUTPUT);
  pinMode(led_pin_2, OUTPUT);
  pinMode(led_pin_3, OUTPUT);
}

void loop() {
  // Handle Bluetooth commands
  if (ESP_BT.available()) {
    incoming = ESP_BT.read(); 
    int button = floor(incoming / 10);
    int value = incoming % 10;
    
    switch (button) {
      case 1:
        Serial.print("Button 1:"); Serial.println(value);
        //digitalWrite(led_pin_1, value);
        digitalWrite(led_pin_2, LOW);
        break;
      case 2:
        Serial.print("Button 2:"); Serial.println(value);
        digitalWrite(led_pin_1, HIGH);
        break;
      case 3:
        Serial.print("Button 3:"); Serial.println(value);
        digitalWrite(led_pin_3, value);
        break;
    }
  }

  // Read temperature, humidity, and light values as before
  float temp = dht.readTemperature();
  float humidity = dht.readHumidity();
  int lightValue = analogRead(AO_PIN);
  Serial.print("temp  "); Serial.println(temp);
  Serial.print("humidity  "); Serial.println(humidity);
  
  // Send data via Bluetooth instead of Serial
  ESP_BT.print(temp);
  ESP_BT.print("|");
  ESP_BT.print(humidity);
  ESP_BT.print("|");
  ESP_BT.println(lightValue);
  
  delay(1000);  // Delay between sends

  i++;
  j--;

  delay(1000);
}
