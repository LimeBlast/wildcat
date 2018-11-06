/************************** Configuration ***********************************/
#include "config.h"

/************************** Libraries ***********************************/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_LEDBackpack.h"
#include <Pushbutton.h>
#include "AdafruitIO_WiFi.h"

/************************** Serial ***********************************/
int baud = 9600;
void setupSerial()
{
    Serial.begin(baud); // start the serial connection
    while (!Serial)
    {
        // wait for serial monitor to open
    }
}

/************************** Adafruit IO ***********************************/
AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);

// Subscribe to feeds
AdafruitIO_Feed *incident = io.feed("wildcat.incident");
AdafruitIO_Time *seconds = io.time(AIO_TIME_SECONDS);

void setupAdafruitIo()
{
    Serial.print("Connecting to Adafruit IO");
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

    // get last value from the incident
    incident->get();
}

/************************ LED Matrix *******************************/
Adafruit_8x16minimatrix matrix = Adafruit_8x16minimatrix();

void setupMatrix()
{
    // Begin the matrix
    matrix.begin(0x70);
    matrix.setTextSize(1);
    matrix.setTextWrap(false);
    matrix.setTextColor(LED_ON);
    matrix.setRotation(1);
}

/************************ Pushbutton *******************************/
#define BUTTON_PIN 2
Pushbutton button(BUTTON_PIN);

/************************ Time calculations *******************************/
// Define some standard durations
#define ONE_MINUTE 60
#define ONE_HOUR (ONE_MINUTE * 60)
#define ONE_DAY (ONE_HOUR * 24)

// To be updated with the latest value from the time/seconds feed
long timestamp = 0;
long lastIncident = 0;
String displayText = "";

/************************ Setup *******************************/
void setup()
{
    setupSerial();
    setupAdafruitIo();
    setupMatrix();
}

/************************ Loop *******************************/
void loop()
{
    // io.run(); is required for all sketches.
    // it should always be present at the top of your loop
    // function. it keeps the client connected to
    // io.adafruit.com, and processes any incoming data.
    io.run();

    setDisplayText();

    for (int8_t x = 16; x >= getEndOfBuffer(); x--)
    {
        if (button.getSingleDebouncedPress())
        {
            reportIncident();
            lastIncident = timestamp;
            setDisplayText();
            x = 16; // resets the display loop
        }

        matrix.clear();
        matrix.setCursor(x, 0);
        matrix.print(displayText);
        matrix.writeDisplay();
        delay(100);
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

void reportIncident()
{
    // save timestamp to the 'incident' feed on Adafruit IO
    Serial.print("sending -> ");
    Serial.println(timestamp);
    incident->save(timestamp);
}

String plural(String string, long value)
{
    if (value < 1 || value > 1)
    {
        string = string + "s";
    }
    return string;
}

void setDisplayText()
{
    long secondsDifference = 0;
    long minutesDifference = 0;
    long hoursDifference = 0;
    long daysDifference = 0;

    secondsDifference = timestamp - lastIncident;

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

    Serial.print("Time since last incident: ");
    Serial.println(displayText);
}

int getEndOfBuffer()
{
    int16_t x1, y1;
    uint16_t w, h;
    matrix.getTextBounds(displayText, 0, 0, &x1, &y1, &w, &h);
    return w - w - w;
}