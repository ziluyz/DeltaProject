#include "mainwindow.h"

MainWindow::MainWindow(Data *data) : QWidget(), needUpdate(false)
{
    this->data = data;
    Data &d = *data;
    for (auto &wgt : d.screenOutputs) {
        content.push_back(new QLabel(this));
        auto &lbl = *static_cast<QLabel*>(content.back());
        lbl.move(0,content.size()*100 - 100);
    }
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (!needUpdate) return;
    for (int i = 0; i < data->screenOutputs.size(); i++) {
        auto &lbl = *static_cast<QLabel*>(content[i]);
        auto &sout = data->screenOutputs[i];
        QString output("");
        for (auto &item : sout.items) {
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
                break;
            }
            case Types::DOUBLEVECTOR: {
                output.append("Vector " + item.mem->name + " : ");
                auto &vec = *static_cast<vector<double>*>(item.mem->mem);
                for (auto d : vec) output.append(QString::number(d) + ", ");
                output.truncate(output.size() - 2);
                output.append("\n");
                break;
            }
            }
        }
        lbl.setText(output);
        lbl.adjustSize();
    }
    needUpdate = false;
}
