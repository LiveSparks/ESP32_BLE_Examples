#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define switchPin 15
char value[]="OFF";

#define serviceID BLEUUID((uint16_t)0x1700)

BLECharacteristic customCharacteristic(
  BLEUUID((uint16_t)0x1A00), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);

bool deviceConnected = false;
class ServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* MyServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* MyServer) {
      deviceConnected = false;
    }
};

void setup() {
  Serial.begin(115200);
  pinMode(switchPin,INPUT_PULLUP);

  // Create the BLE Device
  BLEDevice::init("MyESP32");

  // Create the BLE Server
  BLEServer *MyServer = BLEDevice::createServer();
  MyServer->setCallbacks(new ServerCallbacks());

  // Create the BLE Service
  BLEService *customService = MyServer->createService(serviceID);

  // Create a BLE Characteristic
  customService->addCharacteristic(&customCharacteristic);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  customCharacteristic.addDescriptor(new BLE2902());

  MyServer->getAdvertising()->addServiceUUID(serviceID);

  // Start the service
  customService->start();

  // Start advertising
  MyServer->getAdvertising()->start();
  Serial.println("Waiting for a client to connect....");
}

void loop() {
  if (deviceConnected) {
    if(digitalRead(switchPin)==0){
      customCharacteristic.setValue("ON");
      Serial.println("ON");
    }
    else{
      customCharacteristic.setValue((char*)&value);
      Serial.println("OFF");
    }
    customCharacteristic.notify();
  }
  delay(50);
}
