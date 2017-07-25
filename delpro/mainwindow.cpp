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
        if (var.isNew) {
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

unique_ptr<QDir> MainWindow::outputFolder() {
    unique_ptr<QDir> dir(new QDir());
    if (!dir->exists("Output")) dir->mkdir("Output");
    dir->cd("Output");
    QString folder = data->startTime.toString("yyMMdd_hhmmss_zzz");
    if (!dir->exists(folder)) {
        dir->mkdir(folder);
        dir->cd(folder);
        QFile::copy(QDir().absoluteFilePath(data->inputFile), dir->absoluteFilePath(data->inputFile));
    }
    else dir->cd(folder);
    return dir;
}
