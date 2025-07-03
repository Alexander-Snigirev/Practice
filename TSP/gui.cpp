#include "gui.h"
#include "ui_gui.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsEllipseItem>
#include <QGraphicsTextItem>
#include <QGraphicsLineItem>
#include <QRandomGenerator>
#include <QPen>
#include <QBrush>
#include <QInputDialog>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QApplication>
#include <QTimer>
#include <QDebug>
#include <cmath>
#include <random>
#include <algorithm>
#include <QDateTime>
#include "rapidcsv.h"
#include "tsp.h"

// Переменные для защиты от множественных вызовов
qint64 lastRunStepClickTime = 0;
qint64 lastBackButtonClickTime = 0;
qint64 lastGenerateRandomClickTime = 0;
qint64 lastCompareClickTime = 0;
qint64 lastRunToEndClickTime = 0;
qint64 lastLoadFileClickTime = 0;
const qint64 MIN_CLICK_INTERVAL = 200;

// Счётчики для диагностики
static int generateRandomCallCount = 0;
static int loadFileCallCount = 0;
static int runToEndCallCount = 0;

// Флаги для защиты от повторных вызовов
static bool isGenerateRandomRunning = false;
static bool isLoadFileRunning = false;

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

    setupChart();
    currentGeneration = 0;
    isAnimationRunning = false;
    shouldUpdateChart = false;

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
    controlLayout->addWidget(ui->applyButton);
    controlLayout->addWidget(ui->runStepButton);
    controlLayout->addWidget(ui->backButton);
    controlLayout->addWidget(ui->runToEndButton);
    controlLayout->addWidget(ui->solutionComboBox);
    controlLayout->addWidget(ui->compareButton);
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

    // Отключение автоматических подключений сигналов, созданных Qt Designer
    QMetaObject::connectSlotsByName(this);
    disconnect(ui->loadFileButton, &QPushButton::clicked, this, &Gui::on_loadFileButton_clicked);
    disconnect(ui->generateRandomButton, &QPushButton::clicked, this, &Gui::on_generateRandomButton_clicked);
    disconnect(ui->inputCitiesButton, &QPushButton::clicked, this, &Gui::on_inputCitiesButton_clicked);
    disconnect(ui->runStepButton, &QPushButton::clicked, this, &Gui::on_runStepButton_clicked);
    disconnect(ui->backButton, &QPushButton::clicked, this, &Gui::on_backButton_clicked);
    disconnect(ui->applyButton, &QPushButton::clicked, this, &Gui::on_applyButton_clicked);
    disconnect(ui->compareButton, &QPushButton::clicked, this, &Gui::on_compareButton_clicked);
    disconnect(ui->runToEndButton, &QPushButton::clicked, this, &Gui::on_runToEndButton_clicked);
    disconnect(ui->solutionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Gui::on_solutionComboBox_changed);

    // Явное подключение сигналов
    connect(ui->loadFileButton, &QPushButton::clicked, this, &Gui::on_loadFileButton_clicked);
    connect(ui->generateRandomButton, &QPushButton::clicked, this, &Gui::on_generateRandomButton_clicked);
    connect(ui->inputCitiesButton, &QPushButton::clicked, this, &Gui::on_inputCitiesButton_clicked);
    connect(ui->runStepButton, &QPushButton::clicked, this, &Gui::on_runStepButton_clicked);
    connect(ui->backButton, &QPushButton::clicked, this, &Gui::on_backButton_clicked);
    connect(ui->applyButton, &QPushButton::clicked, this, &Gui::on_applyButton_clicked);
    connect(ui->compareButton, &QPushButton::clicked, this, &Gui::on_compareButton_clicked);
    connect(ui->runToEndButton, &QPushButton::clicked, this, &Gui::on_runToEndButton_clicked);
    connect(ui->solutionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Gui::on_solutionComboBox_changed);

    static qint64 lastGenerateRandomSignalTime = 0;
    static qint64 lastLoadFileSignalTime = 0;
    connect(ui->generateRandomButton, &QPushButton::clicked, this, [=]() {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        qDebug() << "generateRandomButton: Signal emitted, time since last=" << (currentTime - lastGenerateRandomSignalTime) << "ms";
        lastGenerateRandomSignalTime = currentTime;
    });
    connect(ui->loadFileButton, &QPushButton::clicked, this, [=]() {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        qDebug() << "loadFileButton: Signal emitted, time since last=" << (currentTime - lastLoadFileSignalTime) << "ms";
        lastLoadFileSignalTime = currentTime;
    });

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
    axisY->setLabelFormat("%.2f");
    axisY->setRange(0, 1);
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
    qDebug() << "updateChart: generation=" << generation << ", displayed as Поколение" << generation + 1
             << ", bestFitness=" << bestFitness << ", avgFitness=" << avgFitness
             << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex()
             << ", shouldUpdateChart=" << shouldUpdateChart;
    if (shouldUpdateChart) {
        bestFitnessSeries->append(generation + 1, bestFitness);
        avgFitnessSeries->append(generation + 1, avgFitness);

        QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
        axisX->setRange(1, std::max(1, generation + 1));

        double maxFitness = std::max(bestFitness, avgFitness);
        maxFitness = std::max(maxFitness, 0.0001);
        QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
        axisY->setRange(0, maxFitness * 2.5);
    }
}

