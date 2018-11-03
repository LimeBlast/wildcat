// Adafruit IO Publish & Subscribe Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Pushbutton.h>

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ LED Matrix *******************************/
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

/************************ Example Starts Here *******************************/

// Track time of last published messages and limit feed->save events to once
// every IO_LOOP_DELAY milliseconds.
//
// Because this sketch is publishing AND subscribing, we can't use a long
// delay() function call in the main loop since that would prevent io.run()
// from being called often enough to receive all incoming messages.
//
// Instead, we can use the millis() function to get the current time in
// milliseconds and avoid publishing until IO_LOOP_DELAY milliseconds have
// passed.
#define IO_LOOP_DELAY 10000
unsigned long lastUpdate = 0;

// Define some standard durations
#define ONE_MINUTE 60
#define ONE_HOUR (ONE_MINUTE * 60)
#define ONE_DAY (ONE_HOUR * 24)

// And the button input pin
#define BUTTON_PIN 2
Pushbutton button(BUTTON_PIN);

// To be updated with the latest value from the time/seconds feed
long timestamp = 0;
long lastIncident = 0;
String displayText = "";

// Subscribe to feeds
AdafruitIO_Feed *incident = io.feed("wildcat.incident");
AdafruitIO_Time *seconds = io.time(AIO_TIME_SECONDS);

void setup()
{
    // start the serial connection
    Serial.begin(9600);

    // wait for serial monitor to open
    while (!Serial)
    {
    }

    Serial.print("Connecting to Adafruit IO");

    // connect to io.adafruit.com
    io.connect();

    // set up handlers for the feeds
    incident->onMessage(handleMessage);
    seconds->onMessage(updateTimestamp);

    // wait for a connection
    while (io.status() < AIO_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    // we are connected
    Serial.println();
    Serial.println(io.statusText());

    // Begin the matrix
    matrix.begin(0x70);
    matrix.setTextSize(1);
    matrix.setTextWrap(false);
    matrix.setTextColor(LED_ON);
    matrix.setRotation(1);

    // get last value from the incident
    incident->get();
}

void loop()
{
    // io.run(); is required for all sketches.
    // it should always be present at the top of your loop
    // function. it keeps the client connected to
    // io.adafruit.com, and processes any incoming data.
    io.run();

    if (button.getSingleDebouncedRelease())
    {
        // save timestamp to the 'incident' feed on Adafruit IO
        Serial.print("sending -> ");
        Serial.println(timestamp);
        incident->save(timestamp);
    }

    if (millis() > (lastUpdate + IO_LOOP_DELAY))
    {
        calculateDifference();
        updateDisplay();

        // after publishing, store the current time
        lastUpdate = millis();
    }
}

// this function is called whenever a 'incident' message
// is received from Adafruit IO. it was attached to
// the incident feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data)
{
    lastIncident = data->toLong();
    Serial.print("received <- ");
    Serial.println(data->value());
}

// message handler for the seconds feed
void updateTimestamp(char *data, uint16_t len)
{
    timestamp = atol(data);
    // Serial.print("Seconds Feed: ");
    // Serial.println(data);
    // Serial.print("Timestamp: ");
    // Serial.println(timestamp);
}

String plural(String string, long value)
{
    if (value < 1 || value > 1)
    {
        string = string + "s";
    }
    return string;
}

void calculateDifference()
{
    long secondsDifference = 0;
    long minutesDifference = 0;
    long hoursDifference = 0;
    long daysDifference = 0;

    secondsDifference = timestamp - lastIncident;

    Serial.print("Time since last incident: ");

    if (secondsDifference < ONE_MINUTE)
    {
        displayText = secondsDifference + plural(" second", secondsDifference);
    }
    else if (secondsDifference < ONE_HOUR)
    {
        minutesDifference = secondsDifference / ONE_MINUTE;
        displayText = minutesDifference + plural(" minute", minutesDifference);
    }
    else if (secondsDifference < ONE_DAY)
    {
        hoursDifference = secondsDifference / ONE_HOUR;
        displayText = hoursDifference + plural(" hour", hoursDifference);
    }
    else
    {
        daysDifference = secondsDifference / ONE_DAY;
        displayText = daysDifference + plural(" day", daysDifference);
    }

    Serial.println(displayText);
}

void updateDisplay()
{
    for (int8_t x = 16; x >= -60; x--)
    {
        matrix.clear();
        matrix.setCursor(x, 0);
        matrix.print(displayText);
        matrix.writeDisplay();
        delay(100);
    }
}
