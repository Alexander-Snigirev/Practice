#ifndef GUI_H
#define GUI_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QFile>
#include <QVector>
#include <QRandomGenerator>

QT_BEGIN_NAMESPACE
namespace Ui { class Gui; }
QT_END_NAMESPACE

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

private:
    Ui::Gui *ui;
    QGraphicsScene *scene;
    QGraphicsScene *compareScene;
    QVector<City> cities;

    void drawSolution();
    void drawCompareSolution();
    void readCitiesFromInput();
    void updateButtonsState();
};

#endif // GUI_H