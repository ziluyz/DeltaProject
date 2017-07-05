#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include "qcustomplot.h"
#include "delpro.h"
#include <memory>
#include <set>

using namespace std;

struct Data;
struct ScreenOutput;
struct OutputItem;
class Wgt;

class MainWindow : public QWidget
{
    Q_OBJECT
private:
    Data *data;
    set<shared_ptr<Wgt>> toDraw;
public:
    explicit MainWindow(Data *data);
    bool needUpdate;
    vector<shared_ptr<Wgt>> content;
protected:
    void timerEvent(QTimerEvent *event) override;

signals:

public slots:
};

class Wgt {
protected:
    ScreenOutput *source;
public:
    Wgt(ScreenOutput *sout) : source(sout) {}
    virtual void draw()=0;
    virtual void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) = 0;
};

class TextField : public Wgt {
private:
    QLabel *label;
public:
    TextField(ScreenOutput *sout);
    void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
};

class Plot : public Wgt {
private:
    struct Graph {
        struct gpair {
            OutputItem *y;
            QCPGraph *graph;
        };

        QString xtag;
        OutputItem *x;
        vector<gpair> ys;
    };
    QCustomPlot *plot;
    vector<Graph> graphs;
public:
    Plot(ScreenOutput *sout);
    void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
};

#endif // MAINWINDOW_H
