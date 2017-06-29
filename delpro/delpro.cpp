#include "delpro.h"
#include <iostream>
#include <QApplication>

using namespace std;

int registerInput(const char* name, const char* type, void *var, void *pdata) {
    Data *&data = *static_cast<Data**>(pdata);
    if (data == nullptr) data = new Data;

    data->inputNames.push_back(name);
    string stype(type);
    if (stype == "int") data->inputTypes.push_back(Types::INT);
    else if (stype == "double") data->inputTypes.push_back(Types::DOUBLE);
    else if (stype == "intvector") data->inputTypes.push_back(Types::INTVECTOR);
    else if (stype == "doublevector") data->inputTypes.push_back(Types::DOUBLEVECTOR);
    else throw string("Unknown input type");
    data->inputVars.push_back(var);

    *((double*)var) = data->inputNames.size();

    return data->inputNames.size() - 1;
}

int registerOutput(const char* name, const char* type, void *var, void *pdata) {
    Data *&data = *static_cast<Data**>(pdata);
    if (data == nullptr) data = new Data;

    data->outputNames.push_back(name);
    string stype(type);
    if (stype == "int") data->outputTypes.push_back(Types::INT);
    else if (stype == "double") data->outputTypes.push_back(Types::DOUBLE);
    else if (stype == "intvector") data->outputTypes.push_back(Types::INTVECTOR);
    else if (stype == "doublevector") data->outputTypes.push_back(Types::DOUBLEVECTOR);
    else throw string("Unknown output type");
    data->outputVars.push_back(var);
    data->outputIsNew.push_back(false);
    data->outputIsValid.push_back(false);

    return data->outputNames.size() - 1;
}

int execute(int argc, char** argv, int (*f)(), void* data) {
    CalcThread thread(f);

    QApplication app(argc, argv);
    MainWindow window(data);
    static_cast<Data*>(data)->window = &window;
    window.label->setText("Hello");
    window.showMaximized();
    window.startTimer(500);
    thread.start(QThread::HighPriority);
    app.exec();

    return 0;
}

void updateOutput(int index, void *data) {
    Data &d = *static_cast<Data*>(data);
    d.outputIsNew[index] = true;
}

void validateOutput(int index, bool isValid, void *data) {
    Data &d = *static_cast<Data*>(data);
    d.outputIsValid[index] = isValid;
    if (d.window!=nullptr) d.window->needUpdate = d.window->needUpdate || isValid;
}
