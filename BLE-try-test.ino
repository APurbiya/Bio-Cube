#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

/*
  Complete Getting Started Guide: https://RandomNerdTutorials.com/esp32-bluetooth-low-energy-ble-arduino-ide/
  Based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleServer.cpp
  Ported to Arduino ESP32 by Evandro Copercini
*/

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  // Initialize BLE
  BLEDevice::init("MyESP32");

  // Create BLE server
  BLEServer *pServer = BLEDevice::createServer();
  
  // Create BLE service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create BLE characteristic
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  // Set characteristic value
  pCharacteristic->setValue("Hello World says Neil");

  // Start the service
  pService->start();

  // Get BLE advertising object
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();

  // Add service UUID to advertising packet
  pAdvertising->addServiceUUID(SERVICE_UUID);

  // Set complete local name for better discoverability on iOS
  pAdvertising->setCompleteLocalName("MyESP32");

  // Set scan response to true for better detection by iOS devices
  pAdvertising->setScanResponse(true);

  // Functions that help with iPhone connection issues
  pAdvertising->setMinPreferred(0x06);
  pAdvertising->setMinPreferred(0x12);

  // Set advertising interval to improve discoverability (100 ms)
  pAdvertising->setInterval(160);  // 160 * 0.625 ms = 100 ms

  // Set transmission power level to maximum for better range and stability
  pAdvertising->setPowerLevel(ESP_PWR_LVL_P9);  // Maximum power level

  // Disable whitelisting to ensure all devices can discover the ESP32
  pAdvertising->setScanFilter(false);

  // Start advertising
  BLEDevice::startAdvertising();

  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // Main code to run repeatedly
  delay(2000);
}

