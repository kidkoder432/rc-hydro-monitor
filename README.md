# rc-hydro-monitor
A simple data logger and dashboard for RC Hydrogen Club's racecars. 

This system uses two Arduinos and a Python script to log and display data about the race cars. Currently, the system supports voltage and current sensors. A refactoring of the code for easier extensibility is on the horizon. 

Technologies used:
- Python
- Eel
- Arduino
- ESP-NOW

Two previous systems were built using WiFi and Bluetooth LE. However, the constraints of a race environemtn made these systems perform poorly. 
In addition, this is the first iteration of the system with a visual dashboard, allowing for easy monitoring. 

## How to use this system:
> Note: **ONLY Arduino Nano ESP32 boards are supported!**
2 Arduinos are needed for this system.

1. After wiring up one Arduino (the "monitor") to your sensors, upload the sketch in `arduino_car` called `espnow_monitor.ino`.
2. Connect the other Arduino (the "slave" to your computer. Upload the sketch in `arduino_slave` called `espnow_slave.ino`.
3. On your computer, locate the COM port that the slave Arduino is connected to. (You can use Device Manager to do this on Windows). Copy the port name into the `PORT` constant in the Python script.
4. Install Python and `pip`, and then install `openpyxl`, `pyserial`, and `eel` using `pip`.
5. Turn on both Arduinos. Wait for a few seconds to establish a connection. Once a connection is established and the slave Arduino is receiving data, a green light will blink on the Arduino.
6. Start the Python script. A browser window will open and you should be able to see the voltage and current readings coming from the car, as well as an approximation of how long it is until they run out.

