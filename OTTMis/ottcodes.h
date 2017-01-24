#ifndef OTTCODES_H
#define OTTCODES_H



class OTTCode {

public:
    static constexpr int BASE_STATION_CODE  = 1234500000;

    static constexpr int BAT_LVL_CODE       = 1;

    static constexpr int PHI1_CODE          = 11;
    static constexpr int THETA1_CODE        = 12;

    static constexpr int PHI2_CODE          = 21;
    static constexpr int THETA2_CODE        = 22;
    static constexpr int ALPHA_CODE         = 23;

    static constexpr int ANGLES_CODES[5] = {
        11, 12, 21, 22, 23
    };

    static constexpr int ACC_X_CODE         = 110;
    static constexpr int ACC_Y_CODE         = 111;
    static constexpr int ACC_Z_CODE         = 112;

    static constexpr int MAG_X_CODE         = 120;
    static constexpr int MAG_Y_CODE         = 121;
    static constexpr int MAG_Z_CODE         = 122;

    static constexpr int ACC_MAG_CODES[6] = {
        110,111,112,120,121,122
    };

    static constexpr int LINEAR_CODE        = 200;

    static constexpr int TEMPERATURE_CODE   = 310;
    static constexpr int PRESSURE_CODE      = 310;
    static constexpr int HUMIDITY_CODE      = 320;


};

constexpr int OTTCode::ANGLES_CODES[5];
constexpr int OTTCode::ACC_MAG_CODES[6];

#endif // OTTCODES_H
