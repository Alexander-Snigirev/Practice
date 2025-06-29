#include "citydataparser.h"
#include <QFile>
#include <QTextStream>

CityDataParser::CityDataParser() {}

bool CityDataParser::parseFromFile(const QString &fileName, QVector<City> &cities, QStringList &errorMessages)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        errorMessages << QString("Не удалось открыть файл: %1").arg(fileName);
        return false;
    }

    cities.clear();
    QTextStream in(&file);
    int lineNumber = 0;
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        lineNumber++;
        if (line.isEmpty()) continue;

        City city;
        QString errorMessage;
        if (parseLine(line, city, errorMessage)) {
            cities.append(city);
        } else {
            errorMessages << QString("Строка %1: %2").arg(lineNumber).arg(errorMessage);
        }
    }
    file.close();

    if (cities.isEmpty()) {
        errorMessages << "Файл не содержит корректных данных о городах";
        return false;
    }
    return errorMessages.isEmpty();
}

bool CityDataParser::parseFromText(const QString &text, QVector<City> &cities, QStringList &errorMessages)
{
    cities.clear();
    QStringList lines = text.split("\n");
    int lineNumber = 0;
    for (const QString &line : lines) {
        QString trimmedLine = line.trimmed();
        lineNumber++;
        if (trimmedLine.isEmpty()) continue;

        City city;
        QString errorMessage;
        if (parseLine(trimmedLine, city, errorMessage)) {
            cities.append(city);
        } else {
            errorMessages << QString("Строка %1: %2").arg(lineNumber).arg(errorMessage);
        }
    }

    if (cities.isEmpty()) {
        errorMessages << "Введенный текст не содержит корректных данных о городах";
        return false;
    }
    return errorMessages.isEmpty();
}

bool CityDataParser::parseLine(const QString &line, City &city, QString &errorMessage)
{
    QStringList fields = line.split(",");
    if (fields.size() != 3) {
        errorMessage = QString("Неверное количество полей (ожидается 3, получено %1)").arg(fields.size());
        return false;
    }

    if (fields[0].isEmpty() || fields[1].isEmpty() || fields[2].isEmpty()) {
        errorMessage = "Одно или несколько полей пусты";
        return false;
    }

    bool xOk, yOk, priorityOk;
    double x = fields[0].toDouble(&xOk);
    double y = fields[1].toDouble(&yOk);
    int priority = fields[2].toInt(&priorityOk);

    if (!xOk || !yOk) {
        errorMessage = "Некорректный формат координат (x или y)";
        return false;
    }
    if (x < 0 || y < 0) {
        errorMessage = "Координаты x и y должны быть неотрицательными";
        return false;
    }
    if (!priorityOk || priority < 1) {
        errorMessage = "Некорректный приоритет (должен быть целым числом >= 1)";
        return false;
    }

    city = {x, y, priority};
    return true;
}