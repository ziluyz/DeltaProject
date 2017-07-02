#include "mainwindow.h"

MainWindow::MainWindow(Data *data) : QWidget(), needUpdate(false)
{
    this->data = data;
    Data &d = *data;
    QGridLayout *lay = new QGridLayout();
    setLayout(lay);
    for (ScreenOutput &sout : d.screenOutputs) {
        switch (sout.type) {
        case ScreenTypes::TEXTFIELD:
            content.emplace_back(new TextField(&sout));
            break;
        case ScreenTypes::PLOT:
            content.emplace_back(new Plot(&sout));
            break;
        default:
            throw QString("Error initializing ScreenOutput");
            break;
        }
        QString pos = sout.attributes["position"];
        QStringList sp = pos.split(':');
        if (sp.size() != 2) throw QString("Invalid position ") + pos + " of ScreenOutput";
        QStringList coords = sp.at(0).split(',');
        if (coords.size() != 2) throw QString("Invalid position ") + pos + " of ScreenOutput";
        QStringList spans = sp.at(1).split(',');
        if (spans.size() != 2) throw QString("Invalid position ") + pos + " of ScreenOutput";
        content.back()->attach(*lay, coords.at(1).toInt() - 1, coords.at(0).toInt() - 1, spans.at(1).toInt(), spans.at(0).toInt());
    }
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (!needUpdate) return;
    for (shared_ptr<Wgt> wgt : content) {
        wgt->draw();
    }
    needUpdate = false;
}

TextField::TextField(ScreenOutput *sout) : Wgt(sout) {
    label = new QLabel("");
}

void TextField::attach(QGridLayout& c, int row, int col, int rowspan, int colspan) {
    c.addWidget(label, row, col, rowspan, colspan);
}

void TextField::draw() {
    QString output = source->attributes.contains("title") ? source->attributes["title"] + "\n" : "";
    for (OutputItem& item : source->items) {
        if (!item.mem->isValid) continue;
        output.append(item.mem->desc + ": ");
        switch(item.mem->type) {
        case Types::INT:
            output.append(QString::number(*static_cast<int*>(item.mem->mem)));
            break;
        case Types::DOUBLE:
            output.append(QString::number(*static_cast<double*>(item.mem->mem)));
            break;
        case Types::INTVECTOR: {
            auto &vec = *static_cast<vector<int>*>(item.mem->mem);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            break;}
        case Types::DOUBLEVECTOR: {
            auto &vec = *static_cast<vector<double>*>(item.mem->mem);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            break;}
        }
        output.append(" " + item.mem->unit + "\n");
    }
    label->setText(output);
    label->adjustSize();
    //label->setStyleSheet("QLabel { background-color : red; color : blue; }");
}

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
        if (!g.x->mem->isValid) continue;
        QVector<double> vx = QVector<double>::fromStdVector(*static_cast<vector<double>*>(g.x->mem->mem));
        for (Graph::gpair &y : g.ys) {
            if (!y.y->mem->isValid) continue;
            QVector<double> vy = QVector<double>::fromStdVector(*static_cast<vector<double>*>(y.y->mem->mem));
            y.graph->setData(vx, vy);
            y.graph->rescaleAxes();
        }
    }
    plot->replot();
}
