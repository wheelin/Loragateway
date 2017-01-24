#include "conversion.h"
#include <math.h>

Conversion::Conversion()
{
}

double Conversion::codeToAngle(uint16_t code)
{
    double angle = 360.0/((double)(ANG_SENSOR_MAX_CODE - ANG_SENSOR_MIN_CODE)) * ((double)code);
    if(angle < -180.0)
    {
        angle += 360.0;
    }
    else if(angle > 180.0)
    {
        angle -= 360.0;
    }
    return angle;
}

double Conversion::correctAngle(double a)
{
    if(a < -180.0)
    {
        return a + 360.0;
    }
    else if(a > 180.0)
    {
        return a - 360.0;
    }
    else
    {
        return a;
    }
}

double Conversion::measureToImpedance(int16_t cr, int16_t ci, int16_t mr, int16_t mi, int16_t r)
{
    double calMag = sqrt(((double)(cr*cr)) + ((double)(ci*ci)));
    double gainFactor = 1.0/((double)(r))/calMag;
    double measMag = sqrt(((double)(mr*mr)) + ((double)(mr*mr)));
    return 1.0/(gainFactor * measMag);
}

double Conversion::impedancetoDistance(double impedance)
{
    return  IMP_TO_DIST_COEFFS[0] * pow(impedance, 5) +
            IMP_TO_DIST_COEFFS[1] * pow(impedance, 4) +
            IMP_TO_DIST_COEFFS[2] * pow(impedance, 3) +
            IMP_TO_DIST_COEFFS[3] * pow(impedance, 2) +
            IMP_TO_DIST_COEFFS[4] * impedance         +
            IMP_TO_DIST_COEFFS[5];
}

double Conversion::accelerationCodeToG(int16_t accCode)
{
    return (((double)(accCode))*MG_PER_LSB)/1000.0;
}

double Conversion::magCodeToGauss(int16_t magCode)
{
    return (((double)(magCode))*MGAUSS_PER_LSB) * 1000.0;
}
