#include <iostream>
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "OTTMis/ottmisfile.h"
#include "OTTMis/ottcodes.h"
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

// callback for sent packets
void txDoneCallback();

// callback to communicate error to the application layer
void errorCallback(uint8_t errCode);


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
uint32_t pressure;
uint16_t humidity;

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
    signal(SIGINT, onSignalReception);
    GwLoraMac::instance().initialize(txDoneCallback, rxDoneCallback, errorCallback);

    while (looping)
    {
        if(packetReceived)
        {
            Log::instance().message(Log::Severity::Note, false, "Handling received packet");
            packetReceived = false;
            // handle packet here
            GwLoraMac::instance().getLastData(nodeAddress, pktData.getData(), dataLength);
            uint8_t nodeIdx = nodeAddress - 1;
            dataFile[nodeIdx] = OTTMisFile(OTTCode::BASE_STATION_CODE + nodeAddress);
            pktData.getAng(angles[0], angles[1], angles[2], angles[3], angles[4]);
            for(int i = 0; i < 5; i++)
            {
                dataFile[nodeIdx].insertMeasure(OTTCode::ANGLES_CODES[i],
                                                    pktData.getTimeStamp(),
                                                    Conversion::codeToAngle(angles[i]));
            }
            pktData.getLin(linvals[0], linvals[1], linvals[2], linvals[3], linvals[4]);
            impedance = Conversion::measureToImpedance(linvals[0], linvals[1], linvals[2], linvals[3], linvals[4]);
            dataFile[nodeIdx].insertMeasure(OTTCode::LINEAR_CODE, pktData.getTimeStamp(), Conversion::impedancetoDistance(impedance));
            pktData.getDir(accelerations[0], accelerations[1], accelerations[2], magStrengths[0], magStrengths[1], magStrengths[2]);
            for(int i = 0; i < 6; i++)
            {
                if(i < 3)
                {
                    dataFile[nodeIdx].insertMeasure(OTTCode::ACC_MAG_CODES[i],
                                                        pktData.getTimeStamp(),
                                                        Conversion::accelerationCodeToG(accelerations[i]));
                }
                else
                {
                    dataFile[nodeIdx].insertMeasure(OTTCode::ACC_MAG_CODES[i],
                                                        pktData.getTimeStamp(),
                                                        Conversion::magCodeToGauss(magStrengths[i]));
                }
            }
            pktData.getEnv(temperature, pressure, humidity);
            dataFile[nodeIdx].insertMeasure(OTTCode::TEMPERATURE_CODE, pktData.getTimeStamp(), (double)temperature);
            dataFile[nodeIdx].insertMeasure(OTTCode::PRESSURE_CODE, pktData.getTimeStamp(), (double)pressure);
            dataFile[nodeIdx].insertMeasure(OTTCode::HUMIDITY_CODE, pktData.getTimeStamp(), (double)humidity);

            pktData.getBatLvl(batteryLevel);
            dataFile[nodeIdx].insertMeasure(OTTCode::BAT_LVL_CODE, pktData.getTimeStamp(), (double)batteryLevel);
            dataFile[nodeIdx].flush();
            Log::instance().message(Log::Severity::Note, false, "OTT MIS file has been written");
        }
        if(errorHasHappened)
        {
            Log::instance().message(Log::Severity::Error, false, "error code : %d", errorID);
            errorHasHappened = false;
            packetReceived = false;
        }
    }
    Log::instance().message(Log::Severity::Note, false, "Stopping loop ");
    return 0;
}

void onSignalReception(int sigNum)
{
    Log::instance().message(Log::Severity::Note, false, "SIGNAL received : %d", sigNum);
    looping = false;
}

void txDoneCallback()
{
    Log::instance().message(Log::Severity::Note, false, "Radio mode : %d", GwLoraMac::instance().getRadioMode());
}

void rxDoneCallback()
{
    Log::instance().message(Log::Severity::Note, false, "packet receive notification to the application");
    packetReceived = true;
}

void errorCallback(uint8_t errCode)
{
    errorHasHappened = true;
    errorID = errCode;
}
