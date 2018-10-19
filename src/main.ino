/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board:
  ----> https://www.adafruit.com/product/2471

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>      //ESP8266 Core WiFi Library
#include <DNSServer.h>        //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h> //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>      //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "config.h"

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, MQTT_SERVER, MQTT_SERVERPORT, MQTT_USERNAME, MQTT_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Subscribe input = Adafruit_MQTT_Subscribe(&mqtt, MQTT_USERNAME "/feeds/wildcat.incident");
Adafruit_MQTT_Publish output = Adafruit_MQTT_Publish(&mqtt, MQTT_USERNAME "/feeds/wildcat.incident");

/****************************** Values **************************************/

int timestamp = 0;

/*************************** Sketch Code ************************************/

void inputcallback(double x)
{
    Serial.print("Input value is: ");
    Serial.println((int)x);
    timestamp = (int)x;
}

void setup()
{
    Serial.begin(9600);
    delay(10);

    // updateStatus();

    WiFiManager wifiManager;
    wifiManager.setTimeout(180);

    if (!wifiManager.autoConnect("Wildcat"))
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        ESP.reset();
        delay(5000);
    }

    Serial.println("connected...yeey :)");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    input.setCallback(inputcallback);

    mqtt.subscribe(&input);
}

uint32_t x = 0;

void loop()
{
    // Ensure the connection to the MQTT server is alive (this will make the first
    // connection and automatically reconnect when disconnected).  See the MQTT_connect
    // function definition further below.
    MQTT_connect();

    // this is our 'wait for incoming subscription packets and callback em' busy subloop
    // try to spend your time here:
    mqtt.processPackets(10000);

    // updateStatus();

    // ping the server to keep the mqtt connection alive
    // NOT required if you are publishing once every KEEPALIVE seconds
    if (!mqtt.ping())
    {
        mqtt.disconnect();
    }
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect()
{
    int8_t ret;

    // Stop if already connected.
    if (mqtt.connected())
    {
        return;
    }

    Serial.print("Connecting to MQTT... ");

    uint8_t retries = 3;
    while ((ret = mqtt.connect()) != 0)
    { // connect will return 0 for connected
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("Retrying MQTT connection in 10 seconds...");
        mqtt.disconnect();
        delay(10000); // wait 10 seconds
        retries--;
        if (retries == 0)
        {
            // basically die and wait for WDT to reset me
            while (1)
                ;
        }
    }
    Serial.println("MQTT Connected!");
}