#include "gui.h"
#include "ui_gui.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>
#include <QInputDialog>
#include <QtCharts/QLineSeries>
#include <QThread>
#include <cmath>
#include <random>
#include <algorithm>

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

    // Инициализация графика
    setupChart();
    currentGeneration = 0;

    updateButtonsState();
}

Gui::~Gui()
{
    delete ui;
}

void Gui::setupChart()
{
    bestFitnessSeries = new QLineSeries();
    bestFitnessSeries->setName("Лучшая приспособленность");
    bestFitnessSeries->setColor(Qt::red);

    avgFitnessSeries = new QLineSeries();
    avgFitnessSeries->setName("Средняя приспособленность");
    avgFitnessSeries->setColor(Qt::blue);

    chart = new QChart();
    chart->addSeries(bestFitnessSeries);
    chart->addSeries(avgFitnessSeries);
    chart->createDefaultAxes();
    chart->setTitle("Динамика приспособленности по поколениям");
    chart->legend()->setVisible(true);
    chart->setTitleFont(QFont("Arial", 12));
    chart->axes(Qt::Horizontal).first()->setTitleFont(QFont("Arial", 10));
    chart->axes(Qt::Vertical).first()->setTitleFont(QFont("Arial", 10));
    chart->legend()->setFont(QFont("Arial", 10));

    chart->axes(Qt::Horizontal).first()->setTitleText("Поколение");
    chart->axes(Qt::Vertical).first()->setTitleText("Приспособленность");

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(600, 400);

    ui->verticalLayout_3->addWidget(chartView);
}

void Gui::updateChart(int generation, double bestFitness, double avgFitness)
{
    bestFitnessSeries->append(generation, bestFitness);
    avgFitnessSeries->append(generation, avgFitness);

    chart->axes(Qt::Horizontal).first()->setRange(0, generation + 1);
    double maxFitness = std::max(bestFitness, avgFitness);
    chart->axes(Qt::Vertical).first()->setRange(0, maxFitness * 1.1);

    ui->generationLabel->setText(QString("Поколение: %1").arg(generation));
    ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(bestFitness, 0, 'f', 2));
    ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(avgFitness, 0, 'f', 2));

    QApplication::processEvents();
}

void Gui::updateButtonsState()
{
    ui->loadFileButton->setEnabled(true);
    ui->generateRandomButton->setEnabled(true);
    ui->inputCitiesButton->setEnabled(true);
    ui->runStepButton->setEnabled(!cities.isEmpty());
    ui->runToEndButton->setEnabled(!cities.isEmpty());
}

void Gui::on_loadFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Текстовые файлы (*.txt)", nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл");
        return;
    }

    cities.clear();
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue; // Пропускаем пустые строки
        QStringList fields = line.split(",");
        if (fields.size() == 3 && !fields[0].isEmpty() && !fields[1].isEmpty() && !fields[2].isEmpty()) {
            bool xOk, yOk, priorityOk;
            double x = fields[0].toDouble(&xOk);
            double y = fields[1].toDouble(&yOk);
            int priority = fields[2].toInt(&priorityOk);
            if (xOk && yOk && priorityOk) {
                City p = {x, y, priority};
                cities.append(p);
            } else {
                QMessageBox::warning(this, "Ошибка", QString("Некорректные данные в строке: %1").arg(line));
            }
        } else {
            QMessageBox::warning(this, "Ошибка", QString("Неправильный формат строки: %1").arg(line));
        }
    }
    file.close();
    bestSolution.clear();
    currentGeneration = 0;
    bestFitnessSeries->clear();
    avgFitnessSeries->clear();
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
        QString trimmedLine = line.trimmed();
        if (trimmedLine.isEmpty()) continue; // Пропускаем пустые строки
        QStringList fields = trimmedLine.split(",");
        if (fields.size() == 3 && !fields[0].isEmpty() && !fields[1].isEmpty() && !fields[2].isEmpty()) {
            bool xOk, yOk, priorityOk;
            double x = fields[0].toDouble(&xOk);
            double y = fields[1].toDouble(&yOk);
            int priority = fields[2].toInt(&priorityOk);
            if (xOk && yOk && priorityOk) {
                City p = {x, y, priority};
                cities.append(p);
            } else {
                QMessageBox::warning(this, "Ошибка", QString("Некорректные данные в строке: %1").arg(trimmedLine));
            }
        } else {
            QMessageBox::warning(this, "Ошибка", QString("Неправильный формат строки: %1").arg(trimmedLine));
        }
    }
    bestSolution.clear();
    currentGeneration = 0;
    bestFitnessSeries->clear();
    avgFitnessSeries->clear();
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
    bestSolution.clear();
    currentGeneration = 0;
    bestFitnessSeries->clear();
    avgFitnessSeries->clear();
    drawSolution();
    drawCompareSolution();
    updateButtonsState();
}