void Gui::updateButtonsState()
{
    qDebug() << "updateButtonsState: currentGeneration=" << currentGeneration
             << ", solutionHistory.size=" << solutionHistory.size()
             << ", isAnimationRunning=" << isAnimationRunning
             << ", towns.isEmpty=" << towns.isEmpty()
             << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex();
    ui->loadFileButton->setEnabled(true);
    ui->generateRandomButton->setEnabled(true);
    ui->inputCitiesButton->setEnabled(true);
    ui->runStepButton->setEnabled(!towns.isEmpty() && !solutionHistory.isEmpty() && !isAnimationRunning && currentGeneration < solutionHistory.size() - 1);
    ui->backButton->setEnabled(!towns.isEmpty() && !solutionHistory.isEmpty() && !isAnimationRunning && currentGeneration > 0);
    ui->applyButton->setEnabled(!towns.isEmpty() && !isAnimationRunning);
    ui->runToEndButton->setEnabled(!towns.isEmpty() && !solutionHistory.isEmpty() && !isAnimationRunning);
    ui->compareButton->setEnabled(!solutionHistory.isEmpty() && !isAnimationRunning);
}

void Gui::on_loadFileButton_clicked()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastLoadFileClickTime < MIN_CLICK_INTERVAL) {
        qDebug() << "loadFileButton: Ignored due to rapid successive calls, time since last click=" << (currentTime - lastLoadFileClickTime) << "ms";
        return;
    }
    if (isLoadFileRunning) {
        qDebug() << "loadFileButton: Ignored due to function already running";
        return;
    }
    isLoadFileRunning = true;
    lastLoadFileClickTime = currentTime;

    qDebug() << "loadFileButton: Clicked, call count=" << ++loadFileCallCount << ", blocking signals";
    this->blockSignals(true);
    ui->loadFileButton->blockSignals(true);
    ui->loadFileButton->setEnabled(false);

    QTimer::singleShot(0, this, [=]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Текстовые файлы (*.txt)", nullptr, QFileDialog::DontUseNativeDialog);
        this->blockSignals(false);
        ui->loadFileButton->blockSignals(false);
        ui->loadFileButton->setEnabled(true);
        isLoadFileRunning = false;

        if (fileName.isEmpty()) {
            qDebug() << "loadFileButton: File selection cancelled";
            updateButtonsState();
            return;
        }

        QStringList errorMessages;
        if (parser.parseFromFile(fileName, towns, errorMessages)) {
            bestSolution.clear();
            currentGeneration = 0;
            solutionHistory.clear();
            ui->solutionComboBox->clear();
            bestFitnessSeries->clear();
            avgFitnessSeries->clear();
            drawSolution();
            drawCompareSolution(QVector<int>());
            updateButtonsState();
        } else {
            QMessageBox::warning(this, "Ошибка", errorMessages.join("\n"));
            updateButtonsState();
        }
    });
}

void Gui::on_generateRandomButton_clicked()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastGenerateRandomClickTime < MIN_CLICK_INTERVAL) {
        qDebug() << "generateRandomButton: Ignored due to rapid successive calls, time since last click=" << (currentTime - lastGenerateRandomClickTime) << "ms";
        return;
    }
    if (isGenerateRandomRunning) {
        qDebug() << "generateRandomButton: Ignored due to function already running";
        return;
    }
    isGenerateRandomRunning = true;
    lastGenerateRandomClickTime = currentTime;

    qDebug() << "generateRandomButton: Clicked, call count=" << ++generateRandomCallCount << ", blocking signals";
    this->blockSignals(true);
    ui->generateRandomButton->blockSignals(true);
    ui->generateRandomButton->setEnabled(false);

    QTimer::singleShot(0, this, [=]() {
        QInputDialog dialog(this);
        dialog.setWindowTitle(tr("Генерация городов"));
        dialog.setLabelText(tr("Введите количество городов (5-100):"));
        dialog.setIntRange(5, 100);
        dialog.setIntValue(10);
        dialog.setOkButtonText(tr("OK"));
        dialog.setCancelButtonText(tr("Отмена"));
        bool ok = (dialog.exec() == QDialog::Accepted);
        int townCount = dialog.intValue();

        this->blockSignals(false);
        ui->generateRandomButton->blockSignals(false);
        ui->generateRandomButton->setEnabled(true);
        isGenerateRandomRunning = false;

        if (!ok) {
            qDebug() << "generateRandomButton: Dialog cancelled";
            updateButtonsState();
            return;
        }

        if (townCount < 5 || townCount > 100) {
            qDebug() << "generateRandomButton: Invalid town count=" << townCount;
            QMessageBox::warning(this, tr("Ошибка"), tr("Количество городов должно быть от 5 до 100"));
            updateButtonsState();
            return;
        }

        towns.clear();
        solutionHistory.clear();
        bestSolution.clear();
        ui->solutionComboBox->clear();
        bestFitnessSeries->clear();
        avgFitnessSeries->clear();
        currentGeneration = 0;

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(0, 1000);

        for (int i = 0; i < townCount; ++i) {
            double x = dis(gen);
            double y = dis(gen);
            towns.append(Town(x, y, 1, "City" + std::to_string(i)));
        }

        qDebug() << "generateRandomButton: Generated" << townCount << "towns";
        drawSolution();
        drawCompareSolution(QVector<int>());
        updateButtonsState();
    });
}

