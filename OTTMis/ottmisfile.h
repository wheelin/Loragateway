#ifndef OTTMISFILE_H
#define OTTMISFILE_H

#include <stdint.h>
#include <QString>
#include <QList>
#include "OTTMis/ottmissensor.h"

class OTTMisFile
{
public:
    OTTMisFile();
    OTTMisFile(int stationID);
    void setStationID(int stationID);
    QString getFileName();
    void insertMeasure(int sensorID, uint32_t timestamp, double value);
    void flush();

private:
    QString _fileName;
    int _stationID;
    QList<OTTMisSensor *> _sensorsList;
    bool _hasBeenFlushed;
};

#endif // OTTMISFILE_H
