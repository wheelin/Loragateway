#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <unistd.h>

#include "Transmission/gwloramac.h"
#include "Transmission/packet.h"

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

// data packet
uint8_t data[256];

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
    gw_lora_mac::instance().initialize(txDoneCallback, rxDoneCallback, errorCallback);

    while (looping)
    {
        if(packetReceived)
        {
            packetReceived = false;
            // handle packet here
            gw_lora_mac::instance().get_last_data(nodeAddress, data, dataLength);
            int snr = gw_lora_mac::instance().get_snr();
            int pkt_rssi = gw_lora_mac::instance().get_pkt_rssi();
            int rssi = gw_lora_mac::instance().get_rssi();
            printf("========== From node 0x%02x =============\n", nodeAddress);
            printf("SNR : %d dB\n", snr);
            printf("Packet RSSI = %d dB\n", pkt_rssi);
            printf("RSSI = %d dB\n", rssi);

            printf("Received packet contains : \n");
            for(int i = 0; i < dataLength - 1; i++)
            {
                printf("%d) 0x%02X, ", i, data[i]);
            }
            printf("%d) 0x%02X\n", dataLength - 1, data[dataLength - 1]);
            printf("%s\n", data);
            memset(data, 0, 256);
        }
        if(errorHasHappened)
        {
            errorHasHappened = false;
            packetReceived = false;
        }
        usleep(10000);
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