void Gui::on_inputCitiesButton_clicked()
{
    QStringList errorMessages;
    if (parser.parseFromText(ui->textEdit->toPlainText(), towns, errorMessages)) {
        bestSolution.clear();
        currentGeneration = 0;
        solutionHistory.clear();
        ui->solutionComboBox->clear();
        bestFitnessSeries->clear();
        avgFitnessSeries->clear();
        drawSolution();
        drawCompareSolution(QVector<int>());
        updateButtonsState();
    } else {
        QMessageBox::warning(this, "Ошибка", errorMessages.join("\n"));
    }
}

void Gui::on_applyButton_clicked()
{
    if (towns.isEmpty() || isAnimationRunning) {
        qDebug() << "applyButton: No towns loaded or animation running";
        QMessageBox::warning(this, "Ошибка", "Сначала введите города или дождитесь завершения анимации");
        return;
    }

    try {
        std::vector<Town> townsStd(towns.begin(), towns.end());
        int populationSize = ui->popSizeSpinBox->value();
        int generationsNumber = ui->maxGenSpinBox->value();
        double mutationProb = ui->mutationRateSpinBox->value();
        double crossProb = 0.8;
        std::string filename = "data.csv";

        qDebug() << "applyButton: Starting Evolution with towns.size=" << towns.size()
                 << ", populationSize=" << populationSize
                 << ", generationsNumber=" << generationsNumber
                 << ", mutationProb=" << mutationProb
                 << ", crossProb=" << crossProb;

        // Запуск алгоритма
        std::vector<double> best_fitnesses = Evolution(townsStd, populationSize, generationsNumber, mutationProb, crossProb, filename);
        qDebug() << "applyButton: Evolution returned best_fitnesses size=" << best_fitnesses.size();

        // Загрузка данных из CSV
        rapidcsv::Document doc(filename, rapidcsv::LabelParams(0, -1), rapidcsv::SeparatorParams(','));
        std::vector<std::vector<int>> best_individs(generationsNumber);
        std::vector<double> var_lens(generationsNumber);
        int bestGeneration = 0;
        double minBestFitness = std::numeric_limits<double>::max();
        std::vector<int> bestPath;

        for (int i = 0; i < generationsNumber; ++i) {
            try {
                std::vector<std::string> row = doc.GetRow<std::string>(i + 1);
                if (row.empty()) {
                    qDebug() << "applyButton: Warning: Empty row at generation" << i;
                    continue;
                }
                std::vector<int> path;
                for (size_t j = 1; j <= towns.size(); ++j) {
                    if (j < row.size()) {
                        int townIndex = std::stoi(row[j]);
                        if (townIndex >= 0 && townIndex < towns.size()) {
                            path.push_back(townIndex);
                        } else {
                            qDebug() << "applyButton: Invalid town index" << townIndex << "at generation" << i << ", column" << j;
                        }
                    } else {
                        qDebug() << "applyButton: Warning: Missing town data at generation" << i << ", column" << j;
                    }
                }
                best_individs[i] = path;
                var_lens[i] = row.size() > towns.size() + 1 ? std::stod(row[row.size() - 1]) : 0.0;
                double bestFitness = calculateFitness(path, townsStd);
                qDebug() << "applyButton: Loaded generation" << i << ", path size=" << path.size()
                         << ", path=" << QVector<int>(path.begin(), path.end())
                         << ", avgFitness=" << var_lens[i]
                         << ", bestFitness=" << bestFitness;

                if (bestFitness < minBestFitness && !path.empty() && path.size() == towns.size()) {
                    minBestFitness = bestFitness;
                    bestGeneration = i;
                    bestPath = path;
                }
            } catch (const std::exception &e) {
                qDebug() << "applyButton: Error loading generation" << i << ":" << e.what();
            }
        }

        // Обновление истории решений
        solutionHistory.clear();
        ui->solutionComboBox->clear();
        bestFitnessSeries->clear();
        avgFitnessSeries->clear();
        currentGeneration = 0;

        for (int i = 0; i < generationsNumber; ++i) {
            if (!best_individs[i].empty() && best_individs[i].size() == towns.size()) {
                double bestFitness = best_fitnesses[i]; // Используем best_fitnesses из Evolution (1e6 - way_length)
                solutionHistory.append(qMakePair(i, qMakePair(QVector<int>(best_individs[i].begin(), best_individs[i].end()), qMakePair(bestFitness, var_lens[i]))));
                ui->solutionComboBox->addItem(QString("Поколение %1").arg(i + 1));
                qDebug() << "applyButton: Added to solutionHistory, generation=" << i
                         << ", displayed as Поколение" << i + 1 << ", path size=" << best_individs[i].size()
                         << ", bestFitness=" << bestFitness;
            } else {
                qDebug() << "applyButton: Skipped generation" << i << "due to invalid path, size=" << best_individs[i].size();
            }
        }

        qDebug() << "applyButton: solutionHistory size=" << solutionHistory.size();
        if (!solutionHistory.isEmpty()) {
            bestSolution = solutionHistory[0].second.first;
            qDebug() << "applyButton: Set currentGeneration=0, bestSolution size=" << bestSolution.size();
            ui->solutionComboBox->blockSignals(true);
            ui->solutionComboBox->setCurrentIndex(0);
            ui->solutionComboBox->blockSignals(false);
            ui->generationLabel->setText(QString("Поколение: %1").arg(1));
            ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(best_fitnesses[0], 0, 'f', 2));
            ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(var_lens[0], 0, 'f', 2));
            drawSolution(); // Первое поколение в graphicsView
            drawCompareSolution(QVector<int>(bestPath.begin(), bestPath.end())); // Лучшее решение в compareGraphicsView
            qDebug() << "applyButton: Best solution at generation" << bestGeneration
                     << ", bestFitness=" << minBestFitness
                     << ", bestPath size=" << bestPath.size();
        } else {
            qDebug() << "applyButton: No valid solutions loaded from data.csv";
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить данные из алгоритма: пустая история решений");
        }
        updateButtonsState();
    } catch (const std::exception &e) {
        qDebug() << "applyButton: Error in algorithm:" << e.what();
        QMessageBox::critical(this, "Ошибка", QString("Ошибка в алгоритме: %1").arg(e.what()));
    }
}

