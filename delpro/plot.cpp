#include "plot.h"
#include "mainwindow.h"
#include <algorithm>

Plot::Plot(ScreenOutput *sout) : QObject(), Wgt(sout) {
    plot = new QCustomPlot();
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectLegend | QCP::iMultiSelect);
    plot->legend->setVisible(true);
    plot->legend->setSelectableParts(QCPLegend::spItems);

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
                plot->xAxis->setLabel(g.x->var->desc + " (" + g.x->var->unit + ")");
            }
            else {
                if (pg->x != nullptr) throw QString("Double xtag for Plot");
                pg->x = &item;
                plot->xAxis->setLabel(pg->x->var->desc + " (" + pg->x->var->unit + ")");
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
    QVBoxLayout *holder = new QVBoxLayout();
    QLabel *title = new QLabel(source->attributes["title"]);
    title->setFont(QFont("Helvetica",11,QFont::Bold));
    holder->addWidget(title, 0, Qt::AlignHCenter);
    holder->addWidget(plot, 1);
    c.addLayout(holder, row, col, rowspan, colspan);
    for (Graph &g : graphs) for (Graph::gpair &y : g.ys) {
        y.graph = plot->addGraph();
        double width = 1;
        if (!y.y->attributes["width"].isNull()) width = y.y->attributes["width"].toDouble();
        Qt::PenStyle style = Qt::SolidLine;
        QString ss = y.y->attributes["style"];
        if (ss == "dot") style = Qt::DotLine;
        else if (ss == "dash") style = Qt::DashLine;
        else if (ss == "dashdot") style = Qt::DashDotLine;
        else if (ss == "dashdotdot") style = Qt::DashDotDotLine;
        y.graph->setPen(QPen(QBrush(QColor(y.y->attributes["color"])), width, style));
        y.graph->setName(y.y->var->desc + " (" + y.y->var->unit + ")");
        connect(plot->legend->itemWithPlottable(y.graph), SIGNAL(selectionChanged(bool)), SLOT(syncGraphsWithLegend(bool)));
        connect(y.graph, SIGNAL(selectionChanged(bool)), SLOT(syncLegendWithGraphs(bool)));
    }
    plot->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(plot, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showContextMenu(const QPoint&)));
}

void Plot::draw() {
    for (Graph &g : graphs) {
        if (!g.x->var->isValid) continue;
        if (g.x->var->needUpdate) {
            g.xdata = QVector<double>::fromStdVector(*static_cast<vector<double>*>(g.x->var->mem));
        }
        for (Graph::gpair &y : g.ys) {
            if (!y.y->var->isValid) continue;
            if (y.y->var->needUpdate) {
                y.ydata = QVector<double>::fromStdVector(*static_cast<vector<double>*>(y.y->var->mem));
            }
            if (y.y->var->needUpdate || g.x->var->needUpdate) {
                y.graph->setData(g.xdata, y.ydata);
                y.graph->rescaleAxes(true);
            }
        }
    }
    plot->replot();
}

void Plot::showContextMenu(const QPoint &point) {
    auto savePng = [this]() {
        auto dir = static_cast<MainWindow*>(plot->parent())->outputFolder();
        plot->savePng(dir->absoluteFilePath(QDateTime::currentDateTime().toString("yyMMdd_hhmmss_zzz_") +
                                            source->attributes["title"] + ".png"));
    };
    auto savePdf = [this]() {
        auto dir = static_cast<MainWindow*>(plot->parent())->outputFolder();
        plot->savePdf(dir->absoluteFilePath(QDateTime::currentDateTime().toString("yyMMdd_hhmmss_zzz_") +
                                            source->attributes["title"] + ".pdf"));
    };
    auto saveData = [this](QSet<QCPGraph*> &selectedLines) {
        vector<Graph*> selectedGraphs;
        for (Graph &g : graphs) {
            for (Graph::gpair &gp : g.ys) {
                if (selectedLines.contains(gp.graph)) {
                    selectedGraphs.push_back(&g);
                    break;
                }
            }
        }
        auto dir = static_cast<MainWindow*>(plot->parent())->outputFolder();
        QFile file(dir->absoluteFilePath(QDateTime::currentDateTime().toString("yyMMdd_hhmmss_zzz_") +
                                            source->attributes["title"] + ".dat"));
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        QString str;
        for (Graph *g : selectedGraphs) {
            str = str + g->x->var->desc + " (" + g->x->var->unit + "), ";
            for (Graph::gpair &gp : g->ys) {
                if (selectedLines.contains(gp.graph)) str = str + gp.y->var->desc + " (" + gp.y->var->unit + "), ";
            }
        }
        str.truncate(str.size() - 2);
        out << str << endl;
        int ind = 0;
        bool endOfData = false;
        while (!endOfData) {
            str.clear();
            endOfData = true;
            for (Graph *g : selectedGraphs) {
                if (g->xdata.size() > ind) {
                    str = str + QString::number(g->xdata.at(ind)) + ", ";
                    endOfData = false;
                }
                else str = str + ",";
                for (Graph::gpair &gp : g->ys) {
                    if (gp.ydata.size() > ind) {
                        if (selectedLines.contains(gp.graph)) str = str + QString::number(gp.ydata.at(ind)) + ", ";
                        endOfData = false;
                    }
                    else str = str + ",";
                }
            }
            if (!endOfData) {
                str.truncate(str.size() - 2);
                out << str << endl;
            }
            ind++;
        }
        file.close();
    };
    auto saveSelected = [this, saveData]() {
        QSet<QCPGraph*> lines = QSet<QCPGraph*>::fromList(plot->selectedGraphs());
        saveData(lines);
    };
    auto saveAll = [this, saveData]() {
        QSet<QCPGraph*> lines;
        for (int i = 0, len = plot->graphCount(); i < len; i++) lines.insert(plot->graph(i));
        saveData(lines);
    };

    QMenu *menu = new QMenu(plot);
    menu->move(plot->pos() + point);
    if (plot->selectedGraphs().size() > 0) menu->addAction("Save selected as data", saveSelected);
    menu->addSeparator();
    menu->addAction("Save all as data", saveAll);
    menu->addAction("Save all as png", savePng);
    menu->addAction("Save all as pdf", savePdf);
    menu->exec();
    delete menu;
}

#include <iostream>

void Plot::syncGraphsWithLegend(bool hz) {
    cout << "Legend " << hz << endl;
    for (int i = 0, len = plot->legend->itemCount(); i < len; i++) {
        static_cast<QCPPlottableLegendItem*>(plot->legend->item(i))->plottable()->
                setSelection(QCPDataSelection(QCPDataRange(0,
                                                           plot->legend->item(i)->selected())));
    }
}

void Plot::syncLegendWithGraphs(bool hz) {
    cout << "Graph " << hz << endl;
    QSet<QCPGraph*> set = QSet<QCPGraph*>::fromList(plot->selectedGraphs());
    for (int i = 0, len = plot->graphCount(); i < len; i++) {
        plot->legend->itemWithPlottable(plot->graph(i))->setSelected(set.contains(plot->graph(i)));
    }
}
