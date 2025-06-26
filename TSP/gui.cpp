#include "gui.h"
#include "ui_gui.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QPen>
#include <QBrush>
#include <QInputDialog>

Gui::Gui(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Gui)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    compareScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->compareGraphicsView->setScene(compareScene);

    ui->graphicsView->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
    ui->compareGraphicsView->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);

    updateButtonsState();
}

Gui::~Gui()
{
    delete ui;
}

void Gui::updateButtonsState()
{
    ui->loadFileButton->setEnabled(true);
    ui->generateRandomButton->setEnabled(true);
    ui->inputCitiesButton->setEnabled(true);
}

void Gui::on_loadFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Текстовые файлы (*.txt)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    cities.clear();
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList fields = line.split(",");
        if (fields.size() == 3) {
            City p = {fields[0].toDouble(), fields[1].toDouble(), fields[2].toInt()};
            cities.append(p);
        }
    }
    file.close();
    drawSolution();
    drawCompareSolution();
    updateButtonsState();
}

void Gui::on_generateRandomButton_clicked()
{
    cities.clear();
    bool ok;
    int numCities = QInputDialog::getInt(this, "Количество городов", "Введите количество городов:", 10, 3, 1000, 1, &ok);
    if (!ok) return;

    for (int i = 0; i < numCities; ++i) {
        City c;
        c.x = QRandomGenerator::global()->bounded(0, 500);
        c.y = QRandomGenerator::global()->bounded(0, 500);
        c.priority = QRandomGenerator::global()->bounded(1, 10);
        cities.append(c);
    }
    drawSolution();
    drawCompareSolution();
    updateButtonsState();
}

void Gui::on_inputCitiesButton_clicked()
{
    readCitiesFromInput();
    drawSolution();
    drawCompareSolution();
    updateButtonsState();
}

void Gui::readCitiesFromInput()
{
    cities.clear();
    QString input = ui->textEdit->toPlainText();
    QStringList lines = input.split("\n");
    for (const QString &line : lines) {
        QStringList fields = line.split(",");
        if (fields.size() == 3) {
            bool xOk, yOk, priorityOk;
            double x = fields[0].toDouble(&xOk);
            double y = fields[1].toDouble(&yOk);
            int priority = fields[2].toInt(&priorityOk);
            if (xOk && yOk && priorityOk) {
                City p = {x, y, priority};
                cities.append(p);
            }
        }
    }
}

void Gui::drawSolution()
{
    scene->clear();
    if (cities.isEmpty()) return;

    double minX = cities[0].x, maxX = cities[0].x;
    double minY = cities[0].y, maxY = cities[0].y;
    for (const auto &p : cities) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    double paddingX = (maxX - minX) * 0.1;
    double paddingY = (maxY - minY) * 0.1;
    minX -= paddingX;
    maxX += paddingX;
    minY -= paddingY;
    maxY += paddingY;

    double scaleX = ui->graphicsView->width() / (maxX - minX);
    double scaleY = ui->graphicsView->height() / (maxY - minY);

    QPen pointPen(Qt::red);
    QBrush pointBrush(Qt::red);
    for (int i = 0; i < cities.size(); ++i) {
        const auto &p = cities[i];
        double x = (p.x - minX) * scaleX;
        double y = (p.y - minY) * scaleY;

        scene->addEllipse(x - 5, y - 5, 10, 10, pointPen, pointBrush);
        QGraphicsTextItem *label = scene->addText(QString("Город %1 (П: %2)").arg(i + 1).arg(p.priority));
        label->setPos(x + 5, y + 5);
    }
}

void Gui::drawCompareSolution()
{
    compareScene->clear();
    if (cities.isEmpty()) return;

    double minX = cities[0].x, maxX = cities[0].x;
    double minY = cities[0].y, maxY = cities[0].y;
    for (const auto &p : cities) {
        minX = std::min(minX, p.x);
        maxX = std::max(maxX, p.x);
        minY = std::min(minY, p.y);
        maxY = std::max(maxY, p.y);
    }

    double paddingX = (maxX - minX) * 0.1;
    double paddingY = (maxY - minY) * 0.1;
    minX -= paddingX;
    maxX += paddingX;
    minY -= paddingY;
    maxY += paddingY;

    double scaleX = ui->compareGraphicsView->width() / (maxX - minX);
    double scaleY = ui->compareGraphicsView->height() / (maxY - minY);

    QPen pointPen(Qt::red);
    QBrush pointBrush(Qt::red);
    for (int i = 0; i < cities.size(); ++i) {
        const auto &p = cities[i];
        double x = (p.x - minX) * scaleX;
        double y = (p.y - minY) * scaleY;

        compareScene->addEllipse(x - 5, y - 5, 10, 10, pointPen, pointBrush);
        QGraphicsTextItem *label = compareScene->addText(QString("Город %1 (П: %2)").arg(i + 1).arg(p.priority));
        label->setPos(x + 5, y + 5);
    }
}