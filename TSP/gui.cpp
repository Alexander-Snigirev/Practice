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
qint64 lastRunStepClickTime = 0;
qint64 lastBackButtonClickTime = 0;
qint64 lastGenerateRandomClickTime = 0;
const qint64 MIN_CLICK_INTERVAL = 200;
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

    // Настройка layout
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

    // Подключение сигналов
    connect(ui->loadFileButton, &QPushButton::clicked, this, &Gui::on_loadFileButton_clicked);
    connect(ui->generateRandomButton, &QPushButton::clicked, this, &Gui::on_generateRandomButton_clicked);
    connect(ui->inputCitiesButton, &QPushButton::clicked, this, &Gui::on_inputCitiesButton_clicked);
    connect(ui->runStepButton, &QPushButton::clicked, this, &Gui::on_runStepButton_clicked);
    connect(ui->backButton, &QPushButton::clicked, this, &Gui::on_backButton_clicked);
    connect(ui->applyButton, &QPushButton::clicked, this, &Gui::on_applyButton_clicked);
    connect(ui->compareButton, &QPushButton::clicked, this, &Gui::on_compareButton_clicked);
    connect(ui->runToEndButton, &QPushButton::clicked, this, &Gui::on_runToEndButton_clicked);
    connect(ui->solutionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Gui::on_solutionComboBox_changed);

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
    axisX->setRange(1, 10); // Нумерация с 1
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
             << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex();
    bestFitnessSeries->append(generation + 1, bestFitness); // Нумерация с 1
    avgFitnessSeries->append(generation + 1, avgFitness);

    QValueAxis *axisX = qobject_cast<QValueAxis*>(chart->axes(Qt::Horizontal).first());
    axisX->setRange(1, std::max(1, generation + 1));

    // Динамическое масштабирование оси Y
    double maxFitness = std::max(bestFitness, avgFitness);
    maxFitness = std::max(maxFitness, 0.0001);
    QValueAxis *axisY = qobject_cast<QValueAxis*>(chart->axes(Qt::Vertical).first());
    axisY->setRange(0, maxFitness * 2.5);

    ui->generationLabel->setText(QString("Поколение: %1").arg(generation + 1));
    ui->bestFitnessLabel->setText(QString("Лучшая приспособленность: %1").arg(bestFitness, 0, 'f', 2));
    ui->avgFitnessLabel->setText(QString("Средняя приспособленность: %1").arg(avgFitness, 0, 'f', 2));
    ui->solutionComboBox->blockSignals(true); // Предотвращаем рекурсию
    ui->solutionComboBox->setCurrentIndex(generation);
    ui->solutionComboBox->blockSignals(false);

    QApplication::processEvents();
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
    QString fileName = QFileDialog::getOpenFileName(this, "Открыть файл", "", "Текстовые файлы (*.txt)", nullptr, QFileDialog::DontUseNativeDialog);
    if (fileName.isEmpty()) return;

    QStringList errorMessages;
    if (parser.parseFromFile(fileName, towns, errorMessages)) {
        bestSolution.clear();
        currentGeneration = 0;
        solutionHistory.clear();
        ui->solutionComboBox->clear();
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
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastGenerateRandomClickTime < MIN_CLICK_INTERVAL) {
        qDebug() << "generateRandomButton: Ignored due to rapid successive calls, time since last click=" << (currentTime - lastGenerateRandomClickTime) << "ms";
        return;
    }
    lastGenerateRandomClickTime = currentTime;

    qDebug() << "generateRandomButton: Clicked";
    bool ok;
    int townCount = QInputDialog::getInt(this, tr("Генерация городов"),
                                         tr("Введите количество городов (5-100):"),
                                         10, 5, 100, 1, &ok);
    if (!ok) {
        qDebug() << "generateRandomButton: Dialog cancelled";
        return;
    }

    if (townCount < 5 || townCount > 100) {
        qDebug() << "generateRandomButton: Invalid town count=" << townCount;
        QMessageBox::warning(this, tr("Ошибка"), tr("Количество городов должно быть от 5 до 100"));
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

    QString inputText;
    for (int i = 0; i < townCount; ++i) {
        double x = dis(gen);
        double y = dis(gen);
        towns.append(Town(x, y, 1));
        inputText += QString("%1,%2,1\n").arg(x, 0, 'f', 2).arg(y, 0, 'f', 2);
    }

    ui->textEdit->setText(inputText);
    qDebug() << "generateRandomButton: Generated" << townCount << "towns, inputText size=" << inputText.size();
    drawSolution();
    updateButtonsState();
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
        drawCompareSolution();
        updateButtonsState();
    } else {
        QMessageBox::warning(this, "Ошибка", errorMessages.join("\n"));
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
    ui->runStepButton->setEnabled(false); // Защита от повторных нажатий
    currentGeneration++;
    if (currentGeneration < solutionHistory.size()) {
        if (!solutionHistory[currentGeneration].second.first.isEmpty() && solutionHistory[currentGeneration].second.first.size() == towns.size()) {
            bestSolution = solutionHistory[currentGeneration].second.first;
            double bestFitness = solutionHistory[currentGeneration].second.second.first;
            double avgFitness = solutionHistory[currentGeneration].second.second.second;
            qDebug() << "runStepButton: After increment, currentGeneration=" << currentGeneration
                     << ", displaying Поколение" << currentGeneration
                     << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex()
                     << ", bestSolution size=" << bestSolution.size()
                     << ", bestFitness=" << bestFitness << ", avgFitness=" << avgFitness;
            ui->solutionComboBox->blockSignals(true);
            ui->solutionComboBox->setCurrentIndex(currentGeneration);
            ui->solutionComboBox->blockSignals(false);
            updateChart(currentGeneration, bestFitness, avgFitness);
            drawSolution();
        } else {
            qDebug() << "runStepButton: Invalid solution at currentGeneration=" << currentGeneration
                     << ", bestSolution size=" << solutionHistory[currentGeneration].second.first.size()
                     << ", expected size=" << towns.size();
            currentGeneration--; // Откат
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
    ui->backButton->setEnabled(false); // Защита от повторных нажатий
    currentGeneration--;
    if (currentGeneration >= 0) {
        if (!solutionHistory[currentGeneration].second.first.isEmpty() && solutionHistory[currentGeneration].second.first.size() == towns.size()) {
            bestSolution = solutionHistory[currentGeneration].second.first;
            double bestFitness = solutionHistory[currentGeneration].second.second.first;
            double avgFitness = solutionHistory[currentGeneration].second.second.second;
            qDebug() << "backButton: After decrement, currentGeneration=" << currentGeneration
                     << ", displaying Поколение" << currentGeneration
                     << ", solutionComboBox index=" << ui->solutionComboBox->currentIndex()
                     << ", bestSolution size=" << bestSolution.size()
                     << ", bestFitness=" << bestFitness << ", avgFitness=" << avgFitness;
            ui->solutionComboBox->blockSignals(true);
            ui->solutionComboBox->setCurrentIndex(currentGeneration);
            ui->solutionComboBox->blockSignals(false);
            updateChart(currentGeneration, bestFitness, avgFitness);
            drawSolution();
        } else {
            qDebug() << "backButton: Invalid solution at currentGeneration=" << currentGeneration
                     << ", bestSolution size=" << solutionHistory[currentGeneration].second.first.size()
                     << ", expected size=" << towns.size();
            currentGeneration++; // Откат
        }
    } else {
        currentGeneration++;
        qDebug() << "backButton: Reverted, currentGeneration=" << currentGeneration;
    }
    updateButtonsState();
}
void Gui::on_applyButton_clicked()
{
    if (towns.isEmpty() || isAnimationRunning) {
        QMessageBox::warning(this, "Ошибка", "Сначала введите города или дождитесь завершения анимации");
        return;
    }

    try {
        std::vector<Town> townsStd;
        for (const auto &t : towns) {
            townsStd.push_back(t);
        }

        int populationSize = ui->popSizeSpinBox->value();
        int generationsNumber = ui->maxGenSpinBox->value();
        double mutationProb = ui->mutationRateSpinBox->value();
        double crossProb = 0.8;
        std::string filename = "data.csv";

        // Запуск алгоритма
        std::vector<double> best_fitnesses = Evolution(townsStd, populationSize, generationsNumber, mutationProb, crossProb, filename);

        // Загрузка данных из CSV с проверкой
        rapidcsv::Document doc(filename, rapidcsv::LabelParams(0, -1), rapidcsv::SeparatorParams(','));
        std::vector<std::vector<int>> best_individs(generationsNumber);
        std::vector<double> var_lens(generationsNumber);
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
                qDebug() << "applyButton: Loaded generation" << i << ", path size=" << path.size()
                         << ", avgFitness=" << var_lens[i] << ", bestFitness=" << (i < best_fitnesses.size() ? best_fitnesses[i] : 0.0);
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
                solutionHistory.append(qMakePair(i, qMakePair(QVector<int>(best_individs[i].begin(), best_individs[i].end()), qMakePair(best_fitnesses[i], var_lens[i]))));
                ui->solutionComboBox->addItem(QString("Поколение %1").arg(i + 1));
                qDebug() << "applyButton: Added to solutionHistory, generation=" << i
                         << ", displayed as Поколение" << i + 1 << ", path size=" << best_individs[i].size();
            } else {
                qDebug() << "applyButton: Skipped generation" << i << "due to invalid path, size=" << best_individs[i].size();
            }
        }

        qDebug() << "applyButton: solutionHistory size=" << solutionHistory.size();
        if (!solutionHistory.isEmpty()) {
            bestSolution = solutionHistory[0].second.first;
            ui->solutionComboBox->blockSignals(true);
            ui->solutionComboBox->setCurrentIndex(0);
            ui->solutionComboBox->blockSignals(false);
            updateChart(0, best_fitnesses[0], var_lens[0]);
            drawSolution();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить данные из алгоритма: пустая история решений");
        }
        updateButtonsState();
    } catch (const std::exception &e) {
        QMessageBox::critical(this, "Ошибка", QString("Ошибка в алгоритме: %1").arg(e.what()));
    }
}
void Gui::on_solutionComboBox_changed(int index)
{
    if (index < 0 || index >= solutionHistory.size() || isAnimationRunning) return;

    qDebug() << "solutionComboBox: Selected index=" << index << ", setting currentGeneration=" << index
             << ", solutionHistory size=" << solutionHistory.size();
    currentGeneration = index;
    if (!solutionHistory[index].second.first.isEmpty()) {
        bestSolution = solutionHistory[index].second.first;
        double bestFitness = solutionHistory[index].second.second.first;
        double avgFitness = solutionHistory[index].second.second.second;
        qDebug() << "solutionComboBox: Updating to generation=" << index
                 << ", displayed as Поколение" << index + 1 << ", bestSolution size=" << bestSolution.size();
        updateChart(index, bestFitness, avgFitness);
        drawSolution();
    } else {
        qDebug() << "solutionComboBox: Empty solution at index=" << index;
    }
    updateButtonsState();
}

