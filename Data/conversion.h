#ifndef CONVERSION_H
#define CONVERSION_H

#include <stdint.h>

class Conversion
{
public:
    Conversion();

    static constexpr uint16_t ANG_SENSOR_MAX_CODE = 61734;
    static constexpr uint16_t ANG_SENSOR_MIN_CODE = 2595;

    static constexpr double IMP_TO_DIST_COEFFS[6] = {
        3e-10, -4e-7, 0.0002, -0.0491, 6.1236, -136.91
    };

    static double codeToAngle(uint16_t code);
    static double correctAngle(double a);
    static double measureToImpedance(int16_t cr, int16_t ci, int16_t mr, int16_t mi, int16_t r);
    static double impedancetoDistance(double impedance);

    static double refAngles[5];
    static double refImpedance;
    static double refDistance;
    static double refTemperature;
    static double refPressure;
    static double refAccelerations[3];
    static double refMagStrengths[3];

};

#endif // CONVERSION_H
