#include "plot.h"
#include "mainwindow.h"
#include <algorithm>

Plot::Plot(ScreenOutput *sout) : QObject(), Wgt(sout) {
    plot = new QCustomPlot();
    //Setting automatically processed mouse events
    plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectPlottables | QCP::iSelectLegend | QCP::iMultiSelect);

    //Positioning of the legend
    QString legend = source->attributes["legend"];
    QString lwrap = source->attributes["legend_wrap"];
    int lw;
    bool ok;
    lw = lwrap.toInt(&ok);
    if (!ok) lw = 2;
    if (legend == "off");
    else {
        plot->legend->setVisible(true);
        plot->legend->setSelectableParts(QCPLegend::spItems);
        if (legend == "out" || legend == "right") {
            QCPLayoutGrid *subLayout = new QCPLayoutGrid();
            plot->plotLayout()->addElement(0, 1, subLayout);
            subLayout->setMargins(QMargins(0,5,5,5));
            subLayout->addElement(0, 0, plot->legend);
            subLayout->insertRow(1);
            subLayout->setRowStretchFactor(1,100);
            plot->plotLayout()->setColumnStretchFactor(1,0.001);
        }
        else if (legend == "bottom") {
            QCPLayoutGrid *subLayout = new QCPLayoutGrid();
            plot->plotLayout()->addElement(1, 0, subLayout);
            subLayout->setMargins(QMargins(5,0,5,5));
            subLayout->addElement(0, 0, plot->legend);
            plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
            plot->legend->setWrap(lw);
            plot->plotLayout()->setRowStretchFactor(1,0.001);
        }
    }

    //Find Graph with given xtag
    auto find = [](QString xtag, vector<Graph> &gs) {
        for (Graph &g : gs) {
            if (g.xtag == xtag) return &g; // and return pointer to it
        }
        return static_cast<Graph*>(nullptr); // or nullptr
    };

    //Populate graphs depending on xtag and xref attributes
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
    holder->setContentsMargins(0,15,0,0);
    //Setting Title
    QLabel *title = new QLabel(source->attributes["title"]);
    title->setFont(QFont("Helvetica",11,QFont::Bold));
    holder->addWidget(title, 0, Qt::AlignHCenter);
    holder->addWidget(plot, 1);
    c.addLayout(holder, row, col, rowspan, colspan);
    for (Graph &g : graphs) for (Graph::gpair &y : g.ys) {
        //Choosing y-axis position
        QString yaxis = y.y->attributes["yaxis"];
        if (yaxis == "right") {
            yaxis = "[R] "; // Hint to the legend
            y.graph = plot->addGraph(plot->xAxis, plot->yAxis2);
            plot->yAxis2->setVisible(true);
        }
        else {
            yaxis = "[L] "; // Hint to the legend
            y.graph = plot->addGraph();
        }
        //Processing attributes
        double width = 1;
        if (!y.y->attributes["width"].isNull()) width = y.y->attributes["width"].toDouble();
        Qt::PenStyle style = Qt::SolidLine;
        QString ss = y.y->attributes["style"];
        if (ss == "dot") style = Qt::DotLine;
        else if (ss == "dash") style = Qt::DashLine;
        else if (ss == "dashdot") style = Qt::DashDotLine;
        else if (ss == "dashdotdot") style = Qt::DashDotDotLine;
        y.graph->setPen(QPen(QBrush(QColor(y.y->attributes["color"])), width, style));
        auto name = y.y->var->desc;
        if (y.y->var->type == Types::DOUBLEVECTORSET) name += "#" + y.y->attributes["index"];
        y.graph->setName(yaxis + name + " (" + y.y->var->unit + ")");
        //Synchronize curves and legends selection
        connect(plot->legend->itemWithPlottable(y.graph), SIGNAL(selectionChanged(bool)), SLOT(syncGraphsWithLegend(bool)));
        connect(y.graph, SIGNAL(selectionChanged(bool)), SLOT(syncLegendWithGraphs(bool)));
    }
    plot->setContextMenuPolicy(Qt::CustomContextMenu);// show context menu on rbuttonclick
    connect(plot, SIGNAL(customContextMenuRequested(const QPoint&)), SLOT(showContextMenu(const QPoint&)));
}

void Plot::draw() {
    set<QCPAxis*> usedAxes; // to enlarge axis range for second and further curves
    for (Graph &g : graphs) {
        if (!g.x->var->isValid) continue;
        if (g.x->var->needUpdate) {
            g.xdata = QVector<double>::fromStdVector(*static_cast<vector<double>*>(g.x->var->mem)); // xdata to buffer
        }
        for (Graph::gpair &y : g.ys) {
            if (!y.y->var->isValid) continue;
            if (y.y->var->needUpdate) {
                vector<double> *vec;
                if (y.y->var->type == Types::DOUBLEVECTORSET) {
                    auto &vecs = *static_cast<vector<vector<double>>*>(y.y->var->mem);
                    size_t ind = y.y->attributes["index"].toInt();
                    vec = &vecs[ind];
                }
                else vec = static_cast<vector<double>*>(y.y->var->mem);
                y.ydata = QVector<double>::fromStdVector(*vec); // ydata to buffer
            }
            if (y.y->var->needUpdate || g.x->var->needUpdate) {
                y.graph->setData(g.xdata, y.ydata);
                if (usedAxes.count(y.graph->valueAxis())) y.graph->rescaleAxes(true);
                else {
                    usedAxes.insert(y.graph->valueAxis());
                    y.graph->rescaleAxes();
                }
                if (y.y->attributes.count("ymin")) y.graph->valueAxis()->setRangeLower(y.y->attributes["ymin"].toInt());
                if (y.y->attributes.count("ymax")) y.graph->valueAxis()->setRangeUpper(y.y->attributes["ymax"].toInt());
                if (y.y->attributes.count("ysymmetric")) {
                    auto rang = y.graph->valueAxis()->range();
                    double ylim = abs(rang.upper);
                    if (abs(rang.lower) > ylim) ylim = abs(rang.lower);
                    y.graph->valueAxis()->setRange(-ylim, ylim);
                }
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
                auto name = gp.y->var->desc;
                if (gp.y->var->type == Types::DOUBLEVECTORSET) name += "#" + gp.y->attributes["index"];
                if (selectedLines.contains(gp.graph)) str = str + name + " (" + gp.y->var->unit + "), ";
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

void Plot::syncGraphsWithLegend(bool hz) {
    for (int i = 0, len = plot->legend->itemCount(); i < len; i++) {
        static_cast<QCPPlottableLegendItem*>(plot->legend->item(i))->plottable()->
                setSelection(QCPDataSelection(QCPDataRange(0,
                                                           plot->legend->item(i)->selected())));
    }
}

void Plot::syncLegendWithGraphs(bool hz) {
    QSet<QCPGraph*> set = QSet<QCPGraph*>::fromList(plot->selectedGraphs());
    for (int i = 0, len = plot->graphCount(); i < len; i++) {
        plot->legend->itemWithPlottable(plot->graph(i))->setSelected(set.contains(plot->graph(i)));
    }
}