void Gui::on_compareButton_clicked()
{
    if (isAnimationRunning) return;

    int index = ui->solutionComboBox->currentIndex();
    if (index >= 0 && index < solutionHistory.size()) {
        drawCompareSolution(solutionHistory[index].second.first);
    } else {
        QMessageBox::information(this, "Информация", "Выберите поколение для сравнения");
    }
}

void Gui::on_runToEndButton_clicked()
{
    if (towns.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала введите города");
        return;
    }
    if (isAnimationRunning) {
        QMessageBox::warning(this, "Ошибка", "Анимация уже выполняется");
        return;
    }
    if (solutionHistory.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Сначала нажмите кнопку 'Применить'");
        return;
    }

    try {
        isAnimationRunning = true;
        updateButtonsState();

        std::vector<Town> townsStd;
        for (const auto &t : towns) {
            townsStd.push_back(t);
        }

        int populationSize = ui->popSizeSpinBox->value();
        int generationsNumber = ui->maxGenSpinBox->value();
        double mutationProb = ui->mutationRateSpinBox->value();
        double crossProb = 0.8;
        std::string filename = "data.csv";

        // Очистка перед новым запуском
        solutionHistory.clear();
        ui->solutionComboBox->clear();
        bestFitnessSeries->clear();
        avgFitnessSeries->clear();
        currentGeneration = 0;
        bestSolution.clear();

        // Запуск алгоритма один раз
        std::vector<double> best_fitnesses = Evolution(townsStd, populationSize, generationsNumber, mutationProb, crossProb, filename);

        // Загрузка данных из CSV с проверкой
        rapidcsv::Document doc(filename, rapidcsv::LabelParams(0, -1), rapidcsv::SeparatorParams(','));
        std::vector<std::vector<int>> best_individs(generationsNumber);
        std::vector<double> var_lens(generationsNumber);
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
                qDebug() << "runToEndButton: Loaded generation" << i << ", path size=" << path.size()
                         << ", avgFitness=" << var_lens[i] << ", bestFitness=" << (i < best_fitnesses.size() ? best_fitnesses[i] : 0.0);
            } catch (const std::exception &e) {
                qDebug() << "runToEndButton: Error loading generation" << i << ":" << e.what();
            }
        }

        // Анимация результатов с помощью QTimer
        QTimer *timer = new QTimer(this);
        int i = 0;
        connect(timer, &QTimer::timeout, this, [=]() mutable {
            if (i >= generationsNumber) {
                timer->stop();
                delete timer;
                currentGeneration = generationsNumber - 1;
                isAnimationRunning = false;
                updateButtonsState();
                qDebug() << "runToEndButton: Animation finished, currentGeneration=" << currentGeneration
                         << ", displayed as Поколение" << currentGeneration + 1;
                return;
            }

            if (!best_individs[i].empty() && best_individs[i].size() == towns.size()) {
                solutionHistory.append(qMakePair(i, qMakePair(QVector<int>(best_individs[i].begin(), best_individs[i].end()), qMakePair(best_fitnesses[i], var_lens[i]))));
                ui->solutionComboBox->addItem(QString("Поколение %1").arg(i + 1));
                qDebug() << "runToEndButton: Added generation=" << i << ", displayed as Поколение" << i + 1
                         << ", path size=" << best_individs[i].size();

                currentGeneration = i;
                bestSolution = QVector<int>(best_individs[i].begin(), best_individs[i].end());
                ui->solutionComboBox->blockSignals(true);
                ui->solutionComboBox->setCurrentIndex(currentGeneration);
                ui->solutionComboBox->blockSignals(false);
                updateChart(i, best_fitnesses[i], var_lens[i]);
                drawSolution();
            } else {
                qDebug() << "runToEndButton: Skipped generation" << i << "due to invalid path, size=" << best_individs[i].size();
            }
            i++;
        });
        timer->start(500);
    } catch (const std::exception &e) {
        isAnimationRunning = false;
        updateButtonsState();
        QMessageBox::critical(this, "Ошибка", QString("Ошибка в алгоритме: %1").arg(e.what()));
    }
}

