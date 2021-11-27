/* Project : 
 * Send  moisture, temperature, EC, PH, Nitrogen, Phosphorus, Potassium data from 7 in 1 Sensor to LoRaWAN with Maxiiot DL7612-AS923-TH Shield
 * Date : 12/11/2021
*/

#include <ModbusMaster.h>
#include <HardwareSerial.h>
#include <ArduinoOTA.h>
#include <WiFiManager.h>
#include <ThingControl_DL7612.h>
#include "REG_CONFIG.h"

WiFiManager wifiManager;
ThingControl_DL7612 lora;

// Modbus
#define rxPin1                  16
#define txPin1                  17
#define buadrate1               4800
#define configParam1            SERIAL_8N1

// OTA
#define HOSTNAME "SmartSensorLoRa01"
#define PASSWORD "12345678"

HardwareSerial modbus(1);

ModbusMaster node;

struct Seveninone
{
  String moisture;
  String temperature;
  String EC;
  String PH;
  String Nit;
  String Pho;
  String Pot;
};

Seveninone sensor ;

unsigned long currentMillis;
unsigned long previousMillis;
const unsigned long interval = 60000; // Interval Time

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  Serial.begin(115200);
  modbus.begin(buadrate1, configParam1, rxPin1, txPin1);

  // OTA
  wifiManager.setTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setAPClientCheck(true);
  String wifiName = "@ESP32-";
  wifiName.concat(String((uint32_t)ESP.getEfuseMac(), HEX));
  if (!wifiManager.autoConnect(wifiName.c_str())) {
    //Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    //    ESP.reset();
    //delay(1000);
    ESP.restart();
    delay(1);
  }
  setupWIFI();
  setupOTA();
  // LoRaWAN
  lora.setupModule();
  lora.LoRaShowConfig();
}

void loop() {
  int portTransmit = random(1, 223);
  ArduinoOTA.handle();
  currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    readSensor();
    sendSensor();
    previousMillis = currentMillis;
  }
}

void sendSensor() {
  /* Decode
    case 0x0768:// moisture (0-1000)  2 Byte
    case 0x0267:// Temperature (0-800)  2 Byte
    case 0x0673:// EC (0-20000)   2 Byte
    case 0x0188:// PH (30-90)   2 Byte
    case 0x0371:// N (1-1999)   2 Byte
    case 0x0402:// P (1-1999)   2 Byte
    case 0x0586:// K (1-1999)   2 Byte
    example data
    sensorData = "0768009E0267010D06730088018841037100090402000D05860020";
    sensorData = "0768009E0267010D06730088018841037100090402000D05860020";
                  0768 0171 0267 00fa 0673 00c9 0188 0050 0371 e 0402 0013 0586 002f
    sensorData = '0768' + sensor.moisture + '0267' + sensor.temperature + '0673' + sensor.EC + '0188' + sensor.PH + '0371' + sensor.Nit + '0402' + sensor.Pho + '0586' + sensor.Pot;
  */
  int dataLength;
  String sensorData;
  sensorData = "0768" + convert2Hex(sensor.moisture) + "0267" + convert2Hex(sensor.temperature) + "0673" + convert2Hex(sensor.EC) + "0188" + convert2Hex(sensor.PH) + "0371" + convert2Hex(sensor.Nit) + "0402" + convert2Hex(sensor.Pho) + "0586" + convert2Hex(sensor.Pot);
  Serial.println(sensorData);
  dataLength = sensorData.length();
  Serial.println(dataLength);
  lora.sendHexData(dataLength / 2 , sensorData);
}

String convert2Hex(String tempdata) {
  unsigned long datatemp;
  int dataLength;
  String sensorData;
  Serial.println(tempdata);
  datatemp = tempdata.toInt();
  //  Serial.println(datatemp);
  sensorData = String(datatemp, HEX);
  Serial.println(sensorData);
  dataLength = sensorData.length();
  //  Serial.println(dataLength);
  if (dataLength == 3) {
    sensorData = "0" + sensorData;
    Serial.println(sensorData);
  } else if (dataLength == 2) {
    sensorData = "00" + sensorData;
    Serial.println(sensorData);
  } else if (dataLength == 1) {
    sensorData = "000" + sensorData;
  } else  if (sensorData == 0) {
    sensorData = "0000";
  }
  return sensorData;
}

