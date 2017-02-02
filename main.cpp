#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "Transmission/gwloramac.h"

using namespace std;

// signal handling for ctrl-c
// stop loop execution
void onSignalReception(int sigNum);

// transmission callbacks

// callback for received packets. Only used for notification
void rxDoneCallback();

// callback for sent packets
void txDoneCallback();

// callback to communicate error to the application layer
void errorCallback(uint8_t errCode);

// data from MAC
uint8_t nodeAddress;
uint8_t dataLength;


// variables modified by the callbacks
volatile bool looping;
volatile bool packetReceived;
volatile bool errorHasHappened;
volatile int  errorID;

// error check variable
int ret;

int main()
{
    looping = true;
    signal(SIGINT, onSignalReception);
    GwLoraMac::instance().initialize(txDoneCallback, rxDoneCallback, errorCallback);

    while (looping)
    {
        if(packetReceived)
        {
            // handle packet here
            GwLoraMac::instance().getLastData(nodeAddress, pktData.getData(), dataLength);
            
        }
        if(errorHasHappened)
        {
            errorHasHappened = false;
            packetReceived = false;
        }
    }
    return 0;
}

void onSignalReception(int sigNum)
{
    looping = false;
}

void txDoneCallback()
{

}

void rxDoneCallback()
{
    packetReceived = true;
}

void errorCallback(uint8_t errCode)
{
    errorHasHappened = true;
    errorID = errCode;
}
