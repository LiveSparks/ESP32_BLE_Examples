#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;

bool deviceConnected = false;

char value[50] = "Default";

#define customService BLEUUID((uint16_t)0x1700)
BLECharacteristic customCharacteristic(BLEUUID((uint16_t)0x1A00), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *customCharacteristic){
    std::string rcvString = customCharacteristic->getValue();
    if(rcvString.length()>0){
      Serial.println("Value Received from BLE: ");
      for (int i = 0; i < rcvString.length(); ++i)
      {
        Serial.print(rcvString[i]);
        value[i]=rcvString[i];
      }
      for (int i = rcvString.length(); i < 50; ++i)
      {
        value[i]=NULL;
      }
      customCharacteristic->setValue((char*)&value);
    }
    else{
      Serial.println("Empty Value Received!");
    }
  }
};

void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("MyESP32");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();

  // Create the BLE Service
  BLEService *pService = pServer->createService(customService);

  // Create a BLE Characteristic
  pService->addCharacteristic(&customCharacteristic);

  customCharacteristic.setCallbacks(new MyCharacteristicCallbacks());

  pServer->getAdvertising()->addServiceUUID(customService);

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();

  Serial.println(value);
  customCharacteristic.setValue((char*)&value);
}

void loop() {
  delay(100);
}