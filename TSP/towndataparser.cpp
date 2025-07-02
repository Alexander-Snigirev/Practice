#include "towndataparser.h"
#include <QFile>
#include <QTextStream>

TownDataParser::TownDataParser() {}

bool TownDataParser::parseFromFile(const QString &fileName, QVector<Town> &towns, QStringList &errorMessages)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMessages << QString("Не удалось открыть файл: %1").arg(fileName);
        return false;
    }

    towns.clear();
    QTextStream in(&file);
    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;
        if (line.isEmpty() || line.startsWith("#")) continue;

        Town town;
        QString errorMessage;
        if (parseLine(line, town, errorMessage, lineNumber, towns)) {
            towns.append(town);
        } else {
            errorMessages << errorMessage;
        }
    }

    file.close();
    return errorMessages.isEmpty();
}

bool TownDataParser::parseFromText(const QString &text, QVector<Town> &towns, QStringList &errorMessages)
{
    towns.clear();
    QStringList lines = text.split('\n', Qt::SkipEmptyParts);
    int lineNumber = 0;

    for (const QString &line : lines) {
        QString trimmedLine = line.trimmed();
        lineNumber++;
        if (trimmedLine.isEmpty() || trimmedLine.startsWith("#")) continue;

        Town town;
        QString errorMessage;
        if (parseLine(trimmedLine, town, errorMessage, lineNumber, towns)) {
            towns.append(town);
        } else {
            errorMessages << errorMessage;
        }
    }

    return errorMessages.isEmpty();
}

bool TownDataParser::parseLine(const QString &line, Town &town, QString &errorMessage, int lineNumber, const QVector<Town> &towns)
{
    QStringList parts = line.split(',', Qt::SkipEmptyParts);
    if (parts.size() != 3) {
        errorMessage = QString("Строка %1: неверный формат, ожидается 'x,y,priority'").arg(lineNumber);
        return false;
    }

    bool xOk, yOk, priorityOk;
    double x = parts[0].trimmed().toDouble(&xOk);
    double y = parts[1].trimmed().toDouble(&yOk);
    int priority = parts[2].trimmed().toInt(&priorityOk);

    if (!xOk || !yOk || !priorityOk || x < 0 || y < 0 || priority < 1 || priority > 10) {
        errorMessage = QString("Строка %1: неверные значения x, y или priority (x >= 0, y >= 0, 1 <= priority <= 10)").arg(lineNumber);
        return false;
    }

    town.x = x;
    town.y = y;
    town.priority = priority;
    town.name = QString("Город %1").arg(towns.size() + 1).toStdString();
    return true;
}