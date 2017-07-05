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
        for (OutputItem &item : sout.items) item.var->wgts.push_back(content.back());
    }
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (!needUpdate) return;
    toDraw.clear();
    for (Variable& var : data->inputVars) {
        if (var.isNew) {
            var.isNew = false;
            var.needUpdate = true;
            for (shared_ptr<Wgt> wgt : var.wgts) toDraw.insert(wgt);
        }
        else var.needUpdate = false;
    }
    for (Variable& var : data->outputVars) {
        if (var.isNew && var.isValid) {
            var.isNew = false;
            var.needUpdate = true;
            for (shared_ptr<Wgt> wgt : var.wgts) toDraw.insert(wgt);
        }
        else var.needUpdate = false;
    }
    for (shared_ptr<Wgt> wgt : toDraw) {
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
        if (!item.var->isValid) continue;
        output.append(item.var->desc + ": ");
        switch(item.var->type) {
        case Types::INT:
            output.append(QString::number(*static_cast<int*>(item.var->mem)));
            break;
        case Types::DOUBLE:
            output.append(QString::number(*static_cast<double*>(item.var->mem)));
            break;
        case Types::INTVECTOR: {
            auto &vec = *static_cast<vector<int>*>(item.var->mem);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            break;}
        case Types::DOUBLEVECTOR: {
            auto &vec = *static_cast<vector<double>*>(item.var->mem);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            break;}
        }
        output.append(" " + item.var->unit + "\n");
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
