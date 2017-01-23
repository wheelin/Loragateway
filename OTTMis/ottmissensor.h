#ifndef OTTMISSENSOR_H
#define OTTMISSENSOR_H

#include <QStringList>
#include <QString>

class OTTMisSensor
{
public:
    OTTMisSensor(int stationID, int sensorID);
    void addRecord(QDateTime &date, double value);
    QString header();
    QStringList &getList();
    void clearRecords();

    int getSensorID();

private:
    int _stationID;
    int _sensorID;
    QString _header;
    QStringList _recordsList;
};

#endif // OTTMISSENSOR_H
