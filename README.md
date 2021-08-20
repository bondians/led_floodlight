# Outdoor RGB LED Floodlight Replacement Controller

## Project aims
Create firmware for a custom designed controller based on the ESP32C3 SoC to replace the one provided in the target device, which uses a CIR (handheld remote) C&C interface. New controller should be able to accept commands sent over a WiFi network (and possibly BLE as well) to control light color/intensity. Enhancement to use WiFi and/or BLE mesh networking will be considered.

This controller is not intended to turn the lamp into a "smart device" that is controllable using smart home environments such as Amazon Alexa, Apple HomeKit or Google Home. Over IP (WiFi) the device will provide a simple socket interface (or something similarily lightweight) that can be accessed by an application running on a dedicated controller to provide command and control, likely with "vTree-like" functionality. If BLE functionality is incorporated, it will advertise as a GAT peripheral using vendor-defined GATT and characteristics.

## Target hardware
10W RGB LED Flood Light Outdoor, 16 Color Changing RGB Floodlight with Remote, Warm White to Daylight Tunable, IP65 Waterproof 4 Modes Dimmable Stage Lighting with US 3-Plug
[Amazon - econoLED - original but unavailable](https://amazon.com/gp/product/B005KYOE2Q)
[Amazon - Blinngo - alternative, available](https://amazon.com/dp/B01GCDV3Q8)

Target MCU for this project: Espressif ESP32-C3 SoC (RISCV5 core)

## Development environment
This project is targeted to the Espressif ESP32-C3 WiFi/BLE SoC using the Espressif-provided development environment and API.
Toolchain consists of GCC, CMake, Python scripts, and (optionally) Eclipse IDE.
API used is provided by Espressif: ESP-IDF, master branch v4.4

API documentation and setup instructions can be found [here](https://docs.espressif.com/projects/esp-idf/en/latest/esp32c3/get-started/index.html)

## More to come as the project details are fleshed out...