void Gui::on_runStepButton_clicked()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastRunStepClickTime < MIN_CLICK_INTERVAL) {
        qDebug() << "runStepButton: Ignored due to rapid successive calls, time since last click=" << (currentTime - lastRunStepClickTime) << "ms";
        return;
    }
    lastRunStepClickTime = currentTime;

    if (towns.isEmpty() || solutionHistory.isEmpty() || isAnimationRunning) {
        qDebug() << "runStepButton: Aborted due to invalid state, towns.isEmpty=" << towns.isEmpty()
                 << ", solutionHistory.isEmpty=" << solutionHistory.isEmpty()
                 << ", isAnimationRunning=" << isAnimationRunning;
        return;
    }

    qDebug() << "runStepButton: Clicked, currentGeneration=" << currentGeneration
             << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex()
             << ", towns.size=" << towns.size();
    ui->runStepButton->setEnabled(false);
    currentGeneration++;
    if (currentGeneration < solutionHistory.size()) {
        if (!solutionHistory[currentGeneration].second.first.isEmpty() && solutionHistory[currentGeneration].second.first.size() == towns.size()) {
            bestSolution = solutionHistory[currentGeneration].second.first;
            double bestFitness = solutionHistory[currentGeneration].second.second.first;
            double avgFitness = solutionHistory[currentGeneration].second.second.second;
            qDebug() << "runStepButton: After increment, currentGeneration=" << currentGeneration
                     << ", displaying Поколение" << currentGeneration + 1
                     << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex()
                     << ", bestSolution size=" << bestSolution.size()
                     << ", bestFitness=" << bestFitness << ", avgFitness=" << avgFitness;
            ui->solutionComboBox->blockSignals(true);
            ui->solutionComboBox->setCurrentIndex(currentGeneration);
            ui->solutionComboBox->blockSignals(false);
            shouldUpdateChart = true;
            updateChart(currentGeneration, bestFitness, avgFitness);
            shouldUpdateChart = false;
            ui->generationLabel->setText(QString("Поколение: %1").arg(currentGeneration + 1));
            ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(bestFitness, 0, 'f', 2));
            ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(avgFitness, 0, 'f', 2));
            drawSolution();
        } else {
            qDebug() << "runStepButton: Invalid solution at currentGeneration=" << currentGeneration
                     << ", bestSolution size=" << solutionHistory[currentGeneration].second.first.size()
                     << ", expected size=" << towns.size();
            currentGeneration--;
        }
    } else {
        currentGeneration--;
        qDebug() << "runStepButton: Reverted, currentGeneration=" << currentGeneration;
    }
    updateButtonsState();
}

