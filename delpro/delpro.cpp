#include "delpro.h"
#include <iostream>
#include <QApplication>
#include <QDomDocument>

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
    else throw QString("Unknown input type");
    data->inputVars.push_back(var);

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
    else throw QString("Unknown output type");
    data->outputVars.push_back(var);
    data->outputIsNew.push_back(false);
    data->outputIsValid.push_back(false);

    return data->outputNames.size() - 1;
}

int execute(int argc, char** argv, int (*f)(), void* data) {
    Data &d = *static_cast<Data*>(data);
    QString fname("input.ixml");
    if (argc > 1) fname = argv[1];
    try {
        parseInput(fname, d);
    }
    catch (QString str) {
        cout << str.toStdString() << endl;
        return 1;
    }

    QApplication app(argc, argv);

    MainWindow window(&d);
    d.window = &window;
    window.showMaximized();
    window.startTimer(500);

    CalcThread thread(f);
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

void parseInput(QString &filename, Data &data) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) throw QString("Error opening input file");
    QDomDocument doc("input_file");
    QString parsError;
    int errorLine;
    if (!doc.setContent(&file, false, &parsError, &errorLine)) {
        file.close();
        throw QString("Error parsing input file\n") + parsError + "\nLine: " + QString::number(errorLine);
    }
    file.close();
    auto root = doc.documentElement();

    auto find = [](QDomNodeList &list, QString &name) {
        for (int i = 0, len = list.size(); i < len; i++) {
            auto node = list.at(i);
            if (node.nodeName() != "Var") continue;
            if (!node.toElement().hasAttribute("name")) continue;
            if (node.toElement().attribute("name") == name) return i;
        }
        return -1;
    };

    auto input = root.elementsByTagName("Input").at(0).childNodes();

    for (int i = 0; i < data.inputNames.size(); i++) {
        auto inv = data.inputNames[i];
        int ind = find(input, inv);
        if (ind == -1) throw QString("No input for ") + inv;
        auto el = input.at(ind).toElement();
        data.inputDescriptions.push_back(el.attribute("desc"));
        data.inputUnits.push_back(el.attribute("unit"));
        bool ok;
        auto value = el.firstChild().nodeValue();
        switch (data.inputTypes[i]) {
        case Types::INT:
            *static_cast<int*>(data.inputVars[i])=value.toInt(&ok);
            break;
        case Types::DOUBLE:
            *static_cast<double*>(data.inputVars[i])=value.toDouble(&ok);
            break;
        case Types::INTVECTOR: {
            auto list = value.split(QRegExp("\\s+"));
            auto &vec = *static_cast<vector<int>*>(data.inputVars[i]);
            for (auto v : list) {
                vec.push_back(v.toInt(&ok));
                if (!ok) break;
            }
            break;}
        case Types::DOUBLEVECTOR: {
            auto list = value.split(QRegExp("\\s+"));
            auto &vec = *static_cast<vector<double>*>(data.inputVars[i]);
            for (auto v : list) {
                vec.push_back(v.toDouble(&ok));
                if (!ok) break;
            }
            break;}
        }
        if (!ok) throw QString("Error parsing value of ") + inv + " from " + value;
    }
}
