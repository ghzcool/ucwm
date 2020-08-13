# ucwm
Universal Configurable Wireless Module for ESP8266

Main idea is to make wireless module that provides HTTP and WebSocket API for configuring IO pins as INPUT or OUTPUT and set or get values for these pins.
This way there will be no need to edit module code and flash it for different tasks and all changes will be made using API.

So user can start module, connect to WiFi Access Point of this module, set SSID and password of own WiFi so module connect to it.
After that user should be able to select INPUT or OUTPUT or NONE mode for pins, set pins as DIGITAL or ANALOG, set values for OUTPUT pins, get values from INPUT pins.
Should be possible to set more complex output options like Servo, Shim, etc.
Value transformations like min, max, map should be possible to set.
