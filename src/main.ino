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

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"

/************************ Example Starts Here *******************************/

// this int will hold the current count for our sketch
int count = 0;

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
#define IO_LOOP_DELAY 5000
unsigned long lastUpdate = 0;

// To be updated with the latest value from the time/seconds feed
char *timestamp = 0;

// set up the 'incident' feed
AdafruitIO_Feed *incident = io.feed("wildcat.incident");

// set up the 'seconds' feed
AdafruitIO_Time *seconds = io.time(AIO_TIME_SECONDS);

void setup()
{
    // start the serial connection
    Serial.begin(9600);

    // wait for serial monitor to open
    while (!Serial);

    Serial.print("Connecting to Adafruit IO");

    // connect to io.adafruit.com
    io.connect();

    // set up a message handler for the count feed.
    // the handleMessage function (defined below)
    // will be called whenever a message is
    // received from adafruit io.
    incident->onMessage(handleMessage);
    seconds->onMessage(handleSecs);

    // wait for a connection
    while (io.status() < AIO_CONNECTED)
    {
        Serial.print(".");
        delay(500);
    }

    // we are connected
    Serial.println();
    Serial.println(io.statusText());

    // get last value
    incident->get();
}

void loop()
{
    // io.run(); is required for all sketches.
    // it should always be present at the top of your loop
    // function. it keeps the client connected to
    // io.adafruit.com, and processes any incoming data.
    io.run();

    if (millis() > (lastUpdate + IO_LOOP_DELAY))
    {
        // save count to the 'incident' feed on Adafruit IO
        Serial.print("sending -> ");
        Serial.println(count);
        incident->save(count);

        // increment the count by 1
        count++;

        // after publishing, store the current time
        lastUpdate = millis();
    }
}

// this function is called whenever a 'incident' message
// is received from Adafruit IO. it was attached to
// the incident feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data)
{
    Serial.print("received <- ");
    Serial.println(data->value());
}

// message handler for the seconds feed
void handleSecs(char *data, uint16_t len)
{
    Serial.print("Seconds Feed: ");
    Serial.println(data);
    timestamp = data;
}
