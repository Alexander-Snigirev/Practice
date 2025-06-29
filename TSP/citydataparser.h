#ifndef CITYDATAPARSER_H
#define CITYDATAPARSER_H

#include <QVector>
#include <QString>
#include <QStringList>
#include "city.h"

class CityDataParser
{
public:
    CityDataParser();
    bool parseFromFile(const QString &fileName, QVector<City> &cities, QStringList &errorMessages);
    bool parseFromText(const QString &text, QVector<City> &cities, QStringList &errorMessages);

private:
    bool parseLine(const QString &line, City &city, QString &errorMessage);
};

#endif // CITYDATAPARSER_H