#include <Homie.h>
#include <Timer.h>
#include <FastLED.h>

#define MOISTUREPIN A0
Timer t;

// how often you want the sensor to update, in seconds
const int DEFAULT_SLEEP_INTERVAL = 30;
bool DEFAULT_DEEP_SLEEP = false;
unsigned long DEFAULT_WAIT_TO_SLEEP = 20UL;
HomieSetting<long> sleepDurationSetting("sleepSeconds", "Seconds between moisture readings");
HomieSetting<bool> deepSleepSetting("deepSleep", "Should the ESP8266 go into deep sleep?");
HomieSetting<long> awakeDurationSetting("awakeSeconds", "Seconds to remain awake after deep sleeping");

// placeholder for sensor value
unsigned long lastMoistureSent = 0;

// default values here for "wet" and "dry" soil calculations
const int DEFAULT_HIGH_MOISTURE = 85;   // At what % is the soil "wet"
const int DEFAULT_LOW_MOISTURE = 20;    // At what % is the soil "dry"
const int DEFAULT_DRY_VALUE = 830;      // What your dry sensor reads (in the air)
const int DEFAULT_WET_VALUE = 380;      // What your wet sensor reads (totally saturated)

// LED config
#define LED_PIN 4
#define COLOR_ORDER GRB
#define CHIPSET WS2812B
#define NUM_LEDS 1
#define BRIGHT 200
#define DIM 10
#define FRAMES_PER_SECOND 60
CRGB leds[NUM_LEDS];

bool DEFAULT_LED_STATUS = true;
HomieSetting<bool> ledFeedbackSetting("ledFeedback", "show visual indication of soil condition");

// first param is the name of the property in mqtt
HomieNode moistureNode("sensor", "sensor");

// Exposing settings for our wet and dry soil calculations
HomieSetting<long> sensorDrySetting("sensorDry", "The value (as an int) your sensor reports when dry (in the air)");
HomieSetting<long> sensorWetSetting("sensorWet", "The value (as an int) your sensor reports when wet (totally saturated)");
HomieSetting<long> highMoistureThresholdSetting("highMoistureThreshold", "The percentage (as an int) where the soil is 'wet'");
HomieSetting<long> lowMoistureThresholdSetting("lowMoistureThreshold", "The percentage (as an int) where the soil is 'dry'");

void prepareSleep() {
    Homie.prepareToSleep();
}

void clearLed() {
    leds[0] = CRGB::Black;
    FastLED.show();
}

void onHomieEvent(const HomieEvent& event) {
    bool deepSleep = deepSleepSetting.get();

    // if we're deep sleeping AND we've been awake for at least 10 seconds
    if (deepSleep)
    {
        const int duration = (sleepDurationSetting.get() * 1000000UL);
        switch (event.type)
        {
        case HomieEventType::MQTT_READY:
            Homie.getLogger() << "MQTT connected, preparing for deep sleep" << endl;
            t.after(100, prepareSleep);     // we don't increment the timer until after 20 seconds have passed via loop()
            break;
        case HomieEventType::READY_TO_SLEEP:
            Homie.getLogger() << "Ready to sleep" << endl;
            Homie.doDeepSleep(duration);
            break;
        default:
            break;
        }
    }
}

void setupHandler()
{
    moistureNode.setProperty("uom").send("%");
}

// Sets up some sane defaults in case there are no custom settings
void loadDefaults()
{
    // soil sensor settings
    highMoistureThresholdSetting.setDefaultValue(DEFAULT_HIGH_MOISTURE).setValidator([](long candidate) {
        return candidate > 0;
    });
    lowMoistureThresholdSetting.setDefaultValue(DEFAULT_LOW_MOISTURE).setValidator([](long candidate) {
        return candidate > 0;
    });
    sensorDrySetting.setDefaultValue(DEFAULT_DRY_VALUE).setValidator([](long candidate) {
        return candidate > 0;
    });
    sensorWetSetting.setDefaultValue(DEFAULT_WET_VALUE).setValidator([](long candidate) {
        return candidate > 0;
    });

    // LED settings
    ledFeedbackSetting.setDefaultValue(DEFAULT_LED_STATUS).setValidator([](bool candidate) {
        return !!candidate == candidate;
    });

    // Sleep-related settings
    deepSleepSetting.setDefaultValue(DEFAULT_DEEP_SLEEP).setValidator([](bool candidate) {
        return !!candidate == candidate;
    });
    sleepDurationSetting.setDefaultValue(DEFAULT_SLEEP_INTERVAL).setValidator([](long candidate) {
        return candidate > 0;
    });
    awakeDurationSetting.setDefaultValue(DEFAULT_WAIT_TO_SLEEP).setValidator([](unsigned long candidate) {
        return candidate >= DEFAULT_WAIT_TO_SLEEP;
    });
}

