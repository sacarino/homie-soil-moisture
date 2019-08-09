# Super Soil Moisture Sensor

A Homie node that figures out if your plants need some watery love.

The following properties are exposed:
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
        "sleepSeconds": 300
    }
}
```

If you don't provide values, sane defaults will be assumed.

#### Moisture Threshold

Both the `high` and `low` values are used to figure out if the soil is dry, good, or wet. These are going to be subjective based on what kind of soil conditions your plant wants. 

#### Sensor

Sensors can be tricky, and no two sensors have output the same range. The `dry` and `wet` values are here so you can configure what the sensor reads when totally dry and when totally wet.

#### LED Feedback

Pretty obvious, no? If you want a visual indicator of the soil conditions via an external LED, turn it on. Dry::Red, Wet::Blue, Ok::Green. 

#### Deep Sleep

If you're running on battery power, setting this true will trigger Homie's deep sleep routine. Note that normal rules about the ESP8266 and deep sleep apply.

#### Sleep Seconds

If you're using deep sleep, this is how long it'll deep sleep for. Otherwise, this is how long it'll wait between soil condition reporting.