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
            content.emplace_back(new TextField(&sout));
            break;
        default:
            throw QString("Error initializing ScreenOutput");
            break;
        }
        content.back()->attach(*lay,content.size() - 1, 0, 1, 1);
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
    base = new QGridLayout();
    label = new QLabel("");
}

void TextField::attach(QGridLayout& c, int row, int col, int rowspan, int colspan) {
    c.addLayout(base, row, col, rowspan, colspan);
    base->addWidget(label);
}

void TextField::draw() {
    QString output;
    for (OutputItem& item : source->items) {
        if (!item.mem->isValid) continue;
        switch(item.mem->type) {
        case Types::INT:
            output.append(item.mem->name + " = " + QString::number(*static_cast<int*>(item.mem->mem)) + "\n");
            break;
        case Types::DOUBLE:
            output.append(item.mem->name + " = " + QString::number(*static_cast<double*>(item.mem->mem)) + "\n");
            break;
        case Types::INTVECTOR: {
            output.append("Vector " + item.mem->name + " : ");
            auto &vec = *static_cast<vector<int>*>(item.mem->mem);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            output.append("\n");
            break;}
        case Types::DOUBLEVECTOR: {
            output.append("Vector " + item.mem->name + " : ");
            auto &vec = *static_cast<vector<double>*>(item.mem->mem);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            output.append("\n");
            break;}
        }
    }
    label->setText(output);
    label->adjustSize();
}
