#include "mainwindow.h"

// populates itself with ScreenOutput widgets according to their positions
MainWindow::MainWindow(Data *data) : QWidget(), needUpdate(true)
{
    this->data = data;
    Data &d = *data;
    //Main Layout
    QBoxLayout *mainBox = new QBoxLayout(QBoxLayout::RightToLeft);
    setLayout(mainBox);
    //Control Panel
    QBoxLayout *controlPanel = new QBoxLayout(QBoxLayout::TopToBottom);
    mainBox->addLayout(controlPanel, 0);
    pauseButton = new QPushButton("Pause");
    connect(pauseButton, SIGNAL(clicked(bool)), SLOT(pauseClicked()));
    controlPanel->addWidget(pauseButton);
    timeLabel = new QLabel();
    controlPanel->addWidget(timeLabel);
    //Output Widgets
    QGridLayout *lay = new QGridLayout();
    mainBox->addLayout(lay, 1);
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
    if (data->paused) return;
    auto end = chrono::steady_clock::now();
    if (data->thread->isFinished()) end = data->chronoEnd;
    double tdur = chrono::duration_cast<chrono::duration<double>>(end - data->chronoStart).count();
    int hours = tdur / 3600;
    tdur -= hours * 3600;
    int minutes = tdur / 60;
    tdur -= minutes * 60;
    int seconds = tdur;
    tdur -= seconds;
    int millis = tdur * 1000;
    QString time = QString::number(hours) + ":";
    if (minutes < 10) time += "0";
    time += QString::number(minutes) + ":";
    if (seconds < 10) time += "0";
    time += QString::number(seconds) + ":";
    if (millis < 100) time += "0";
    if (millis < 10) time += "0";
    time += QString::number(millis);
    timeLabel->setText(time);
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
        //QFile::copy(QDir().absoluteFilePath(data->inputFile), dir->absoluteFilePath(data->inputFile));
        QFile file(dir->absoluteFilePath(data->inputFile));
        file.open(QFile::WriteOnly | QFile::Truncate);
        QTextStream tstr(&file);
        data->inputIXML.save(tstr, 4);
    }
    else dir->cd(folder);
    // return pointer to output path
    return dir;
}

void MainWindow::calcStarted() {
    data->chronoStart = chrono::steady_clock::now();
}

void MainWindow::calcFinished() {
    data->chronoEnd = chrono::steady_clock::now();
    pauseButton->setEnabled(false);
}

void MainWindow::pauseClicked() {
    data->paused = !data->paused;
    if (data->paused) {
        pauseButton->setText("Resume");
        timePaused = chrono::steady_clock::now();
    }
    else {
        pauseButton->setText("Pause");
        data->chronoStart += chrono::steady_clock::now() - timePaused;
    }
}