// here's how we update our sensor properties the Homie way
void loopHandler()
{
    unsigned long duration = (sleepDurationSetting.get() * 1000UL);
    if (millis() - lastMoistureSent >= duration || lastMoistureSent == 0)
    {
        // get current reading
        float reading = analogRead(MOISTUREPIN);
        
        // fetch our config settings
        int tooWet = highMoistureThresholdSetting.get();
        int tooDry = lowMoistureThresholdSetting.get();
        int calibratedWet = sensorWetSetting.get();
        int calibratedDry = sensorDrySetting.get();
        bool ledStatus = ledFeedbackSetting.get();

        // calculating the soil water content percentage using our calibrated sensor settings
        int percentMoist = 100 - map(reading, calibratedDry, calibratedWet, 100, 0);
        moistureNode.setProperty("saturation").send(String(percentMoist));

        // debugging output
        Homie.getLogger() << "💧 Moisture reading: " << percentMoist << "%" << endl;

        String label = "UNKNOWN";
        String colorVal = "UNKNOWN";
        if (percentMoist >= tooDry and percentMoist <= tooWet) // Goldilocks zone
        {
            label = "OK";
            colorVal = "Green";
            if (ledStatus)
            {
                leds[0] = CRGB::Green;
                FastLED.setBrightness(DIM);
                FastLED.show();
                Homie.getLogger() << "Soil condition: perfect 💚" << endl;
            } else 
            {
                clearLed();
            }
        }
        else if (percentMoist < tooDry) // needs water
        {
            label = "DRY";
            colorVal = "Red";
            if (ledStatus) {
                leds[0] = CRGB::Red;
                FastLED.setBrightness(BRIGHT);
                FastLED.show();
                Homie.getLogger() << "Soil condition: dry 🔴" << endl;
            }
            else
            {
                clearLed();
            }
        }
        else if (percentMoist > tooWet) // too much water
        {
            label = "WET";
            colorVal = "Blue";
            if (ledStatus) {
                leds[0] = CRGB::Blue;
                FastLED.setBrightness(DIM);
                FastLED.show();
                Homie.getLogger() << "Soil condition: wet 🔵" << endl;
            }
            else
            {
                clearLed();
            }
        }
        // setting our property
        moistureNode.setProperty("soil-condition").send(label);
        moistureNode.setProperty("led-color").send(colorVal); // Update the state of the led

        Homie.getLogger() << endl;
        lastMoistureSent = millis();
    }
}

void setup()
{
    // setting up our debugger
    Serial.begin(115200);
    Serial << endl << endl;

    // making sure our sensor pin is an input
    pinMode(MOISTUREPIN, INPUT);

    // ready to use LEDs
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHT);

    // humblebrag
    Homie_setFirmware("super-soil-moisture-sensor", "1.0.4");
    
    // Telling homie about our custom functions
    Homie.setSetupFunction(setupHandler).setLoopFunction(loopHandler);

    // Advertising our properties
    moistureNode.advertise("uom");
    moistureNode.advertise("saturation");
    moistureNode.advertise("led-color");
    moistureNode.advertise("soil-condition");

    // Setting default values for our settings
    loadDefaults();
    
    // so we can enable deep sleep 
    Homie.disableResetTrigger();

    // just in case you're on a shared port board
    Homie.disableLedFeedback();

    Homie.onEvent(onHomieEvent);

    // ...and go!
    Homie.setup();
}

void loop() {
    bool deepSleep = deepSleepSetting.get();
    unsigned long awakeDuration = awakeDurationSetting.get() * 1000UL;
    Homie.loop();

    // if deepSleeping, we need to update the timer
    // after awakeDuration has passed so that
    // we have the opportunity to push updated config
    // to the device once it's awake
    if (deepSleep && millis() > awakeDuration)
    {
        t.update();
    }
}
