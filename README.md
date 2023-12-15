## AIS2203 - Sanntids datateknikk for kyberfysiske systemer Portfolio Project

This is a group project in the subject in AIS2203 - Sanntids datateknikk for kyberfysiske systemer. 
The project consist of creating software that enables communication from a PC using a Raspberry Pi and a Sphero RVR.


## PROJECT REQUIREMENTS
There had to be an established communication between the software on PC and the Sphero RVR and Raspberry PI.
We were free to choose communication protocol and format. (unsure if this is needed)


## SOFTWARE
The project is written using mainly C++ and python.

### C++:
The C++ code works by reading input from a user and sending commands to a Raspberry Pi using TCP. . . 

About the classes and how they work 

The project uses multiple libraries such as: "boost-asio", "catch2", "sdl2", "opencv4" and "nlohmann-json". 
These libraries are automatically installed by the project by CMake using vcpkg.
To run the project it is required to provide a -DCMAKE_TOOLCHAIN_FILE configuration for vcpgk.


### Python:
The python code is responsible for the communication between the C++ program and the Sphero RVR.\
\
it starts 3 TCP servers on port 8000,8001,8002 and sends/receives data.\
8000 for webcam stream\
8001 for receiveing commands in JSON format\
8002 for sending sensordata in JSON format\
\
Command JSON sends a few variables like \
```Command : "string" ``` to set operating mode either MANUAL or AUTO\
```direction : "string"``` either F = forward, L = left, R = right, B = backwards, S = stop\
```speed : "float"``` floatvalue between 0 to 1 representing the % of max speed you wana go.\
```heading : "int"``` interger between -100 to 100. 0 means straight forward -100 full left 100 full right\
\
Sensor JSON sends theese variables as a dictionaries\
```battery : "int"``` send battery percentage as an interger\
```"ColorSensor" : "{"R": "int", "G": "int","B": "int"}``` RBG valued from the color sensor beneath the rvr.\
```"Accelerometer" : "{"X": "float", "Y": "float","Z": "float"} ``` XYZ accel values rounded down to 3 decimals.\
```"AmbientLight"`: "float"``` Float value of the onboard ambient light sensor.\
\
The python code uses the external library "sphero_sdk" to communicate with the Sphero RVR.\
This library is included in the project file and requires no further installation.\
\
there is also a included discord script that sends the raspberry's ipv4 address to a webhook url if needed\
thanks to fhodun for his awesome repo https://github.com/fhodun/ip-webhook\
the script has been slightly modified to fit this project.\
\
## HARDWARE
The hardware used in this project was:\
- Raspberry Pi zero \
- sparkfun pi servohat https://www.sparkfun.com/products/15316\
- raspberry pi camera module v2\
- Sphero RVR.\
- A hard Flat tube was printed to avoid camera wobble while driving, look for file in CAD files\


## INSTALLATION

### Sphero Pi
update rasbian\
```sudo apt update
sudo apt dist-upgrade
sudo apt clean
sudo reboot```\
\
Install the sphero library\
```git clone https://github.com/sphero-inc/sphero-sdk-raspberrypi-python.git```\
navigate and execute first time setup\
```cd ~/sphero-sdk-raspberrypi-python
./first-time-setup.sh```\
run through the installer\
\
git clone the project repo onto the raspberry pi\
```git clone https://github.com/Meelinank/sanntid.git```\
\
find the iphandover file and insert your discord webhook should you want it\
```nano sanntid/Python/iphandover.py```\
\
run this command to update bootscript to run discord ip handler and python\
```sudo cp sanntid/Python/rc.local /etc/rc.local```\
the rvr should now properly start on boot.\
## UML?




