#include "mainwindow.h"

MainWindow::MainWindow(void *data) : QWidget(), needUpdate(false)
{
    this->data = static_cast<Data*>(data);
    label = new QLabel(this);
}

void MainWindow::timerEvent(QTimerEvent *event) {
    if (!needUpdate) return;
    QString output("");
    for (int i = 0; i < data->outputNames.size(); i++) {
        if (!data->outputIsValid[i]) continue;
        switch(data->outputTypes[i]) {
        case Types::INT:
            output.append(data->outputNames[i] + " = " + QString::number(*static_cast<int*>(data->outputVars[i])) + "\n");
            break;
        case Types::DOUBLE:
            output.append(data->outputNames[i] + " = " + QString::number(*static_cast<double*>(data->outputVars[i])) + "\n");
            break;
        case Types::INTVECTOR: {
            output.append("Vector " + data->outputNames[i] + " : ");
            auto &vec = *static_cast<vector<int>*>(data->outputVars[i]);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            output.append("\n");
            break;
        }
        case Types::DOUBLEVECTOR: {
            output.append("Vector " + data->outputNames[i] + " : ");
            auto &vec = *static_cast<vector<double>*>(data->outputVars[i]);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            output.append("\n");
            break;
        }
        }
    }
    label->setText(output);
    label->adjustSize();
    needUpdate = false;
}
