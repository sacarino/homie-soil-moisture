# Super Soil Moisture Sensor

A Homie node that figures out if your plants need some watery love.

<!-- TOC depthFrom:2 -->

- [Why?](#why)
- [Parts used](#parts-used)
- [How does it work](#how-does-it-work)
- [Dependencies](#dependencies)
- [Configuration](#configuration)

<!-- /TOC -->

## Why
I like plants, especially plants that grow food I can eat. However, I've got a million things going on, and I've been known to forget a watering or two.

Most people kill their plants, shrug, and give up. I decided to build a system that keeps me from being a killer. Step one is knowing the soil conditions.

## Parts used
This SUPER soil moisture sensor is made from these parts:
- 3D printed [sensor housing](https://www.thingiverse.com/thing:3658789) (Thanks for sharing, cabuu!) 
- D1 Mini ESP8266
- Capacitive Soil Moisture Sensor v1.2 (This is the only soil sensor you should buy. Really.)
- 3.7V 18650 Li-ion Battery
- Battery Holder for 18650
- TP4056 Charge Controller
- WS2818B RGB LED
- (optional) 6V 4.5W Solar Panel

I'll update the readme with build pics later.

## How does it work
After you put stuff together, you stick the business end of the sensor into dirt. The sensor thinks carefully, gets all judgy about the quality of your dirt, and tells you exactly what it thinks. Oh, you can have a cheery LED glow that cold judgement directly into your eyeballs, if you're not into the whole MQTT thing. (Odd, but whatever.)

That's pretty much it. ¯\_(ツ)_/¯

On the Homie side, this device exposes a `sensor` node. The following properties are exposed on that node:
- UOM (secret surprise: it's just `%`. Always. )
- Saturation (an integer value from 0 to 100)
- LED Color (what color the LED is showing)
- Soil Condition (a string saying WET, DRY, or OK)

The software is based on [Homie (v2.0)](https://github.com/marvinroger/homie-esp8266) and is developed using [PlatformIO](https://github.com/platformio)

## Dependencies

All dependencies are included in `platformio.ini`

## Configuration

This node uses a few custom settings in the Homie `config.json`:

```
{
    "settings": {
        "highMoistureThreshold": 81,
        "lowMoistureThreshold": 21,
        "sensorDry": 830,
        "sensorWet": 380,
        "ledFeedback": true,
        "deepSleep": false,
        "sleepSeconds": 300,
        "awakeSeconds": 20
    }
}
```

If you don't provide values, some sane defaults will be assumed. *They may not be right for you!*

#### Moisture Threshold

Both the `high` and `low` values are used to figure out if the soil is dry, good, or wet. These are going to be subjective based on what kind of soil conditions your plant wants. 

#### Sensor

Sensors can be tricky, and no two sensors have output the same range. The `dry` and `wet` values are here so you can calibrate to what the sensor reads when totally dry and when totally wet. _This is important!_ If you don't calibrate for your sensor, the results could be wildly incorrect.

#### LED Feedback

Pretty obvious, no? If you want a visual indicator of the soil conditions via an external LED, turn it on. Dry::Red, Wet::Blue, Ok::Green. 

#### Deep Sleep

If you're running on battery power, setting this true will trigger Homie's deep sleep routine. Note that normal rules about the ESP8266 and deep sleep apply.

#### Sleep Seconds

If you're using deep sleep, this is how long it'll deep sleep for. Otherwise, this is how long it'll wait between soil condition reporting.

#### Awake Seconds

If you're using deep sleep, this is how long to wait until calling `deepSleep` again. This is used to make sure the board stays awake long enough for you to send updated config values to it. (Like, for instance, if you wanted to disable deep sleep!)