#ifndef TOWNDATAPARSER_H
#define TOWNDATAPARSER_H

#include <QString>
#include <QVector>
#include "town.h"

class TownDataParser
{
public:
    TownDataParser();
    bool parseFromFile(const QString &fileName, QVector<Town> &towns, QStringList &errorMessages);
    bool parseFromText(const QString &text, QVector<Town> &towns, QStringList &errorMessages);

private:
    bool parseLine(const QString &line, Town &town, QString &errorMessage, int lineNumber, const QVector<Town> &towns);
};

#endif // TOWNDATAPARSER_H