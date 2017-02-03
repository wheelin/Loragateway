# Loragateway
This repository provides a simple gateway software between a custom
Lora-based protocol and the Internet, in general. It is based on the
raspberry pi and the Dragino lora/gps module.

This software must only be considered as a research project and must not
be used in any critical/real product application.  

## Context
A raspberry pi is used as a gateway between nodes and the internet. The
radio module is a RFM95W module that cannot listen on multiple radio
channels as the Lora concentrators do. But with a small protocol handling
nodes synchronization, it is possible to increase the nodes number.


## Dependencies
This software depends on :
* wiringPi : gpio and spi communication with the lora module

## State
* Driver : works, tested between gateway and stm32 and RFM95 node
* Protocol : implemented, tested with one node. 
* synchronization : not implemented yet