void Gui::on_backButton_clicked()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastBackButtonClickTime < MIN_CLICK_INTERVAL) {
        qDebug() << "backButton: Ignored due to rapid successive calls, time since last click=" << (currentTime - lastBackButtonClickTime) << "ms";
        return;
    }
    lastBackButtonClickTime = currentTime;

    if (towns.isEmpty() || solutionHistory.isEmpty() || isAnimationRunning) {
        qDebug() << "backButton: Aborted due to invalid state, towns.isEmpty=" << towns.isEmpty()
                 << ", solutionHistory.isEmpty=" << solutionHistory.isEmpty()
                 << ", isAnimationRunning=" << isAnimationRunning;
        return;
    }

    qDebug() << "backButton: Clicked, currentGeneration=" << currentGeneration
             << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex()
             << ", towns.size=" << towns.size();
    ui->backButton->setEnabled(false);
    currentGeneration--;
    if (currentGeneration >= 0) {
        if (!solutionHistory[currentGeneration].second.first.isEmpty() && solutionHistory[currentGeneration].second.first.size() == towns.size()) {
            bestSolution = solutionHistory[currentGeneration].second.first;
            double bestFitness = solutionHistory[currentGeneration].second.second.first;
            double avgFitness = solutionHistory[currentGeneration].second.second.second;
            qDebug() << "backButton: After decrement, currentGeneration=" << currentGeneration
                     << ", displaying Поколение" << currentGeneration + 1
                     << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex()
                     << ", bestSolution size=" << bestSolution.size()
                     << ", bestFitness=" << bestFitness << ", avgFitness=" << avgFitness;
            ui->solutionComboBox->blockSignals(true);
            ui->solutionComboBox->setCurrentIndex(currentGeneration);
            ui->solutionComboBox->blockSignals(false);
            shouldUpdateChart = true;
            updateChart(currentGeneration, bestFitness, avgFitness);
            shouldUpdateChart = false;
            ui->generationLabel->setText(QString("Поколение: %1").arg(currentGeneration + 1));
            ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(bestFitness, 0, 'f', 2));
            ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(avgFitness, 0, 'f', 2));
            drawSolution();
        } else {
            qDebug() << "backButton: Invalid solution at currentGeneration=" << currentGeneration
                     << ", bestSolution size=" << solutionHistory[currentGeneration].second.first.size()
                     << ", expected size=" << towns.size();
            currentGeneration++;
        }
    } else {
        currentGeneration++;
        qDebug() << "backButton: Reverted, currentGeneration=" << currentGeneration;
    }
    updateButtonsState();
}

void Gui::on_compareButton_clicked()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastCompareClickTime < MIN_CLICK_INTERVAL) {
        qDebug() << "compareButton: Ignored due to rapid successive calls, time since last click=" << (currentTime - lastCompareClickTime) << "ms";
        return;
    }
    lastCompareClickTime = currentTime;

    if (isAnimationRunning) {
        qDebug() << "compareButton: Aborted due to animation running";
        return;
    }

    int index = ui->solutionComboBox->currentIndex();
    if (index >= 0 && index < solutionHistory.size()) {
        qDebug() << "compareButton: Selected generation=" << index
                 << ", path size=" << solutionHistory[index].second.first.size();
        bestSolution = solutionHistory[index].second.first;
        double bestFitness = solutionHistory[index].second.second.first;
        double avgFitness = solutionHistory[index].second.second.second;
        ui->generationLabel->setText(QString("Поколение: %1").arg(index + 1));
        ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(bestFitness, 0, 'f', 2));
        ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(avgFitness, 0, 'f', 2));
        drawCompareSolution(solutionHistory[index].second.first);
    } else {
        qDebug() << "compareButton: Invalid generation index=" << index;
        QMessageBox::information(this, "Информация", "Выберите поколение для сравнения");
    }
    updateButtonsState();
}

