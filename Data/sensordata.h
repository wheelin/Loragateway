#ifndef SENSORDATA_H_INCLUDED
#define SENSORDATA_H_INCLUDED

#include <stdint.h>

class SensorData
{
public:
    static const uint8_t SD_ANG_PHI1_IDX           = 0;
    static const uint8_t SD_ANG_PHI2_IDX           = 2;
    static const uint8_t SD_ANG_THETA1_IDX         = 4;
    static const uint8_t SD_ANG_THETA2_IDX         = 6;
    static const uint8_t SD_ANG_ALPHA_IDX          = 8;
    static const uint8_t SD_ANG_DATA_LEN           = 10;

    static const uint8_t SD_LIN_CAL_REAL_IDX       = (SD_ANG_DATA_LEN + 0);
    static const uint8_t SD_LIN_CAL_IMG_IDX        = (SD_ANG_DATA_LEN + 2);
    static const uint8_t SD_LIN_MEAS_REAL_IDX      = (SD_ANG_DATA_LEN + 4);
    static const uint8_t SD_LIN_MEAS_IMG_IDX       = (SD_ANG_DATA_LEN + 6);
    static const uint8_t SD_LIN_CALIB_RESISTOR_IDX = (SD_ANG_DATA_LEN + 8);
    static const uint8_t SD_LIN_DATA_LEN           = 10;

    static const uint8_t SD_ENV_TEMPERATURE_IDX    = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + 0);
    static const uint8_t SD_ENV_PRESSURE_IDX       = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + 2);
    static const uint8_t SD_ENV_HUMIDITY_IDX       = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + 6);
    static const uint8_t SD_ENV_DATA_LEN           = 8;

    static const uint8_t SD_DIR_VERTICALITY_X_IDX  = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + 0);
    static const uint8_t SD_DIR_VERTICALITY_Y_IDX  = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + 2);
    static const uint8_t SD_DIR_VERTICALITY_Z_IDX  = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + 4);
    static const uint8_t SD_DIR_ORIENTATION_X_IDX  = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + 6);
    static const uint8_t SD_DIR_ORIENTATION_Y_IDX  = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + 8);
    static const uint8_t SD_DIR_ORIENTATION_Z_IDX  = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + 10);
    static const uint8_t SD_DIR_DATA_LEN           = 12;

    static const uint8_t SD_BATTERY_LEVEL_IDX      = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + SD_DIR_DATA_LEN + 0);
    static const uint8_t SD_BATTERY_LEVEL_DATA_LEN = 2;

    static const uint8_t SD_TIMESTAMP_IDX          = (SD_ANG_DATA_LEN + SD_LIN_DATA_LEN + SD_ENV_DATA_LEN + SD_DIR_DATA_LEN + SD_BATTERY_LEVEL_DATA_LEN);
    static const uint8_t SD_TIMESTAMP_DATA_LEN     = 4;

    static const uint8_t SD_REFERENCE_IDX          = (SD_ANG_DATA_LEN +
                                                      SD_LIN_DATA_LEN +
                                                      SD_ENV_DATA_LEN +
                                                      SD_DIR_DATA_LEN +
                                                      SD_BATTERY_LEVEL_DATA_LEN +
                                                      SD_TIMESTAMP_DATA_LEN);
    static const uint8_t SD_REFERENCE_DATA_LEN     = 1;

    static const uint8_t SENSOR_DATA_LENGTH        = (SD_ANG_DATA_LEN +
                                                      SD_LIN_DATA_LEN +
                                                      SD_ENV_DATA_LEN +
                                                      SD_DIR_DATA_LEN +
                                                      SD_BATTERY_LEVEL_DATA_LEN +
                                                      SD_TIMESTAMP_DATA_LEN +
                                                      SD_REFERENCE_DATA_LEN);

    SensorData();
    void fillWithAng(uint16_t phi1, uint16_t phi2,
                     uint16_t theta1, uint16_t theta2,
                     uint16_t alpha);
    void getAng(uint16_t &phi1, uint16_t &phi2,
                uint16_t &theta1, uint16_t &theta2,
                uint16_t &alpha);
    void fillWithLin(int16_t calreal, int16_t calimg,
                     int16_t measreal, int16_t measimg,
                     int16_t calres);
    void getLin(int16_t &calreal, int16_t &calimg,
                int16_t &measreal, int16_t &measimg,
                int16_t &calres);
    void fillWithEnv(int16_t temp, uint32_t pressure, uint16_t humidity);
    void getEnv(int16_t &temp, uint32_t &pressure, uint16_t &humidity);
    void fillWithDir(int16_t ax, int16_t ay, int16_t az,
                     int16_t mx, int16_t my, int16_t mz);
    void getDir(int16_t &ax, int16_t &ay, int16_t &az,
                int16_t &mx, int16_t &my, int16_t &mz);
    void fillWithBatLvl(uint16_t batLvl);
    void getBatLvl(uint16_t &batLvl);
    void fillWithTimeStamp(uint32_t timestamp);
    uint32_t getTimeStamp();

    void setAsReferenceData(bool state);
    bool isReferenceData();

    uint8_t * getData();
    void clear();
    uint8_t& operator[](const int16_t x);
    static int16_t toInt16(uint8_t msb, uint8_t lsb);
    static uint16_t toUint16(uint8_t msb, uint8_t lsb);
    static uint32_t toUint32(uint8_t msb, uint8_t midMsb, uint8_t midLsb, uint8_t lsb);

    uint8_t data[SENSOR_DATA_LENGTH];
};

#endif /* SENSORDATA_H_INCLUDED */
