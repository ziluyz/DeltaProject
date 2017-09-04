#include "mainwindow.h"

// populates itself with ScreenOutput widgets according to their positions
MainWindow::MainWindow(Data *data) : QWidget(), needUpdate(false)
{
    this->data = data;
    Data &d = *data;
    QGridLayout *lay = new QGridLayout();
    setLayout(lay);
    // cycle over all ScrrenOutputs
    for (ScreenOutput &sout : d.screenOutputs) {
        // constructing widgets according to types
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
        // extracting widget position in the layout
        QString pos = sout.attributes["position"];
        QStringList sp = pos.split(':');
        if (sp.size() != 2) throw QString("Invalid position ") + pos + " of ScreenOutput";
        QStringList coords = sp.at(0).split(',');
        if (coords.size() != 2) throw QString("Invalid position ") + pos + " of ScreenOutput";
        QStringList spans = sp.at(1).split(',');
        if (spans.size() != 2) throw QString("Invalid position ") + pos + " of ScreenOutput";
        // attaching widget to the layout
        content.back()->attach(*lay, coords.at(1).toInt() - 1, coords.at(0).toInt() - 1, spans.at(1).toInt(), spans.at(0).toInt());
        // each variable knows all the widgets it is drawn by
        for (OutputItem &item : sout.items) item.var->wgts.push_back(content.back());
    }
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (!needUpdate) return; // gllobal flag shows if redraw is necessary at all
    toDraw.clear(); // set should contain only those which really contain recently updated data
    for (Variable& var : data->inputVars) {
        if (var.isNew) { // input variables are always valid
            var.isNew = false;
            var.needUpdate = true;
            for (shared_ptr<Wgt> wgt : var.wgts) toDraw.insert(wgt);
        }
        else var.needUpdate = false;
    }
    for (Variable& var : data->outputVars) {
        if (var.isNew && var.isValid) { // output variable can be updated but invalid
            var.isNew = false;
            var.needUpdate = true;
            for (shared_ptr<Wgt> wgt : var.wgts) toDraw.insert(wgt);
        }
        else var.needUpdate = false;
    }
    // draw what should be drawn
    for (shared_ptr<Wgt> wgt : toDraw) {
        wgt->draw();
    }
    needUpdate = false; // waiting for new updates
}

// generate output folder for this particular run or return existing, if already exists
unique_ptr<QDir> MainWindow::outputFolder() {
    unique_ptr<QDir> dir(new QDir());
    // if common ./Output folder does not exist, create it
    if (!dir->exists("Output")) dir->mkdir("Output");
    dir->cd("Output");
    // if output folder for the current run does not exist, create it...
    QString folder = data->startTime.toString("yyMMdd_hhmmss_zzz");
    if (!dir->exists(folder)) {
        dir->mkdir(folder);
        dir->cd(folder);
        // ...and copy input ixml there
        QFile::copy(QDir().absoluteFilePath(data->inputFile), dir->absoluteFilePath(data->inputFile));
    }
    else dir->cd(folder);
    // return pointer to output path
    return dir;
}
