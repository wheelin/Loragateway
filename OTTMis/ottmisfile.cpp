#include "ottmisfile.h"
#include <stdio.h>
#include <QDateTime>
#include <QFile>
#include <QString>
#include <QList>
#include "OTTMis/ottmissensor.h"
#include "Log/log.h"

OTTMisFile::OTTMisFile()
{
    _hasBeenFlushed = false;
    _stationID = 0;
}

OTTMisFile::OTTMisFile(int stationID)
{
    _hasBeenFlushed = false;
    _stationID = stationID;
}

void OTTMisFile::setStationID(int stationID)
{
    _stationID = stationID;
}

QString OTTMisFile::getFileName()
{
    if(_hasBeenFlushed)
    {
        return _fileName;
    }
    else
    {
        return QString();
    }
}

void OTTMisFile::insertMeasure(int sensorID, uint32_t timestamp, double value)
{
    bool found = false;
    QDateTime cdt = QDateTime::fromTime_t((time_t)timestamp);
    for(OTTMisSensor * i : _sensorsList)
    {
        if(i->getSensorID() == sensorID)
        {
            i->addRecord(cdt, value);
            found = true;
        }
    }
    if(found == false)
    {
        _sensorsList.append(new OTTMisSensor(_stationID, sensorID));
        OTTMisSensor * truc = _sensorsList.at(_sensorsList.count() - 1);
        truc->addRecord(cdt, value);
    }
}

void OTTMisFile::flush()
{
    _fileName = QString("%1/%2_%3.MIS")
                .arg("/home/pi/Desktop")
                .arg(_stationID, 10, 10, QChar('0'))
                .arg(QDateTime::currentDateTime().toString("yyyyMMddHHmmss"));
    QFile file(_fileName);
    if(!file.open(QIODevice::WriteOnly))
    {
        printf("ERROR >> cannot open .MIS file for flush\n");
        file.close();
        return;
    }
    for(OTTMisSensor * s : _sensorsList)
    {
        file.write(s->header().toLocal8Bit());
        for(QString str : s->getList())
        {
            file.write(str.toLocal8Bit());
        }
    }
    for(OTTMisSensor * s : _sensorsList)
    {
        s->clearRecords();
    }
    _sensorsList.clear();
    file.close();
}

