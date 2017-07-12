#ifndef PLOT_H
#define PLOT_H

#include "qcustomplot.h"
#include "wgt.h"

using namespace std;

struct OutputItem;

class Plot : public QObject, public Wgt {
    Q_OBJECT
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
public slots:
    void showContextMenu(const QPoint &point);
};

#endif // PLOT_H
