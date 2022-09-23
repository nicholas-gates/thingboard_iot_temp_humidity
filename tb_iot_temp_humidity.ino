// Heltec ESP32 Dev-Boards - Version: Latest
#include "thingProperties.h"

#include "heltec.h"
#include "images.h"

#include <DHT.h>
#include "WiFi.h"

#include "ThingsBoard.h"

#include <iostream>
#include <vector>

std::vector<String> screenOutput;

// void writeLine(String str, int row = 10, bool clear = true, bool writeToSerial = true);
void writeLines(std::vector<String> lines, bool clear = true, bool writeToSerial = true);

#define DHTPIN 5 // Digital pin connected to the DHT sensor

// Baud rate for debug serial
#define SERIAL_DEBUG_BAUD 115200

// Initialize ThingsBoard client
WiFiClient espClient;
// Initialize ThingsBoard instance
ThingsBoard tb(espClient);
// the Wifi radio's status
int status = WL_IDLE_STATUS;

// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11
// #define DHTTYPE DHT22 // DHT 22  (AM2302), AM2321
// #define DHTTYPE DHT21   // DHT 21 (AM2301)

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  serialInit();

  setupDisplay();

  cloudInit();

  dht.begin();
}

void loop()
{
  wifi_connect();

  tb_connect();

  screenOutput.clear();

  heartbeat();

  processDht();

  writeLines(screenOutput);

  tb.loop();
  // delay(10 * 60 * 1000); // 10 minutes
  delay(30 * 1000); // 5 seconds
}

void heartbeat() {
  random_value = random(1, 100);
  screenOutput.push_back("Random Value: " + String(random_value));
  tb.sendTelemetryFloat("random_value", random_value);
}

void wifi_connect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  
  Serial.println("Connected to WiFi");

  Serial.println(WiFi.localIP());
  //The ESP8266 tries to reconnect automatically when the connection is lost
  WiFi.setAutoReconnect(true);
  WiFi.persistent(true);
}

void tb_connect()
{

  if (!tb.connected())
  {
    // Connect to the ThingsBoard
    Serial.print("Connecting to: ");
    Serial.print(THINGSBOARD_SERVER);
    Serial.print(" with token ");
    Serial.println(DEVICE_KEY);
    if (!tb.connect(THINGSBOARD_SERVER, DEVICE_KEY))
    {
      Serial.println("Failed to connect");
      return;
    }

    Serial.println("Connected to ThingBoard!");

    // Serial.println("Firwmare Update...");
    // tb.Start_Firmware_Update(CURRENT_FIRMWARE_TITLE, CURRENT_FIRMWARE_VERSION, UpdatedCallback);
  }
}

void processDht()
{
  humidity = dht.readHumidity();
  tempCel = dht.readTemperature();
  tempFahr = dht.readTemperature(true);

  if (isnan(humidity) || isnan(tempCel || isnan(tempFahr)))
  {
    screenOutput.push_back("Failed to read from DHT sensor!");
    return;
  }

  screenOutput.push_back("Humidity: " + String(humidity) + " %");
  screenOutput.push_back("Temperature (C): " + String(tempCel) + " *C");
  screenOutput.push_back("Temperature (F): " + String(tempFahr) + " *F");

  tb.sendTelemetryFloat("humidity", humidity);
  tb.sendTelemetryFloat("tempCel", tempCel);
  tb.sendTelemetryFloat("tempFahr", tempFahr);
}

// void onLedSwitchChange()
// {
//   if (led_switch)
//   {
//     screenOutput.push_back("Toggled: LED ON (" + String(led_switch) + ")");
//   }
//   else
//   {
//     screenOutput.push_back("Toggled: LED OFF (" + String(led_switch) + ")");
//   }
// }

void setupDisplay()
{
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Enable*/, true /*Serial Enable*/);

  logo();
  delay(1000);

  Heltec.display->clear();
}

void logo()
{
  Heltec.display->clear();
  Heltec.display->drawXbm(0, 5, logo_width, logo_height, (const unsigned char *)logo_bits);
  Heltec.display->display();
}

/**
 * @brief This function writes a vector of strings and outputs them to the OLED display
 *
 * @param vector<String> lines: The strings to be displayed on screen
 * @param bool clear: If true, the display will not be cleared before writing the lines
 * @param bool writeToSerial: If true, the lines will also be written to the serial output
 * @return void
 */
void writeLines(std::vector<String> lines, bool clear, bool writeToSerial)
{
  if (clear)
  {
    Heltec.display->clear();
  }

  int row = 10;
  for (int i = 0; i < lines.size(); i++)
  {
    Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
    Heltec.display->setFont(ArialMT_Plain_10);
    Heltec.display->drawString(0, row, lines[i]);
    row += 10;

    if (writeToSerial)
    {
      Serial.println(lines[i]);
    }
  }

  Heltec.display->display();
}

void serialInit()
{
  // // Initialize serial and wait for port to open:
  // https://bit.ly/3CI7m59
  Serial.begin(SERIAL_DEBUG_BAUD);
  Serial.println();

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

}

void cloudInit()
{
  // !!! MUST BE CALLED BEFORE ANYTHING ELSE about cloud
  // Defined in thingProperties.h
  initProperties();

  wifi_connect();

  tb_connect();

}
