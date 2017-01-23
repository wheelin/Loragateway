# Loragateway

This repository provides a simple gateway software between a custom 
Lora-based protocol and the Internet, in general. It is based on the 
raspberry pi and the Dragino lora/gps module. 

This software is part of a bigger project and is used to convey data 
from many 3D extensometer nodes to the Internet. As it doesn't rely on
the LoraWAN protocol, a custom one has to be developed to allow multiple
nodes to connect to one gateway through a unique channel. 

This software must only be considered as a research project and must not
be used in any critical/real product application.  

## Context 

A raspberry pi is used as a gateway between nodes and the internet. The
radio module is a RFM95W module that cannot hear on multiple radio 
channels as the Lora concentrators do. But with a small protocol handling
nodes synchronization, it is possible to increase the nodes number. 


## Dependencies
This software depends on :
* Qt : DateTime, QDebug, QString, etc. Qt evenloop not used
* wiringPi : gpio and spi communication with the lora module

## Organization
### Data
This directory provides facilities to handle data coming from the 3D
extensometers nodes. It doesn't contain any kind of dependencies for the
rest of the software and can be removed for other applications. 

### Log 
Provides a convenient way to log formated messages into a desktop located
file. 

### OTTMis 
Abstraction between the OTT MIS format and the sensors topography. The 
OTT MIS format cat describe some basic sensors hierarchy and is used to
log the sensors measured data with a timestamp, etc. 

### Transmission
This directory contains the protocol handling and the "driver" for the
Lora RFM95W module. 
The driver can easily be ported to other platforms where the radio 
module is based on the SX1276 and the microcontroller provides interrupt
functionalities. 

## Workflow
This software is not complete at the moment and this section only 
describes the function goal. 

A 3D extensometer is a system that can measure ground movements in a 
3-dimensions coordinate system using a linear position sensor and some
angular position sensors. These data are sent periodically from the 
sensors system to a gateway that is always listening for incoming 
messages. When the gateway receives a message, it first checks if someone
on the Internet (most probably on an ftp server, not yet implemented) 
has left some message for the given node. If so, the gateway transmits
the message. Else, it just acknowledges the incoming message and returns 
into listening mode. 

The messages coming from nodes contain informations about sensors and 
battery level, etc. It is empacketed into an array of bytes and has to
be decoded. Then, these data are put into a OTT MIS file and sent to
the ftp server. 

The gateway responds to each message coming from the nodes with either a
ACK or an other message coming from the web. It knows how many nodes are 
cheating with it and then can schedule the future communications with the
nodes by synchronizing them with a packet field telling the nodes the 
next time in seconds they can start to communicate. 
