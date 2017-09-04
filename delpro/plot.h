#ifndef PLOT_H
#define PLOT_H

#include "qcustomplot.h"
#include "wgt.h"

using namespace std;

struct OutputItem;

//Plotting graphs based on DoubleVectors
class Plot : public QObject, public Wgt {
    Q_OBJECT
private:
    //A set of QCPGraphs with the same x data
    struct Graph {
        //A variable, its QVector buffer and corresponding QCPGraph
        struct gpair {
            OutputItem *y;
            QVector<double> ydata;
            QCPGraph *graph;
        };

        QString xtag; // tag for x value in ixml
        OutputItem *x; // x variable
        QVector<double> xdata; // buffer for x variable
        vector<gpair> ys; // from the set of y variable referencing the xtag (xref="xtag") in ixml
    };
    QCustomPlot *plot; // main plot object
    vector<Graph> graphs; // vector.size = number of different xtags in this ScreenOutput
public:
    Plot(ScreenOutput *sout);
    void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
public slots:
    void showContextMenu(const QPoint &point); // for RButton menu
    void syncGraphsWithLegend(bool hz); // to synchronize the selection of curves...
    void syncLegendWithGraphs(bool hz); // ...and corresponding legends
};

#endif // PLOT_H
