#include "plot.h"

Plot::Plot(ScreenOutput *sout) : Wgt(sout) {
    plot = new QCustomPlot();

    auto find = [](QString xtag, vector<Graph> &gs) {
        for (Graph &g : gs) {
            if (g.xtag == xtag) return &g;
        }
        return static_cast<Graph*>(nullptr);
    };

    for (OutputItem &item : source->items) {
        if (item.attributes.contains("xtag")) {
            QString xtag = item.attributes["xtag"];
            Graph *pg = find(xtag, graphs);
            if (pg == nullptr) {
                graphs.emplace_back();
                Graph &g = graphs.back();
                g.xtag = xtag;
                g.x = &item;
            }
            else {
                if (pg->x != nullptr) throw QString("Double xtag for Plot");
                pg->x = &item;
            }
        }
        else if (item.attributes.contains("xref")) {
            QString xtag = item.attributes["xref"];
            Graph *pg = find(xtag, graphs);
            if (pg == nullptr) {
                graphs.emplace_back();
                Graph &g = graphs.back();
                g.xtag = xtag;
                g.ys.emplace_back();
                g.ys.back().y = &item;
            }
            else {
                pg->ys.emplace_back();
                pg->ys.back().y = &item;
            }
        }
        else throw QString("Neither xtag nor xref for item in Plot");
    }
    for (Graph &g : graphs) {
        if (g.x == nullptr) throw QString("No xtag for xref ") + g.xtag;
        if (g.ys.size() == 0) throw QString("No xref for xtag") + g.xtag;
    }
}

void Plot::attach(QGridLayout &c, int row, int col, int rowspan, int colspan) {
    c.addWidget(plot, row, col, rowspan, colspan);
    for (Graph &g : graphs) for (Graph::gpair &y : g.ys) y.graph = plot->addGraph();
}

void Plot::draw() {
    for (Graph &g : graphs) {
        if (!g.x->var->isValid) continue;
        auto vx = QVector<double>::fromStdVector(*static_cast<vector<double>*>(g.x->var->mem));
        for (Graph::gpair &y : g.ys) {
            if (!y.y->var->isValid) continue;
            if (y.y->var->needUpdate || g.x->var->needUpdate) {
                QVector<double> vy = QVector<double>::fromStdVector(*static_cast<vector<double>*>(y.y->var->mem));
                y.graph->setData(vx, vy);
                y.graph->rescaleAxes();
            }
        }
    }
    plot->replot();
}
