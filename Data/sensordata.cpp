#include "sensordata.h"
#include "string.h"

SensorData::SensorData()
{

}

void SensorData::fillWithAng(uint16_t phi1, uint16_t phi2,
                             uint16_t theta1, uint16_t theta2,
                             uint16_t alpha)
{
    data[SD_ANG_PHI1_IDX]         = (uint8_t)(phi1 >> 8);
    data[SD_ANG_PHI1_IDX + 1]     = (uint8_t)(phi1 & 0xFF);
    data[SD_ANG_PHI2_IDX]         = (uint8_t)(phi2 >> 8);
    data[SD_ANG_PHI2_IDX + 1]     = (uint8_t)(phi2 & 0xFF);
    data[SD_ANG_THETA1_IDX]       = (uint8_t)(theta1 >> 8);
    data[SD_ANG_THETA1_IDX + 1]   = (uint8_t)(theta1 & 0xFF);
    data[SD_ANG_THETA2_IDX]       = (uint8_t)(theta2 >> 8);
    data[SD_ANG_THETA2_IDX + 1]   = (uint8_t)(theta2 & 0xFF);
    data[SD_ANG_ALPHA_IDX]        = (uint8_t)(alpha >> 8);
    data[SD_ANG_ALPHA_IDX + 1]    = (uint8_t)(alpha & 0xFF);
}

void SensorData::getAng(uint16_t &phi1, uint16_t &phi2,
                        uint16_t &theta1, uint16_t &theta2,
                        uint16_t &alpha)
{
    phi1   = toUint16(data[SD_ANG_PHI1_IDX]  ,data[SD_ANG_PHI1_IDX + 1]);
    phi2   = toUint16(data[SD_ANG_PHI2_IDX]  ,data[SD_ANG_PHI2_IDX + 1]);
    theta1 = toUint16(data[SD_ANG_THETA1_IDX],data[SD_ANG_THETA1_IDX + 1]);
    theta2 = toUint16(data[SD_ANG_THETA2_IDX],data[SD_ANG_THETA2_IDX + 1]);
    alpha  = toUint16(data[SD_ANG_ALPHA_IDX] ,data[SD_ANG_ALPHA_IDX + 1]);
}

void SensorData::fillWithLin(int16_t calreal, int16_t calimg,
                             int16_t measreal, int16_t measimg,
                             int16_t calres)
{
    data[SD_LIN_CAL_REAL_IDX]               = (uint8_t)(calreal >> 8);
    data[SD_LIN_CAL_REAL_IDX + 1]           = (uint8_t)(calreal & 0xFF);
    data[SD_LIN_CAL_IMG_IDX]                = (uint8_t)(calimg >> 8);
    data[SD_LIN_CAL_IMG_IDX + 1]            = (uint8_t)(calimg & 0xFF);
    data[SD_LIN_MEAS_REAL_IDX]              = (uint8_t)(measreal >> 8);
    data[SD_LIN_MEAS_REAL_IDX + 1]          = (uint8_t)(measreal & 0xFF);
    data[SD_LIN_MEAS_IMG_IDX]               = (uint8_t)(measimg >> 8);
    data[SD_LIN_MEAS_IMG_IDX + 1]           = (uint8_t)(measimg & 0xFF);
    data[SD_LIN_CALIB_RESISTOR_IDX]         = (uint8_t)(calres >> 8);
    data[SD_LIN_CALIB_RESISTOR_IDX + 1]     = (uint8_t)(calres & 0xFF);
}

void SensorData::getLin(int16_t &calreal, int16_t &calimg,
                        int16_t &measreal, int16_t &measimg,
                        int16_t &calres)
{
    calreal  = toInt16(data[SD_LIN_CAL_REAL_IDX]        ,data[SD_LIN_CAL_REAL_IDX + 1]);
    calimg   = toInt16(data[SD_LIN_CAL_IMG_IDX]         ,data[SD_LIN_CAL_IMG_IDX + 1]);
    measreal = toInt16(data[SD_LIN_MEAS_REAL_IDX]       ,data[SD_LIN_MEAS_REAL_IDX + 1]);
    measimg  = toInt16(data[SD_LIN_MEAS_IMG_IDX]        ,data[SD_LIN_MEAS_IMG_IDX + 1]);
    calres   = toInt16(data[SD_LIN_CALIB_RESISTOR_IDX]  ,data[SD_LIN_CALIB_RESISTOR_IDX + 1]);
}

void SensorData::fillWithEnv(int16_t temp, uint32_t pressure, uint16_t humidity)
{
    data[SD_ENV_TEMPERATURE_IDX]        = (uint8_t)(temp >> 8);
    data[SD_ENV_TEMPERATURE_IDX + 1]    = (uint8_t)(temp & 0xFF);
    data[SD_ENV_PRESSURE_IDX]           = (uint8_t)((pressure >> 24) & 0xFF);
    data[SD_ENV_PRESSURE_IDX + 1]       = (uint8_t)((pressure >> 16) & 0xFF);
    data[SD_ENV_PRESSURE_IDX + 2]       = (uint8_t)((pressure >> 8) & 0xFF);
    data[SD_ENV_PRESSURE_IDX + 3]       = (uint8_t)(pressure & 0xFF);
    data[SD_ENV_HUMIDITY_IDX]           = (uint8_t)(humidity >> 8);
    data[SD_ENV_HUMIDITY_IDX + 1]       = (uint8_t)(humidity & 0xFF);
}

