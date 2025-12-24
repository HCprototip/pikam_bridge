#include <Arduino.h>


#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
// BLE UUIDs
#define SERVICE_UUID           "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_UUID_RX "6e400002-b5a3-f393-e0a9-e50e24dcca9e"
// Forward declarations para BLE callbacks (implementadas en main.cpp)
class MyServerCallbacks;
class MyCallbacks;

#define BLE_MAX_LONG 512

char rebut_BLE[BLE_MAX_LONG];

// Cola para comandos JSON recibidos por BLE
QueueHandle_t jsonCommandQueue = NULL;

BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
// BLE Server Callbacks


