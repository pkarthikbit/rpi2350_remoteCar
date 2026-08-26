# rpi2350_ha
Home Automation using RP2350 PICO2W

**To install MQTT:**
We are using the inbuilt example from rpi.

The example should publish its core temperature to the /temperature topic. You can subscribe to this topic from another machine.

```
mosquitto_sub -h $MQTT_SERVER -t '/temperature'
```

You can turn the led on and off by publishing messages.

```
mosquitto_pub -h $MQTT_SERVER -t '/led' -m on
mosquitto_pub -h $MQTT_SERVER -t '/led' -m off