long readModbus(char addr, uint16_t  REG)
{
  uint8_t j, result;
  uint16_t data;
  // communicate with Modbus slave ID 1 over Hardware Serial (port 1)
  node.begin(addr, modbus);
  result = node.readHoldingRegisters(REG, 1);

  // do something with data if read is successful
  if (result == node.ku8MBSuccess)
  {
    data = node.getResponseBuffer(0);
    //Serial.println("Connec modbus Ok.");
    return data;
  } else
  {
    Serial.print("Connec modbus ID: ");
    Serial.print(addr);
    Serial.print(" Sensor fail. REG >>> ");
    Serial.println(REG); // Debug
    delay(100);
    return 0;
  }
}

void readSensor()
{
  sensor.moisture = readModbus(ID_SENSOR, Address[0]);
  sensor.temperature = readModbus(ID_SENSOR, Address[1]);
  sensor.EC = readModbus(ID_SENSOR, Address[2]);
  sensor.PH = readModbus(ID_SENSOR, Address[3]);
  sensor.Nit = readModbus(ID_SENSOR, Address[4]);
  sensor.Pho = readModbus(ID_SENSOR, Address[5]);
  sensor.Pot = readModbus(ID_SENSOR, Address[6]);

  Serial.print("Moisture : ");  Serial.println(sensor.moisture);
  Serial.print("Temperature : ");  Serial.println(sensor.temperature);
  Serial.print("EC : ");  Serial.println(sensor.EC);
  Serial.print("PH : ");  Serial.println(sensor.PH);
  Serial.print("Nitrogen : ");  Serial.println(sensor.Nit);
  Serial.print("Phosphorus : ");  Serial.println(sensor.Pho);
  Serial.print("Potassium : ");  Serial.println(sensor.Pot);
  Serial.println("");
  delay(2000);
}

// OTA
void setupOTA()
{
  //Port defaults to 8266
  //ArduinoOTA.setPort(8266);

  //Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(HOSTNAME);

  //No authentication by default
  ArduinoOTA.setPassword(PASSWORD);

  //Password can be set with it's md5 value as well
  //MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  //ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]()
  {
    Serial.println("Start Updating....");
    Serial.printf("Start Updating....Type:%s\n", (ArduinoOTA.getCommand() == U_FLASH) ? "sketch" : "filesystem");
  });

  ArduinoOTA.onEnd([]()
  {
    Serial.println("Update Complete!");
    ESP.restart();
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
  {
    String pro = String(progress / (total / 100)) + "%";
    int progressbar = (progress / (total / 100));
    //int progressbar = (progress / 5) % 100;
    //int pro = progress / (total / 100);

    //    drawUpdate(progressbar, 265, 195);
    //    tft.drawString(title5, 310, 235, GFXFF); // Print the test text in the custom font

    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error)
  {
    Serial.printf("Error[%u]: ", error);
    String info = "Error Info:";
    switch (error)
    {
      case OTA_AUTH_ERROR:
        info += "Auth Failed";
        Serial.println("Auth Failed");
        break;

      case OTA_BEGIN_ERROR:
        info += "Begin Failed";
        Serial.println("Begin Failed");
        break;

      case OTA_CONNECT_ERROR:
        info += "Connect Failed";
        Serial.println("Connect Failed");
        break;

      case OTA_RECEIVE_ERROR:
        info += "Receive Failed";
        Serial.println("Receive Failed");
        break;

      case OTA_END_ERROR:
        info += "End Failed";
        Serial.println("End Failed");
        break;
    }
    Serial.println(info);
    ESP.restart();
  });
  ArduinoOTA.begin();
}

void setupWIFI()
{
  WiFi.setHostname(HOSTNAME);
  byte count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 10)
  {
    count ++;
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
    Serial.println("Connecting...OK.");
  else
    Serial.println("Connecting...Failed");
}