void Gui::on_runToEndButton_clicked()
{
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastRunToEndClickTime < MIN_CLICK_INTERVAL) {
        qDebug() << "runToEndButton: Ignored due to rapid successive calls, time since last click=" << (currentTime - lastRunToEndClickTime) << "ms";
        return;
    }
    lastRunToEndClickTime = currentTime;

    qDebug() << "runToEndButton: Clicked, call count=" << ++runToEndCallCount << ", blocking signals";
    ui->runToEndButton->blockSignals(true);
    ui->runToEndButton->setEnabled(false);
    ui->runStepButton->blockSignals(true);
    ui->backButton->blockSignals(true);
    ui->applyButton->blockSignals(true);
    ui->compareButton->blockSignals(true);
    updateButtonsState();

    if (towns.isEmpty()) {
        qDebug() << "runToEndButton: No towns loaded";
        QMessageBox::warning(this, "Ошибка", "Сначала введите города");
        ui->runToEndButton->blockSignals(false);
        ui->runStepButton->blockSignals(false);
        ui->backButton->blockSignals(false);
        ui->applyButton->blockSignals(false);
        ui->compareButton->blockSignals(false);
        updateButtonsState();
        return;
    }
    if (isAnimationRunning) {
        qDebug() << "runToEndButton: Animation already running";
        QMessageBox::warning(this, "Ошибка", "Анимация уже выполняется");
        ui->runToEndButton->blockSignals(false);
        ui->runStepButton->blockSignals(false);
        ui->backButton->blockSignals(false);
        ui->applyButton->blockSignals(false);
        ui->compareButton->blockSignals(false);
        updateButtonsState();
        return;
    }
    if (solutionHistory.isEmpty()) {
        qDebug() << "runToEndButton: No solutions loaded";
        QMessageBox::warning(this, "Ошибка", "Сначала нажмите кнопку 'Применить'");
        ui->runToEndButton->blockSignals(false);
        ui->runStepButton->blockSignals(false);
        ui->backButton->blockSignals(false);
        ui->applyButton->blockSignals(false);
        ui->compareButton->blockSignals(false);
        updateButtonsState();
        return;
    }

    try {
        isAnimationRunning = true;
        updateButtonsState();

        std::vector<Town> townsStd(towns.begin(), towns.end());
        int populationSize = ui->popSizeSpinBox->value();
        int generationsNumber = ui->maxGenSpinBox->value();
        double mutationProb = ui->mutationRateSpinBox->value();
        double crossProb = 0.8;
        std::string filename = "data.csv";

        qDebug() << "runToEndButton: Starting Evolution with towns.size=" << towns.size()
                 << ", populationSize=" << populationSize
                 << ", generationsNumber=" << generationsNumber
                 << ", mutationProb=" << mutationProb
                 << ", crossProb=" << crossProb;

        // Очистка перед новым запуском
        solutionHistory.clear();
        ui->solutionComboBox->clear();
        bestFitnessSeries->clear();
        avgFitnessSeries->clear();
        currentGeneration = 0;
        bestSolution.clear();

        // Запуск алгоритма
        std::vector<double> best_fitnesses = Evolution(townsStd, populationSize, generationsNumber, mutationProb, crossProb, filename);
        qDebug() << "runToEndButton: Evolution returned best_fitnesses size=" << best_fitnesses.size();

        // Загрузка данных из CSV
        rapidcsv::Document doc(filename, rapidcsv::LabelParams(0, -1), rapidcsv::SeparatorParams(','));
        std::vector<std::vector<int>> best_individs(generationsNumber);
        std::vector<double> var_lens(generationsNumber);
        int bestGeneration = 0;
        double minBestFitness = std::numeric_limits<double>::max();
        std::vector<int> bestPath;

        for (int i = 0; i < generationsNumber; ++i) {
            try {
                std::vector<std::string> row = doc.GetRow<std::string>(i + 1);
                if (row.empty()) {
                    qDebug() << "runToEndButton: Warning: Empty row at generation" << i;
                    continue;
                }
                std::vector<int> path;
                for (size_t j = 1; j <= towns.size(); ++j) {
                    if (j < row.size()) {
                        int townIndex = std::stoi(row[j]);
                        if (townIndex >= 0 && townIndex < towns.size()) {
                            path.push_back(townIndex);
                        } else {
                            qDebug() << "runToEndButton: Invalid town index" << townIndex << "at generation" << i << ", column" << j;
                        }
                    } else {
                        qDebug() << "runToEndButton: Warning: Missing town data at generation" << i << ", column" << j;
                    }
                }
                best_individs[i] = path;
                var_lens[i] = row.size() > towns.size() + 1 ? std::stod(row[row.size() - 1]) : 0.0;
                double bestFitness = calculateFitness(path, townsStd);
                qDebug() << "runToEndButton: Loaded generation" << i << ", path size=" << path.size()
                         << ", path=" << QVector<int>(path.begin(), path.end())
                         << ", avgFitness=" << var_lens[i]
                         << ", bestFitness=" << bestFitness;

                if (bestFitness < minBestFitness && !path.empty() && path.size() == towns.size()) {
                    minBestFitness = bestFitness;
                    bestGeneration = i;
                    bestPath = path;
                }
            } catch (const std::exception &e) {
                qDebug() << "runToEndButton: Error loading generation" << i << ":" << e.what();
            }
        }

        // Анимация результатов
        QTimer *timer = new QTimer(this);
        int i = 0;
        connect(timer, &QTimer::timeout, this, [=]() mutable {
            if (i >= generationsNumber) {
                timer->stop();
                delete timer;
                currentGeneration = generationsNumber - 1;
                isAnimationRunning = false;
                ui->runToEndButton->blockSignals(false);
                ui->runStepButton->blockSignals(false);
                ui->backButton->blockSignals(false);
                ui->applyButton->blockSignals(false);
                ui->compareButton->blockSignals(false);
                updateButtonsState();
                qDebug() << "runToEndButton: Animation finished, currentGeneration=" << currentGeneration
                         << ", displayed as Поколение" << currentGeneration + 1;
                drawCompareSolution(QVector<int>(bestPath.begin(), bestPath.end())); // Лучшее решение в compareGraphicsView
                ui->generationLabel->setText(QString("Поколение: %1").arg(currentGeneration + 1));
                ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(best_fitnesses[currentGeneration], 0, 'f', 2));
                ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(var_lens[currentGeneration], 0, 'f', 2));
                return;
            }

            if (!best_individs[i].empty() && best_individs[i].size() == towns.size()) {
                double bestFitness = best_fitnesses[i]; // Используем best_fitnesses из Evolution (1e6 - way_length)
                solutionHistory.append(qMakePair(i, qMakePair(QVector<int>(best_individs[i].begin(), best_individs[i].end()), qMakePair(bestFitness, var_lens[i]))));
                ui->solutionComboBox->addItem(QString("Поколение %1").arg(i + 1));
                qDebug() << "runToEndButton: Added generation=" << i << ", displayed as Поколение" << i + 1
                         << ", path size=" << best_individs[i].size()
                         << ", bestFitness=" << bestFitness;

                currentGeneration = i;
                bestSolution = QVector<int>(best_individs[i].begin(), best_individs[i].end());
                ui->solutionComboBox->blockSignals(true);
                ui->solutionComboBox->setCurrentIndex(currentGeneration);
                ui->solutionComboBox->blockSignals(false);
                shouldUpdateChart = true;
                updateChart(i, bestFitness, var_lens[i]);
                shouldUpdateChart = false;
                ui->generationLabel->setText(QString("Поколение: %1").arg(i + 1));
                ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(bestFitness, 0, 'f', 2));
                ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(var_lens[i], 0, 'f', 2));
                drawSolution();
            } else {
                qDebug() << "runToEndButton: Skipped generation" << i << "due to invalid path, size=" << best_individs[i].size();
            }
            i++;
        });
        timer->start(500);
    } catch (const std::exception &e) {
        isAnimationRunning = false;
        ui->runToEndButton->blockSignals(false);
        ui->runStepButton->blockSignals(false);
        ui->backButton->blockSignals(false);
        ui->applyButton->blockSignals(false);
        ui->compareButton->blockSignals(false);
        updateButtonsState();
        qDebug() << "runToEndButton: Error in algorithm:" << e.what();
        QMessageBox::critical(this, "Ошибка", QString("Ошибка в алгоритме: %1").arg(e.what()));
    }
}