void SensorData::getEnv(int16_t &temp, uint32_t &pressure, uint16_t &humidity)
{
    temp     =  toInt16(data[SD_ENV_TEMPERATURE_IDX], data[SD_ENV_TEMPERATURE_IDX + 1]);
    pressure =  toUint32(data[SD_ENV_PRESSURE_IDX], data[SD_ENV_PRESSURE_IDX + 1], data[SD_ENV_PRESSURE_IDX + 2], data[SD_ENV_PRESSURE_IDX + 3]);
    humidity =  toUint16(data[SD_ENV_HUMIDITY_IDX], data[SD_ENV_HUMIDITY_IDX + 1]);
}

void SensorData::fillWithDir(int16_t ax, int16_t ay, int16_t az, int16_t mx, int16_t my, int16_t mz)
{
    data[SD_DIR_VERTICALITY_X_IDX]      = (uint8_t)(ax >> 8);
    data[SD_DIR_VERTICALITY_X_IDX + 1]  = (uint8_t)(ax & 0xFF);
    data[SD_DIR_VERTICALITY_Y_IDX]      = (uint8_t)(ay >> 8);
    data[SD_DIR_VERTICALITY_Y_IDX + 1]  = (uint8_t)(ay & 0xFF);
    data[SD_DIR_VERTICALITY_Z_IDX]      = (uint8_t)(az >> 8);
    data[SD_DIR_VERTICALITY_Z_IDX + 1]  = (uint8_t)(az & 0xFF);

    data[SD_DIR_ORIENTATION_X_IDX]      = (uint8_t)(mx >> 8);
    data[SD_DIR_ORIENTATION_X_IDX + 1]  = (uint8_t)(mx & 0xFF);
    data[SD_DIR_ORIENTATION_Y_IDX]      = (uint8_t)(my >> 8);
    data[SD_DIR_ORIENTATION_Y_IDX + 1]  = (uint8_t)(my & 0xFF);
    data[SD_DIR_ORIENTATION_Z_IDX]      = (uint8_t)(mz >> 8);
    data[SD_DIR_ORIENTATION_Z_IDX + 1]  = (uint8_t)(mz & 0xFF);
}

void SensorData::getDir(int16_t &ax, int16_t &ay, int16_t &az, int16_t &mx, int16_t &my, int16_t &mz)
{
    ax = toInt16(data[SD_DIR_VERTICALITY_X_IDX], data[SD_DIR_VERTICALITY_X_IDX + 1]);
    ay = toInt16(data[SD_DIR_VERTICALITY_Y_IDX], data[SD_DIR_VERTICALITY_Y_IDX + 1]);
    az = toInt16(data[SD_DIR_VERTICALITY_Z_IDX], data[SD_DIR_VERTICALITY_Z_IDX + 1]);
    mx = toInt16(data[SD_DIR_ORIENTATION_X_IDX], data[SD_DIR_ORIENTATION_X_IDX + 1]);
    my = toInt16(data[SD_DIR_ORIENTATION_Y_IDX], data[SD_DIR_ORIENTATION_Y_IDX + 1]);
    mz = toInt16(data[SD_DIR_ORIENTATION_Z_IDX], data[SD_DIR_ORIENTATION_Z_IDX + 1]);
}

void SensorData::fillWithBatLvl(uint16_t batLvl)
{
    data[SD_BATTERY_LEVEL_IDX]      = (uint8_t)(batLvl >> 8);
    data[SD_BATTERY_LEVEL_IDX + 1]  = (uint8_t)(batLvl & 0xFF);
}

void SensorData::getBatLvl(uint16_t &batLvl)
{
    batLvl = toUint16(data[SD_BATTERY_LEVEL_IDX], data[SD_BATTERY_LEVEL_IDX + 1]);
}

void SensorData::fillWithTimeStamp(uint32_t timestamp)
{
    data[SD_TIMESTAMP_IDX]     = ((uint8_t)(timestamp >> 24));
    data[SD_TIMESTAMP_IDX + 1] = ((uint8_t)(timestamp >> 16));
    data[SD_TIMESTAMP_IDX + 2] = ((uint8_t)(timestamp >>  8));
    data[SD_TIMESTAMP_IDX + 3] = ((uint8_t)(timestamp >>  0));
}

uint32_t SensorData::getTimeStamp()
{
    return toUint32(data[SD_TIMESTAMP_IDX],
                    data[SD_TIMESTAMP_IDX + 1],
                    data[SD_TIMESTAMP_IDX + 2],
            data[SD_TIMESTAMP_IDX + 3]);
}

void SensorData::setAsReferenceData(bool state)
{
    if(state)
    {
        data[SD_REFERENCE_IDX] = 0xFF;
    }
    else
    {
        data[SD_REFERENCE_IDX] = 0x00;
    }
}

bool SensorData::isReferenceData()
{
    if(data[SD_REFERENCE_IDX] != 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t * SensorData::getData()
{
    return data;
}

void SensorData::clear()
{
    memset(data, 0, sizeof data);
}

uint8_t& SensorData::operator[](const int16_t x)
{
    if(x > SENSOR_DATA_LENGTH - 1)
    {
        return data[SENSOR_DATA_LENGTH - 1];
    }
    return data[x];
}

int16_t SensorData::toInt16(uint8_t msb, uint8_t lsb)
{
    return (int16_t)((((uint16_t)(msb)) << 8) + ((uint16_t)(lsb)));
}

uint16_t SensorData::toUint16(uint8_t msb, uint8_t lsb)
{
    return (((uint16_t)(msb)) << 8) + ((uint16_t)(lsb));
}

uint32_t SensorData::toUint32(uint8_t msb, uint8_t midMsb, uint8_t midLsb, uint8_t lsb)
{
    return (((uint32_t)(msb)) << 24) + (((uint32_t)(midMsb)) << 16) + (((uint32_t)(midLsb)) << 8) + ((uint32_t)(lsb));
}
