#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QFile>
#include <QVector>
#include <QRandomGenerator>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

QT_BEGIN_NAMESPACE
namespace Ui { class Gui; }
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

struct City {
    double x, y;
    int priority;
};

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
    void updateChart(int generation, double bestFitness, double avgFitness);

private:
    Ui::Gui *ui;
    QGraphicsScene *scene;
    QGraphicsScene *compareScene;
    QVector<City> cities;

    // Для графика
    QChart *chart;
    QChartView *chartView; // Программно созданный chartView
    QLineSeries *bestFitnessSeries;
    QLineSeries *avgFitnessSeries;

    void drawSolution();
    void drawCompareSolution();
    void readCitiesFromInput();
    void updateButtonsState();
    void setupChart();
    void simulateAlgorithm();
};

#endif // GUI_H