void Gui::on_solutionComboBox_changed(int index)
{
    if (index < 0 || index >= solutionHistory.size() || isAnimationRunning) {
        qDebug() << "solutionComboBox: Invalid or blocked change, index=" << index
                 << ", solutionHistory.size=" << solutionHistory.size()
                 << ", isAnimationRunning=" << isAnimationRunning
                 << ", shouldUpdateChart=" << shouldUpdateChart;
        return;
    }

    qDebug() << "solutionComboBox: Selected index=" << index << ", setting currentGeneration=" << index
             << ", solutionHistory size=" << solutionHistory.size();
    currentGeneration = index;
    if (!solutionHistory[index].second.first.isEmpty()) {
        bestSolution = solutionHistory[index].second.first;
        double bestFitness = solutionHistory[index].second.second.first;
        double avgFitness = solutionHistory[index].second.second.second;
        qDebug() << "solutionComboBox: Updating to generation=" << index
                 << ", displayed as Поколение" << index + 1
                 << ", bestSolution size=" << bestSolution.size()
                 << ", bestFitness=" << bestFitness
                 << ", avgFitness=" << avgFitness;
        ui->generationLabel->setText(QString("Поколение: %1").arg(index + 1));
        ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(bestFitness, 0, 'f', 2));
        ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(avgFitness, 0, 'f', 2));
        drawSolution();
    } else {
        qDebug() << "solutionComboBox: Empty solution at index=" << index;
    }
    updateButtonsState();
}

