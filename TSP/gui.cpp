#include "gui.h"
#include "ui_gui.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QPen>
#include <QBrush>
#include <QInputDialog>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QThread>
#include <cmath>
#include <random>
#include <algorithm>
#include <QHBoxLayout>
#include <QVBoxLayout>

Gui::Gui(QWidget *parent)
        : QMainWindow(parent), ui(new Ui::Gui)
{
    ui->setupUi(this);
    scene = new QGraphicsScene(this);
    compareScene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->compareGraphicsView->setScene(compareScene);

    const int viewSize = 500;
    ui->graphicsView->setFixedSize(viewSize, viewSize);
    ui->compareGraphicsView->setFixedSize(viewSize, viewSize);

    ui->graphicsView->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);
    ui->compareGraphicsView->setOptimizationFlag(QGraphicsView::DontSavePainterState, true);

    // Инициализация графика
    setupChart();
    currentGeneration = 0;

    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    QWidget *controlWidget = new QWidget;
    QVBoxLayout *controlLayout = new QVBoxLayout(controlWidget);
    controlLayout->addWidget(ui->textEdit);
    controlLayout->addWidget(ui->loadFileButton);
    controlLayout->addWidget(ui->generateRandomButton);
    controlLayout->addWidget(ui->inputCitiesButton);
    controlLayout->addWidget(ui->popSizeSpinBox);
    controlLayout->addWidget(ui->mutationRateSpinBox);
    controlLayout->addWidget(ui->maxGenSpinBox);
    controlLayout->addWidget(ui->runStepButton);
    controlLayout->addWidget(ui->runToEndButton);
    controlLayout->addWidget(ui->solutionComboBox);
    controlLayout->addWidget(ui->generationLabel);
    controlLayout->addWidget(ui->bestFitnessLabel);
    controlLayout->addWidget(ui->avgFitnessLabel);
    controlLayout->addStretch();
    mainLayout->addWidget(controlWidget, 1);

    QWidget *outputWidget = new QWidget;
    QVBoxLayout *rightLayout = new QVBoxLayout(outputWidget);
    QHBoxLayout *viewLayout = new QHBoxLayout;
    viewLayout->addWidget(ui->graphicsView);
    viewLayout->addWidget(ui->compareGraphicsView);
    rightLayout->addLayout(viewLayout);
    rightLayout->addWidget(chartView);
    rightLayout->addStretch();
    mainLayout->addWidget(outputWidget, 2);

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

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("Поколение");
    axisX->setLabelFormat("%.0f");
    axisX->setTickInterval(1.0);
    axisX->setMinorTickCount(0);
    axisX->setRange(1, 10);
    chart->addAxis(axisX, Qt::AlignBottom);
    bestFitnessSeries->attachAxis(axisX);
    avgFitnessSeries->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Приспособленность");
    axisY->setLabelFormat("%.0f");
    axisY->setMinorTickCount(0);
    axisY->setRange(0, 10);
    chart->addAxis(axisY, Qt::AlignLeft);
    bestFitnessSeries->attachAxis(axisY);
    avgFitnessSeries->attachAxis(axisY);

    chart->setTitle("Динамика приспособленности по поколениям");
    chart->legend()->setVisible(true);
    chart->setTitleFont(QFont("Arial", 12));
    axisX->setTitleFont(QFont("Arial", 10));
    axisY->setTitleFont(QFont("Arial", 10));
    chart->legend()->setFont(QFont("Arial", 10));

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setMinimumSize(800, 400);
    chartView->setMaximumHeight(400);
}

void Gui::updateChart(int generation, double bestFitness, double avgFitness)
{
    bestFitnessSeries->append(generation, bestFitness);
    avgFitnessSeries->append(generation, avgFitness);

    QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    axisX->setRange(1, std::max(1, generation));

    double maxFitness = std::max(bestFitness, avgFitness);
    QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    axisY->setRange(0, maxFitness * 1.1);

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

    QStringList errorMessages;
    if (parser.parseFromFile(fileName, cities, errorMessages)) {
        bestSolution.clear();
        currentGeneration = 0;
        bestFitnessSeries->clear();
        avgFitnessSeries->clear();
        drawSolution();
        drawCompareSolution();
        updateButtonsState();
    } else {
        QMessageBox::warning(this, "Ошибка", errorMessages.join("\n"));
    }
}

void Gui::on_generateRandomButton_clicked()
{
    bool ok;
    int numCities = QInputDialog::getInt(this, "Количество городов", "Введите количество городов:", 10, 3, 1000, 1, &ok);
    if (!ok) return;

    cities.clear();
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
    QStringList errorMessages;
    if (parser.parseFromText(ui->textEdit->toPlainText(), cities, errorMessages)) {
        bestSolution.clear();
        currentGeneration = 0;
        bestFitnessSeries->clear();
        avgFitnessSeries->clear();
        drawSolution();
        drawCompareSolution();
        updateButtonsState();
    } else {
        QMessageBox::warning(this, "Ошибка", errorMessages.join("\n"));
    }
}

void Gui::on_runStepButton_clicked()
{
    if (cities.isEmpty()) return;

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

    double viewSize = std::min(ui->graphicsView->width(), ui->graphicsView->height());
    double scaleX = viewSize / (maxX - minX);
    double scaleY = viewSize / (maxY - minY);

    // Отрисовка городов
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

    // Отрисовка пути лучшего решения
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

    double viewSize = std::min(ui->graphicsView->width(), ui->graphicsView->height());
    double scaleX = viewSize / (maxX - minX);
    double scaleY = viewSize / (maxY - minY);

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

    double viewSize = std::min(ui->compareGraphicsView->width(), ui->compareGraphicsView->height());
    double scaleX = viewSize / (maxX - minX);
    double scaleY = viewSize / (maxY - minY);

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

        // Генерация случайного решения для демонстрации
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