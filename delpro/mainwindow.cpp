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
        for (int j = 0; j < sout.items.size(); j++) {
            auto &item = sout.items[j];
            QString name;
            Types type;
            void* var;
            if (!item.isOutput) {
                name = data->inputNames[item.index];
                type = data->inputTypes[item.index];
                var = data->inputVars[item.index];
            }
            else if (data->outputIsValid[item.index]) {

                name = data->outputNames[item.index];
                type = data->outputTypes[item.index];
                var = data->outputVars[item.index];
            }
            else continue;
            switch(type) {
            case Types::INT:
                output.append(name + " = " + QString::number(*static_cast<int*>(var)) + "\n");
                break;
            case Types::DOUBLE:
                output.append(name + " = " + QString::number(*static_cast<double*>(var)) + "\n");
                break;
            case Types::INTVECTOR: {
                output.append("Vector " + name + " : ");
                auto &vec = *static_cast<vector<int>*>(var);
                for (auto d : vec) output.append(QString::number(d) + ", ");
                output.truncate(output.size() - 2);
                output.append("\n");
                break;
            }
            case Types::DOUBLEVECTOR: {
                output.append("Vector " + name + " : ");
                auto &vec = *static_cast<vector<double>*>(var);
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
