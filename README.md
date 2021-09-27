# Eaton devices project

This repository contains source codes for my device monitoring framework. This readme tries to explain general project architecture and its usage details.

Project consists of 2 seperate components:

1. Device hub and related modules.
2. Simulation related modules.

## Device hub

As the name suggests, this class implements a sort of device hub which collects data from an unbounded number of devices. It can be seen as a main application for a IOT gateway hardware implementation. Its responsibility is to:

    * Collect measurement data from various devices on its local network
    * Delegate collected information to some other, probably on an external network, aggregater. [This part is not implemented in this 
    project]

Device hub is designed to work on various communication protocols. For the scope of this project following transport protocols is implemented:

    * UDP transport
    * TCP transport
    * MQTT transport

Although all these transport layers are TCP/IP based, device hub is not limitted to IP protocols, it can be extended with serial or on-board communication protocols which are not implemented in the scope of this project.

Information collected from devices are organized in to different types of measurements. Any device can send any combination of measurements to the hub with any desired latency and bandwidth restrictions. For demonstration purposes following 3 types of sensor data is implemented:

    1. Temperature sensors
    2. Humidty sensors
    3. Pressure sensors

## Simulation

For demonstrating device hub's abilities, a very simulation component is implemented. Simulator performs following operations:

    1. Creates variable number of devices, with the selected transport layer.
    2. Each device collects data with a given sampling interval.
    3. Collected data are sent to device hub using selected transport with a given communication interval.

## Building and running the project

Project is developed with minimal external dependencies. Currently following components are needed:

1. cmake, as the build system.
2. C++17 compatiple compiler
3. libunwind, for meaningful runtime crash backtrace's.
4. sockpp, for TCP and UDP socket wrappers. (this is included with the source for x64 architecture)
5. ncurses, for statistics printing to the console.
6. loguru, for application log traces. (included in the project)

>loguru and sockpp is included with the source code for easy evaluation purposes.

After cloning, project can be built as:

```bash
cd $PROJECT_CLONE_PATH
mkdir build
cd build
cmake ..
make
```

>Project build is tested with Ubuntu 20.04 and latest Arch Linux. It should build with other Linux distro's without any modifications.

Then project can be run with default arguments:

```bash
./eaton_devices
```
By default program will start with 50 milliseconds communication interval, 10 milliseconds of sampling interval with 10 devices and TCP transport all of which can be overriden using command line parameters. Starting program with "--help" parameter will print description of available command line arguments.

Program will write transport type in use, number sensor devices available, total number of messages recv'ed, message count by measurement type, message count by device endpoint address and some additional communication statistics like instantaneous network bandwidth in use, sensor measurement latency and communication overhead. A sample output is as follows:

```bash
transport type is TCP
we have 10 sensor connections
recved 7298 messages up to now
        2438 pressure messages
        2430 humidity messages
        2430 temperature messages
recved 4421 messages in the last 3 seconds
average latency is calculated as 51 milliseconds
total bandwidth is is calculated as 229.71 kbits / sec
communication overhead is calculated as 59%
message count by device:
        127.0.0.1:38692: 486 messages
        127.0.0.1:38698: 486 messages
        127.0.0.1:38694: 486 messages
        127.0.0.1:38696: 972 messages
        127.0.0.1:38686: 972 messages
        127.0.0.1:38684: 491 messages
        127.0.0.1:38688: 972 messages
        127.0.0.1:38700: 972 messages
        127.0.0.1:38678: 491 messages
        127.0.0.1:38682: 972 messages
```

Instead showing devices by endpoint address, you visualize devices by their unique names by using '--names' flag:

```bash
./eaton_devices --names
```

>Please note that using name for the device name increases protocol overhead since devices need to send their unique names to the hub with each message.

You test UDP and MQTT transports as well:

```
./eaton_devices --udp
or
./eaton_devices --mqtt
```

Any other option listed in the help can be used in UDP and MQTT modes as well.
