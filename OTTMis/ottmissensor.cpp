#include "ottmissensor.h"
#include <QDateTime>

OTTMisSensor::OTTMisSensor(int stationID, int sensorID)
{
    _header.clear();
    _header = QString("<STATION>%1</STATION><SENSOR>%2</SENSOR><DATEFORMAT>YYYYMMDD</DATAFORMAT>\n")
                .arg(stationID, 10, 10, QChar('0'))
                .arg(sensorID , 4, 10, QChar('0'));
    _stationID = stationID;
    _sensorID = sensorID;
}

void OTTMisSensor::addRecord(QDateTime &date, double value)
{
    _recordsList.append(
        QString("%1;%2;%3\n")
                .arg(date.toString("yyyyMMdd"))
                .arg(date.toString("hhmmss"))
                .arg(value, 0, 'g', 5, ' '));
}

QString OTTMisSensor::header()
{
    return _header;
}

QStringList &OTTMisSensor::getList()
{
    return _recordsList;
}

void OTTMisSensor::clearRecords()
{
    _recordsList.clear();
}

int OTTMisSensor::getSensorID()
{
    return _sensorID;
}
