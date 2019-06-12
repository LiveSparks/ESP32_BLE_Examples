#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <OneWire.h>

int sensorPin = 15;
OneWire ds(sensorPin);

// BLECharacteristic *pCharacteristic;

bool deviceConnected = false;

int rec=0;

void getTemp();
void BLETransfer(int16_t);

float temperature = -1000;

#define enviornmentService BLEUUID((uint16_t)0x181A)

BLECharacteristic temperatureCharacteristic(
  BLEUUID((uint16_t)0x2A6E), 
  BLECharacteristic::PROPERTY_READ | 
  BLECharacteristic::PROPERTY_NOTIFY
);
//BLEDescriptor tempDescriptor(BLEUUID((uint16_t)0x2901));

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};



void setup() {
  Serial.begin(115200);

  // Create the BLE Device
  BLEDevice::init("MyESP32");

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pEnviornment = pServer->createService(enviornmentService);

  // Create a BLE Characteristic
  pEnviornment->addCharacteristic(&temperatureCharacteristic);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  temperatureCharacteristic.addDescriptor(new BLE2902());

  BLEDescriptor TemperatureDescriptor(BLEUUID((uint16_t)0x2901));
  TemperatureDescriptor.setValue("Temperature -40-60°C");
  temperatureCharacteristic.addDescriptor(&TemperatureDescriptor);

  pServer->getAdvertising()->addServiceUUID(enviornmentService);

  // Start the service
  pEnviornment->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
}

void loop() {
  getTemp();
  Serial.println(temperature);
  String beaconMsg;
  if(temperature==-1000)
  {
    if(rec>5)
    {
      beaconMsg = "ESPSNA";
      Serial.println(beaconMsg);
      temperature = 100000;
    }
    return;
  }

  if (deviceConnected) {
    int16_t value;
    value = (temperature*100);
    Serial.println(value);
    // BLETransfer(value);
    temperatureCharacteristic.setValue((uint8_t*)&value, 2);
    temperatureCharacteristic.notify();
  }
   delay(200);
}

void BLETransfer(int16_t val){
  temperatureCharacteristic.setValue((uint8_t*)&val, 2);
  temperatureCharacteristic.notify();
}

void getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius
  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      Serial.println("no addr found");
      rec++;
      Serial.println("rec = "+ rec);
      temperature = -1000;
      return;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      rec++;
      Serial.println("rec = "+ rec);
      temperature = -1000;
      return;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      rec++;
      Serial.println("rec = "+ rec);
      temperature = -1000;
      return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  
  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }
  
  ds.reset_search();
  
  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;
  Serial.println("rec = 0");
  rec=0;  
  temperature = TemperatureSum;
}
