# HomeAssistant remote
#### Remote control with ESP32-S3 and touch LCD

I am using an ESP32-S3 based devkit with touchscreen from Waveshare [This](https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-2.8)


## Goal 🥅:
Create a remote controller for the HomeAssistant server. 
I will use Wifi, make HTTP requests, parse json response, render HA entities on the screen. Process touch events.


## User Flow
* Device boots up
* User navigates to settings
* Connects to WiFi network where HA server is discoverable
* Discovers HA
* Queries devices, parses and puts controllable devices into folders by location.

To be continued.....

## Used components:
* esp-idf
* lvgl
* C lang

## Build:
TODO
