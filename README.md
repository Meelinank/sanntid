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
The python code is responsible for the communication between the C++ program and the Sphero RVR.
...

The python code uses the external library "sphero_sdk" to communicate with the Sphero RVR.
This library is included in the project file and requires no further installation.


## HARDWARE
The hardware used in this project is a Raspberry Pi zero and a Sphero RVR.

## INSTALLATION

### Sphero Pi
update rasbian
``` sudo apt update```
```sudo apt dist-upgrade```
```sudo apt clean```
```sudo reboot  ```

Install the sphero library
```git clone https://github.com/sphero-inc/sphero-sdk-raspberrypi-python.git```
navigate and execute first time setup
```cd ~/sphero-sdk-raspberrypi-python```
```./first-time-setup.sh```
run through the installer

git clone the project repo onto the raspberry pi
```git clone https://github.com/Meelinank/sanntid.git```

find the iphandover file and insert your discord webhook should you want it
```nano sanntid/Python/iphandover.py```

run this command to update bootscript to run discord ip handler and python
```sudo cp sanntid/Python/rc.local /etc/rc.local```
the rvr should now properly start on boot.
## UML?




