#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "bsec2.h"
#include "web_pages.h"

// Change these if your wiring is different
#define I2C_SDA 12
#define I2C_SCL 11

// Most BME680 boards are 0x76 or 0x77.
// Bosch's example uses the low address constant.
#define BME68X_I2C_ADDR_LOW 0x76

// BSEC sample rate: LP is a good starting point for most projects.
#define SAMPLE_RATE BSEC_SAMPLE_RATE_LP

// Optional: adjust this later if your temperature reads high/low.
#define TEMP_OFFSET 0.0f

const char* WIFI_SSID = "LUFT-AP";
const char* WIFI_PASSWORD = nullptr;

WebServer server(80);
Bsec2 envSensor;

// Nextion display serial connection. Adjust pins as needed for your hardware.
#define NEXTION_RX 44
#define NEXTION_TX 43
#define NEXTION_BAUD 9600
HardwareSerial nextionSerial(2);
uint8_t currentNextionPage = 255;

float latestCO2 = 0.0f;
float latestBVOC = 0.0f;
float latestPM = 0.0f;

void checkBsecStatus(Bsec2 bsec);
void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);
void handleRoot();
void handleSophia();
void handleSophiaStyles();
void handleData();
void updateNextionPage();
void nextionSetPage(uint8_t page);

//parte do sensor optico
const int measurePin = 5; //Connect dust sensor to Arduino A0 pin MUDAR PINO
const int ledPower = 6;   //Connect 3 led driver pins of dust sensor to Arduino D2  MUDAR PINO
int samplingTime = 280; // time required to sample signal coming out   of the sensor
int deltaTime = 40;
int sleepTime = 9680;
float voMeasured = 0;
float calcVoltage = 0;
float dustDensity = 0;


void setup() {
  Serial.begin(115200);
  pinMode(ledPower, OUTPUT);
  delay(500);
  delay(5000);
  Wire.begin(I2C_SDA, I2C_SCL);

  Serial.println("ESP32-S3 + BME680 + Bosch BSEC2 + Web Server");

  WiFi.softAP(WIFI_SSID);
  IPAddress apIP = WiFi.softAPIP();
  Serial.println();
  Serial.print("Access point started. Connect to ");
  Serial.print(WIFI_SSID);
  Serial.println(" (open network)");
  Serial.print("AP IP address: ");
  Serial.println(apIP);

  server.on("/", HTTP_GET, handleRoot);
  server.on("/sophia", HTTP_GET, handleSophia);
  server.on("/sophia/", HTTP_GET, handleSophia);
  server.on("/sophia/index.html", HTTP_GET, handleSophia);
  server.on("/sophia/styles.css", HTTP_GET, handleSophiaStyles);
  server.on("/data", HTTP_GET, handleData);
  server.begin();

  // Start sensor + BSEC
  if (!envSensor.begin(BME68X_I2C_ADDR_LOW, Wire)) {
    checkBsecStatus(envSensor);
  }

  // Temperature compensation (Bosch example uses this pattern)
  envSensor.setTemperatureOffset(TEMP_OFFSET);

  // Request the outputs you want
  bsecSensor sensorList[] = {
    BSEC_OUTPUT_IAQ,
    BSEC_OUTPUT_RAW_TEMPERATURE,
    BSEC_OUTPUT_RAW_PRESSURE,
    BSEC_OUTPUT_RAW_HUMIDITY,
    BSEC_OUTPUT_RAW_GAS,
    BSEC_OUTPUT_STABILIZATION_STATUS,
    BSEC_OUTPUT_RUN_IN_STATUS,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE,
    BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY,
    BSEC_OUTPUT_STATIC_IAQ,
    BSEC_OUTPUT_CO2_EQUIVALENT,
    BSEC_OUTPUT_BREATH_VOC_EQUIVALENT,
    BSEC_OUTPUT_GAS_PERCENTAGE,
    BSEC_OUTPUT_COMPENSATED_GAS
  };

  if (!envSensor.updateSubscription(sensorList, ARRAY_LEN(sensorList), SAMPLE_RATE)) {
    checkBsecStatus(envSensor);
  }

  envSensor.attachCallback(newDataCallback);

  nextionSerial.begin(NEXTION_BAUD, SERIAL_8N1, NEXTION_RX, NEXTION_TX);
  nextionSetPage(1); // default to moderate page until data is available

  Serial.println("BSEC library version "
                 + String(envSensor.version.major) + "."
                 + String(envSensor.version.minor) + "."
                 + String(envSensor.version.major_bugfix) + "."
                 + String(envSensor.version.minor_bugfix));
}

void loop() {
  server.handleClient();

  if (!envSensor.run()) {
    checkBsecStatus(envSensor);
  }

  digitalWrite(ledPower, LOW); // power on the LED
  delayMicroseconds(samplingTime);
  voMeasured = analogRead(measurePin); // read the dust value
  delayMicroseconds(deltaTime);
  digitalWrite(ledPower, HIGH); // turn the LED off
  delayMicroseconds(sleepTime);

  calcVoltage = voMeasured * (5.0 / 1024.0);
  dustDensity = 170 * calcVoltage - 0.1;
  latestPM = dustDensity;

  updateNextionPage();
  delay(1000);
}


