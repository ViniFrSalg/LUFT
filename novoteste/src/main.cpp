#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include "bsec2.h"

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

float latestCO2 = 0.0f;
float latestBVOC = 0.0f;
float latestPM = 0.0f;

void checkBsecStatus(Bsec2 bsec);
void newDataCallback(const bme68xData data, const bsecOutputs outputs, Bsec2 bsec);
void handleRoot();
void handleData();

const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8" />
  <meta name="viewport" content="width=device-width, initial-scale=1" />
  <title>LUFT Live Data</title>
  <style>
    body { margin: 0; font-family: Arial, sans-serif; background: linear-gradient(180deg, #e9f7ff, #f6fcff); color: #1f3a60; }
    .page { max-width: 900px; margin: 0 auto; padding: 1.5rem; }
    .hero { background: #d9efff; border-radius: 20px; padding: 2rem; box-shadow: 0 16px 35px rgba(94, 160, 230, 0.16); }
    .hero h1 { margin: 0 0 0.5rem; font-size: 2.4rem; }
    .hero p { margin: 0; color: #3d5a7a; }
    .cards { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 1rem; margin-top: 1.5rem; }
    .card { background: white; border-radius: 18px; padding: 1.4rem; border: 1px solid rgba(94, 200, 255, 0.4); box-shadow: 0 10px 22px rgba(94, 200, 255, 0.12); }
    .card h2 { margin: 0 0 0.75rem; color: #1a4f7a; }
    .card p { margin: 0; font-size: 1.3rem; line-height: 1.5; }
    .status { margin-top: 1.5rem; color: #3d5a7a; font-size: 0.96rem; }
    .footer { margin-top: 2rem; text-align: center; color: #6d7c96; font-size: 0.95rem; }
  </style>
</head>
<body>
  <div class="page">
    <section class="hero">
      <h1>LUFT Live Sensor Data</h1>
      <p>Current air quality readings from the ESP32-S3: CO₂, bVOC, and particles from the optical sensor.</p>
    </section>
    <div class="cards">
      <div class="card">
        <h2>CO₂</h2>
        <p id="co2">Loading...</p>
      </div>
      <div class="card">
        <h2>bVOC</h2>
        <p id="bvoc">Loading...</p>
      </div>
      <div class="card">
        <h2>Particles</h2>
        <p id="pm">Loading...</p>
      </div>
    </div>
    <div class="status">Data refreshes every 2 seconds. If you see dashes, the sensor is still warming up.</div>
    <div class="footer">ESP32-S3 web server is serving this page from your local network.</div>
  </div>
  <script>
    async function updateData() {
      try {
        const res = await fetch('/data');
        if (!res.ok) return;
        const json = await res.json();
        document.getElementById('co2').textContent = json.co2.toFixed(1) + ' ppm';
        document.getElementById('bvoc').textContent = json.bvoc.toFixed(2) + ' ppm';
        document.getElementById('pm').textContent = json.pm.toFixed(1) + ' µg/m³';
      } catch (err) {
        console.error(err);
      }
    }
    setInterval(updateData, 2000);
    updateData();
  </script>
</body>
</html>
)rawliteral";


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

  server.on("/", handleRoot);
  server.on("/data", handleData);
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
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  String response = "{";
  response += "\"co2\":" + String(latestCO2, 1) + ",";
  response += "\"bvoc\":" + String(latestBVOC, 2) + ",";
  response += "\"pm\":" + String(latestPM, 1);
  response += "}";
  server.send(200, "application/json", response);
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