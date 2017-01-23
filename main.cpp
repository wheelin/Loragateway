#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "OTTMis/ottmisfile.h"
#include "OTTMis/OTTCodes.h"
#include "Transmission/gwloramac.h"
#include "Data/conversion.h"
#include "Data/sensordata.h"
#include "Log/log.h"
#include "Data/sensordata.h"

using namespace std;

// signal handling for ctrl-c
// stop loop execution
void onSignalReception(int sigNum);

// transmission callbacks

// callback for received packets. Only used for notification
void rxDoneCallback();

// callback to communicate error to the application layer
void errorCallback(int errCode);


// sensors data handling variables
SensorData pktData;
OTTMisFile dataFile[10];
uint8_t nodeAddress;
uint8_t dataLength;

// data
uint16_t angles[5];
double   convertedAngles[5];

int16_t  linvals[5];
double   impedance;
double   distance;

int16_t temperature;
double convertedTemperature;
uint32_t pressure;
uint16_t humidity;
double convertedHumidity;

int16_t accelerations[3];
double  convertedAccelerations[3];
int16_t magStrengths[3];
double  convertedMagStrengths[3];

uint16_t batteryLevel;

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
    GwLoraMac::instance().initialize(NULL, rxDoneCallback, errorCallback);

    Log::instance().message(Log::Critical, "Coucou %d\n", 23);
    Log::instance().message(Log::Major, "Coucou\n");
    Log::instance().message(Log::Minor, "Coucou\n");
    Log::instance().message(Log::Trivial, "Coucou\n");
    Log::instance().message(Log::Critical, "Coucou\n");

    while (looping)
    {
        if(packetReceived)
        {
            packetReceived = false;
            // handle packet here
            GwLoraMac::instance().getLastData(nodeAddress, pktData.getData(), dataLength);
            dataFile[nodeAddress] = OTTMisFile(OTTCode::BASE_STATION_CODE + nodeAddress);
            pktData.getAng(angles[0], angles[1], angles[2], angles[3], angles[4]);
            for(int i = 0; i < 5; i++)
            {
                dataFile[nodeAddress].insertMeasure(OTTCode::ANGLES_CODES[i],
                                                    pktData.getTimeStamp(),
                                                    Conversion::codeToAngle(angles[i]));
            }
            pktData.getLin(linvals[0], linvals[1], linvals[2], linvals[3], linvals[4]);
            impedance = Conversion::measureToImpedance(linvals[0], linvals[1], linvals[2], linvals[3], linvals[4]);
            distance = Conversion::impedancetoDistance(impedance);

        }
        if(errorHasHappened)
        {
            errorHasHappened = false;
        }
    }
    return 0;
}

void onSignalReception(int sigNum)
{
    (void)sigNum;
    looping = false;
}

void rxDoneCallback()
{
    packetReceived = true;
}

void errorCallback(int errCode)
{
    errorHasHappened = true;
    errorID = errCode;
}