void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec) {
  if (!outputs.nOutputs) return;

  Serial.println("\nBSEC outputs:");
  Serial.println("Time stamp = " + String((int)(outputs.output[0].time_stamp / INT64_C(1000000))));

  for (uint8_t i = 0; i < outputs.nOutputs; i++) {
    const bsecData output = outputs.output[i];

    switch (output.sensor_id) {
      case BSEC_OUTPUT_IAQ:
        Serial.println("IAQ = " + String(output.signal));
        Serial.println("IAQ accuracy = " + String((int)output.accuracy));
        break;

      case BSEC_OUTPUT_RAW_TEMPERATURE:
        Serial.println("Temperature = " + String(output.signal) + " C");
        break;

      case BSEC_OUTPUT_RAW_PRESSURE:
        Serial.println("Pressure = " + String(output.signal / 100.0f) + " hPa");
        break;

      case BSEC_OUTPUT_RAW_HUMIDITY:
        Serial.println("Humidity = " + String(output.signal) + " %");
        break;

      case BSEC_OUTPUT_RAW_GAS:
        Serial.println("Gas resistance = " + String(output.signal / 1000.0f) + " KOhms");
        break;

      case BSEC_OUTPUT_STABILIZATION_STATUS:
        Serial.println("Stabilization status = " + String(output.signal));
        break;

      case BSEC_OUTPUT_RUN_IN_STATUS:
        Serial.println("Run-in status = " + String(output.signal));
        break;

      case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_TEMPERATURE:
        Serial.println("Compensated temperature = " + String(output.signal) + " C");
        break;

      case BSEC_OUTPUT_SENSOR_HEAT_COMPENSATED_HUMIDITY:
        Serial.println("Compensated humidity = " + String(output.signal) + " %");
        break;

      case BSEC_OUTPUT_STATIC_IAQ:
        Serial.println("Static IAQ = " + String(output.signal));
        break;

      case BSEC_OUTPUT_CO2_EQUIVALENT:
        latestCO2 = output.signal;
        Serial.println("CO2 equivalent = " + String(output.signal) + " ppm");
        break;

      case BSEC_OUTPUT_BREATH_VOC_EQUIVALENT:
        latestBVOC = output.signal;
        Serial.println("bVOC equivalent = " + String(output.signal) + " ppm");
        break;

      case BSEC_OUTPUT_GAS_PERCENTAGE:
        Serial.println("Gas percentage = " + String(output.signal));
        break;

      case BSEC_OUTPUT_COMPENSATED_GAS:
        Serial.println("Compensated gas = " + String(output.signal));
        break;

      default:
        break;
    }
  }
}

void handleRoot() {
  server.send_P(200, "text/html; charset=utf-8", DASHBOARD_HTML);
}

void handleSophia() {
  server.send_P(200, "text/html; charset=utf-8", SOPHIA_INDEX_HTML);
}

void handleSophiaStyles() {
  server.send_P(200, "text/css; charset=utf-8", SOPHIA_STYLES_CSS);
}

void handleData() {
  String response = "{";
  response += "\"co2\":" + String(latestCO2, 1) + ",";
  response += "\"bvoc\":" + String(latestBVOC, 2) + ",";
  response += "\"pm\":" + String(latestPM, 1);
  response += "}";
  server.send(200, "application/json", response);
}

void nextionSetPage(uint8_t page) {
  if (page == currentNextionPage) return;
  currentNextionPage = page;
  nextionSerial.print("page ");
  nextionSerial.print(page);
  nextionSerial.write(0xFF);
  nextionSerial.write(0xFF);
  nextionSerial.write(0xFF);
}

void updateNextionPage() {
  // Air quality thresholds: adjust as needed for your application.
  const float goodCO2 = 1000.0f;
  const float badBVOC = 1.0f;
  const float badCO2 = 2000.0f;
  const float goodBVOC = 0.5f;
  const float goodPM = 500.0f;
  const float badPM = 1000.0f;

  //bool bad = latestCO2 >= badCO2 || latestBVOC >= badBVOC || latestPM >= badPM;
  bool bad = 0;
  bool moderate = 0;
  //bool moderate = latestCO2 >= goodCO2 || latestBVOC >= goodBVOC || latestPM >= goodPM;

  if (bad) {
    nextionSetPage(0);
  } else if (moderate) {
    nextionSetPage(1);
  } else {
    nextionSetPage(2);
  }
}

void checkBsecStatus(Bsec2 bsec) {
  if (bsec.status < BSEC_OK) {
    Serial.println("BSEC error code: " + String(bsec.status));
  } else if (bsec.status > BSEC_OK) {
    Serial.println("BSEC warning code: " + String(bsec.status));
  }

  if (bsec.sensor.status < BME68X_OK) {
    Serial.println("BME68X error code: " + String(bsec.sensor.status));
  } else if (bsec.sensor.status > BME68X_OK) {
    Serial.println("BME68X warning code: " + String(bsec.sensor.status));
  }
}
