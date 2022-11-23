# Clown Scale

Test application for HX711 library for BigClown modules.

Visit [Hacksters.io](https://www.hackster.io/matejus/clown-scales-13d6cb) to read the story.

|Project components |
|-|
|[BigClown Core module](https://developers.hardwario.com/hardware/about-core-module) REL v1 and v2 supported |
|[Battery module](https://developers.hardwario.com/hardware/about-battery-module)  |
|[LCD module](https://developers.hardwario.com/hardware/about-lcd-module)   |
|[HX711 module](https://arduino-shop.cz/arduino/998-ad-prevodnik-modul-24-bit-2-kanaly-hx711.html) 24bit A/D Converter for weight scales |
|[Scale sensors](https://arduino-shop.cz/arduino/2202-vahovy-senzor-50-kg.html)     |

This simple application shows how to use library for 24bit A/D converter HX711 (AVIA Semiconductor) with BigClown kit. The library is [Bodge/HX711](https://github.com/bogde/HX711) project ported to BigClown.

HX711 product sheet and communication principles are available [here](https://www.mouser.com/datasheet/2/813/hx711_english-1022875.pdf). The scale sensors (in my case sensors from personal scale) have to connected to the [Wheatstone bridge](https://en.wikipedia.org/wiki/Wheatstone_bridge).

To initialize library use `hx711_init()` method to specify your GPIO pins used for communication and selected channel on HX711 module (how the sensors are connected). When you want to measure battery do not use GPIO_0 for communication. For init method the pins are called PD_SCK (power down/serial clock) and DOUT (data output).

To periodic measure register the event and measure period using `hx711_set_event_handler()` and `hx711_set_update_interval()` methods.

You can read raw measured data, the result is 24bit number representing the measured voltage. To use the module as a scale you have to tare (set zero value) and calibrate (set vaue for known weight) your device using `hx711_tare()` and `hx711_calibrate()` methods. This configuration can be stored in EEPROM - `hx711_save()` and `hx711_load()`.

To save battery use `hx711_power_down()` when you don't measure.

------------------

Added Sensor and button module based on https://github.com/hardwario/twr-radio-door-sensor/


