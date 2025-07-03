#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QVector>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include "towndataparser.h"
#include "tsp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class Gui; }
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class Gui : public QMainWindow
{
Q_OBJECT

public:
    Gui(QWidget *parent = nullptr);
    ~Gui();

private slots:
    void on_loadFileButton_clicked();
    void on_generateRandomButton_clicked();
    void on_inputCitiesButton_clicked();
    void on_runStepButton_clicked();
    void on_backButton_clicked();
    void on_applyButton_clicked();
    void on_compareButton_clicked();
    void on_runToEndButton_clicked();
    void on_solutionComboBox_changed(int index);
    void updateChart(int generation, double bestFitness, double avgFitness);

private:
    Ui::Gui *ui;
    QGraphicsScene *scene;
    QGraphicsScene *compareScene;
    QVector<Town> towns;
    QVector<int> bestSolution;
    int currentGeneration;
    bool isAnimationRunning;
    bool shouldUpdateChart;
    TownDataParser parser;
    QVector<QPair<int, QPair<QVector<int>, QPair<double, double>>>> solutionHistory;

    QChart *chart;
    QChartView *chartView;
    QLineSeries *bestFitnessSeries;
    QLineSeries *avgFitnessSeries;

    void drawSolution();
    void drawCompareSolution(const QVector<int> &solution = QVector<int>());
    void drawBestSolutionPath(const QVector<int> &solution);
    void updateButtonsState();
    void setupChart();
};

#endif // GUI_H