void Gui::drawSolution()
{
    scene->clear();
    if (towns.isEmpty()) return;

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

    drawBestSolutionPath(bestSolution);
}

void Gui::drawBestSolutionPath(const QVector<int> &solution)
{
    if (solution.isEmpty()) return;

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

    QPen linePen(Qt::blue);
    linePen.setWidth(2);
    for (int i = 0; i < solution.size() - 1; ++i) {
        int town1 = solution[i];
        int town2 = solution[i + 1];
        double x1 = (towns[town1].x - minX) * scaleX;
        double y1 = (towns[town1].y - minY) * scaleY;
        double x2 = (towns[town2].x - minX) * scaleX;
        double y2 = (towns[town2].y - minY) * scaleY;
        scene->addLine(x1, y1, x2, y2, linePen);
    }
    if (!solution.isEmpty()) {
        int firstTown = solution[0];
        int lastTown = solution[solution.size() - 1];
        double x1 = (towns[lastTown].x - minX) * scaleX;
        double y1 = (towns[lastTown].y - minY) * scaleY;
        double x2 = (towns[firstTown].x - minX) * scaleX;
        double y2 = (towns[firstTown].y - minY) * scaleY;
        scene->addLine(x1, y1, x2, y2, linePen);
    }
}

void Gui::drawCompareSolution(const QVector<int> &solution)
{
    compareScene->clear();
    if (towns.isEmpty()) return;

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
            double x1 = (towns[town1].x - minX) * scaleX;
            double y1 = (towns[town1].y - minY) * scaleY;
            double x2 = (towns[town2].x - minX) * scaleX;
            double y2 = (towns[town2].y - minY) * scaleY;
            compareScene->addLine(x1, y1, x2, y2, linePen);
        }
        int firstTown = solution[0];
        int lastTown = solution[solution.size() - 1];
        double x1 = (towns[lastTown].x - minX) * scaleX;
        double y1 = (towns[lastTown].y - minY) * scaleY;
        double x2 = (towns[firstTown].x - minX) * scaleX;
        double y2 = (towns[firstTown].y - minY) * scaleY;
        compareScene->addLine(x1, y1, x2, y2, linePen);
    }
}