void Gui::drawSolution()
{
    scene->clear();
    if (towns.isEmpty()) {
        qDebug() << "drawSolution: No towns to draw";
        return;
    }

    double minX = towns[0].x, maxX = towns[0].x;
    double minY = towns[0].y, maxY = towns[0].y;
    for (const auto &p : towns) {
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

    QPen pointPen(Qt::red);
    QBrush pointBrush(Qt::red);
    for (int i = 0; i < towns.size(); ++i) {
        const auto &p = towns[i];
        double x = (p.x - minX) * scaleX;
        double y = (p.y - minY) * scaleY;
        scene->addEllipse(x - 5, y - 5, 10, 10, pointPen, pointBrush);
        QGraphicsTextItem *label = scene->addText(QString::fromStdString(p.name) + QString(" (П: %1)").arg(p.priority));
        label->setPos(x + 5, y + 5);
    }

    if (!bestSolution.isEmpty()) {
        QPen linePen(Qt::blue);
        linePen.setWidth(2);
        for (int i = 0; i < bestSolution.size() - 1; ++i) {
            int town1 = bestSolution[i];
            int town2 = bestSolution[i + 1];
            if (town1 >= 0 && town1 < towns.size() && town2 >= 0 && town2 < towns.size()) {
                double x1 = (towns[town1].x - minX) * scaleX;
                double y1 = (towns[town1].y - minY) * scaleY;
                double x2 = (towns[town2].x - minX) * scaleX;
                double y2 = (towns[town2].y - minY) * scaleY;
                scene->addLine(x1, y1, x2, y2, linePen);
            }
        }
        if (bestSolution.size() >= 2) {
            int firstTown = bestSolution[0];
            int lastTown = bestSolution[bestSolution.size() - 1];
            if (firstTown >= 0 && firstTown < towns.size() && lastTown >= 0 && lastTown < towns.size()) {
                double x1 = (towns[lastTown].x - minX) * scaleX;
                double y1 = (towns[lastTown].y - minY) * scaleY;
                double x2 = (towns[firstTown].x - minX) * scaleX;
                double y2 = (towns[firstTown].y - minY) * scaleY;
                scene->addLine(x1, y1, x2, y2, linePen);
            }
        }
    }
}

void Gui::drawCompareSolution(const QVector<int> &solution)
{
    compareScene->clear();
    if (towns.isEmpty()) {
        qDebug() << "drawCompareSolution: No towns to draw";
        return;
    }

    double minX = towns[0].x, maxX = towns[0].x;
    double minY = towns[0].y, maxY = towns[0].y;
    for (const auto &p : towns) {
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
    for (int i = 0; i < towns.size(); ++i) {
        const auto &p = towns[i];
        double x = (p.x - minX) * scaleX;
        double y = (p.y - minY) * scaleY;
        compareScene->addEllipse(x - 5, y - 5, 10, 10, pointPen, pointBrush);
        QGraphicsTextItem *label = compareScene->addText(QString::fromStdString(p.name) + QString(" (П: %1)").arg(p.priority));
        label->setPos(x + 5, y + 5);
    }

    if (!solution.isEmpty()) {
        QPen linePen(Qt::blue);
        linePen.setWidth(2);
        for (int i = 0; i < solution.size() - 1; ++i) {
            int town1 = solution[i];
            int town2 = solution[i + 1];
            if (town1 >= 0 && town1 < towns.size() && town2 >= 0 && town2 < towns.size()) {
                double x1 = (towns[town1].x - minX) * scaleX;
                double y1 = (towns[town1].y - minY) * scaleY;
                double x2 = (towns[town2].x - minX) * scaleX;
                double y2 = (towns[town2].y - minY) * scaleY;
                compareScene->addLine(x1, y1, x2, y2, linePen);
            }
        }
        if (solution.size() >= 2) {
            int firstTown = solution[0];
            int lastTown = solution[solution.size() - 1];
            if (firstTown >= 0 && firstTown < towns.size() && lastTown >= 0 && lastTown < towns.size()) {
                double x1 = (towns[lastTown].x - minX) * scaleX;
                double y1 = (towns[lastTown].y - minY) * scaleY;
                double x2 = (towns[firstTown].x - minX) * scaleX;
                double y2 = (towns[firstTown].y - minY) * scaleY;
                compareScene->addLine(x1, y1, x2, y2, linePen);
            }
        }
    }
}

void Gui::drawBestSolutionPath(const QVector<int> &solution)
{
    drawCompareSolution(solution);
}