void Gui::on_inputCitiesButton_clicked()
{
    readCitiesFromInput();
    bestSolution.clear();
    currentGeneration = 0;
    bestFitnessSeries->clear();
    avgFitnessSeries->clear();
    drawSolution();
    drawCompareSolution();
    updateButtonsState();
}

void Gui::on_runStepButton_clicked()
{
    if (cities.isEmpty()) return;

    // Имитация шага генетического алгоритма
    currentGeneration++;
    double bestFitness = 1.0 + currentGeneration * 0.5 + (rand() % 10) * 0.1;
    double avgFitness = 1.0 + currentGeneration * 0.3 + (rand() % 5) * 0.1;

    // Генерация случайного решения для демонстрации
    bestSolution.clear();
    QVector<int> indices(cities.size());
    for (int i = 0; i < cities.size(); ++i) indices[i] = i;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);
    bestSolution = indices;

    updateChart(currentGeneration, bestFitness, avgFitness);
    drawSolution();
}

void Gui::on_runToEndButton_clicked()
{
    if (cities.isEmpty()) return;

    simulateAlgorithm();
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
    QVector<QPointF> points;
    for (int i = 0; i < cities.size(); ++i) {
        const auto &p = cities[i];
        double x = (p.x - minX) * scaleX;
        double y = (p.y - minY) * scaleY;
        points.append(QPointF(x, y));

        scene->addEllipse(x - 5, y - 5, 10, 10, pointPen, pointBrush);
        QGraphicsTextItem *label = scene->addText(QString("Город %1 (П: %2)").arg(i + 1).arg(p.priority));
        label->setPos(x + 5, y + 5);
    }

    drawBestSolutionPath(bestSolution);
}

void Gui::drawBestSolutionPath(const QVector<int> &solution)
{
    if (solution.isEmpty()) return;

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

    QPen linePen(Qt::blue);
    linePen.setWidth(2);
    for (int i = 0; i < solution.size() - 1; ++i) {
        int city1 = solution[i];
        int city2 = solution[i + 1];
        double x1 = (cities[city1].x - minX) * scaleX;
        double y1 = (cities[city1].y - minY) * scaleY;
        double x2 = (cities[city2].x - minX) * scaleX;
        double y2 = (cities[city2].y - minY) * scaleY;
        scene->addLine(x1, y1, x2, y2, linePen);
    }

    if (!solution.isEmpty()) {
        int firstCity = solution[0];
        int lastCity = solution[solution.size() - 1];
        double x1 = (cities[lastCity].x - minX) * scaleX;
        double y1 = (cities[lastCity].y - minY) * scaleY;
        double x2 = (cities[firstCity].x - minX) * scaleX;
        double y2 = (cities[firstCity].y - minY) * scaleY;
        scene->addLine(x1, y1, x2, y2, linePen);
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

void Gui::simulateAlgorithm()
{
    currentGeneration = 0;
    bestSolution.clear();
    bestFitnessSeries->clear();
    avgFitnessSeries->clear();

    int maxGenerations = ui->maxGenSpinBox->value();
    std::random_device rd;
    std::mt19937 g(rd());

    for (int generation = 0; generation < maxGenerations; ++generation) {
        currentGeneration = generation;
        double bestFitness = 1.0 + generation * 0.5 + (rand() % 10) * 0.1;
        double avgFitness = 1.0 + generation * 0.3 + (rand() % 5) * 0.1;

        bestSolution.clear();
        QVector<int> indices(cities.size());
        for (int i = 0; i < cities.size(); ++i) indices[i] = i;
        std::shuffle(indices.begin(), indices.end(), g);
        bestSolution = indices;

        updateChart(generation, bestFitness, avgFitness);
        drawSolution();
        QThread::msleep(100);
    }
}