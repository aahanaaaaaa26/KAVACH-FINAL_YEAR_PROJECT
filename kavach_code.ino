#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

// Pins Definition
const int buttonPin = 4;         // Button Pin
const int vibrationPin = 5;      // 🚨 Vibration Motor Pin

int lastButtonState = HIGH; 

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      Serial.println("📱 Phone Connected Successfully!");
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      Serial.println("❌ Phone Disconnected! Restarting BLE broadcast...");
      BLEDevice::startAdvertising(); 
    }
};

void setup() {
  Serial.begin(115200);
  
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(vibrationPin, OUTPUT); // 🚨 Motor ko OUTPUT set kiya
  
  // Start hote hi ek baar chota sa buzz karega taaki pata chale hardware ON ho gaya hai
  digitalWrite(vibrationPin, HIGH);
  delay(200);
  digitalWrite(vibrationPin, LOW);

  Serial.println("Starting BLE Engine...");

  BLEDevice::init("KAVACH_BLE"); 
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  
  BLEDevice::startAdvertising();
  
  Serial.println("📡 BLE Active! Phone app se scan karo...");
}

void loop() {
  int currentButtonState = digitalRead(buttonPin);

  // Agar button dabaya gaya
  if (currentButtonState == LOW && lastButtonState == HIGH) {
    Serial.println("------------------------------------");
    Serial.println("🚨 Button Physically Dabaya Gaya!");
    
    // 🚨 Motor Vibrate Karo
    digitalWrite(vibrationPin, HIGH); 
    
    if (deviceConnected) {
      Serial.println("📡 BLE par bhej raha hoon...");
      pCharacteristic->setValue("SOS_TRIGGER");
      pCharacteristic->notify(); 
    } else {
      Serial.println("❌ ERROR: Phone ESP32 se connected NAHI hai!");
    }
    delay(300); 
  }
  
  // Agar button chhoda gaya
  else if (currentButtonState == HIGH && lastButtonState == LOW) {
    Serial.println("✅ Button Released.");
    
    // 🚨 Motor Stop Karo
    digitalWrite(vibrationPin, LOW); 
    
    if (deviceConnected) {
      pCharacteristic->setValue("NORMAL");
      pCharacteristic->notify();
    }
    delay(300);
  }

  lastButtonState = currentButtonState;
  delay(10); 
}