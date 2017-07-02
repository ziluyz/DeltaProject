#include "delpro.h"
#include <iostream>
#include <QApplication>
#include <QDomDocument>

using namespace std;

int registerInput(const char* name, const char* type, void *var, void *pdata) {
    auto *&data = *static_cast<Data**>(pdata);
    if (data == nullptr) data = new Data;
    return registerVar(name, type, var, data->inputVars);
}

int registerOutput(const char* name, const char* type, void *var, void *pdata) {
    auto *&data = *static_cast<Data**>(pdata);
    if (data == nullptr) data = new Data;
    return registerVar(name, type, var, data->outputVars);
}

int execute(int argc, char** argv, int (*f)(), void* data) {
    auto &d = *static_cast<Data*>(data);
    QString fname("input.ixml");
    if (argc > 1) fname = argv[1];
    try {
        parseInput(fname, d);

        QApplication app(argc, argv);

        MainWindow window(&d);
        d.window = &window;
        window.showMaximized();
        window.startTimer(500);

        CalcThread thread(f);
        thread.start(QThread::HighPriority);

        app.exec();
    }
    catch (QString str) {
        cout << str.toStdString() << endl;
        return 1;
    }

    return 0;
}

void updateOutput(int index, void *data) {
    auto &d = *static_cast<Data*>(data);
    d.outputVars[index].isNew = true;
}

void validateOutput(int index, bool isValid, void *data) {
    auto &d = *static_cast<Data*>(data);
    d.outputVars[index].isValid = isValid;
    if (d.window!=nullptr) d.window->needUpdate = d.window->needUpdate || isValid;
}

int registerVar(QString name, QString type, void *mem, vector<Variable> &container) {
    container.emplace_back();
    auto& item = container.back();
    item.name = name;
    if (type == "int") item.type = Types::INT;
    else if (type == "double") item.type = Types::DOUBLE;
    else if (type == "intvector") item.type = Types::INTVECTOR;
    else if (type == "doublevector") item.type = Types::DOUBLEVECTOR;
    else throw QString("Unknown input type");
    item.mem = mem;

    return container.size() - 1;
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
    for (auto &var : data.inputVars) {
        int ind = find(input, var.name);
        if (ind == -1) throw QString("No input for ") + var.name;
        auto el = input.at(ind).toElement();
        var.desc = el.attribute("desc");
        var.unit = el.attribute("unit");
        var.isNew = var.isValid = true;
        bool ok;
        auto value = el.firstChild().nodeValue();
        switch (var.type) {
        case Types::INT:
            *static_cast<int*>(var.mem)=value.toInt(&ok);
            break;
        case Types::DOUBLE:
            *static_cast<double*>(var.mem)=value.toDouble(&ok);
            break;
        case Types::INTVECTOR: {
            auto list = value.split(QRegExp("\\s+"));
            auto &vec = *static_cast<vector<int>*>(var.mem);
            for (auto v : list) {
                vec.push_back(v.toInt(&ok));
                if (!ok) break;
            }
            break;}
        case Types::DOUBLEVECTOR: {
            auto list = value.split(QRegExp("\\s+"));
            auto &vec = *static_cast<vector<double>*>(var.mem);
            for (auto v : list) {
                vec.push_back(v.toDouble(&ok));
                if (!ok) break;
            }
            break;}
        }
        if (!ok) throw QString("Error parsing value of ") + var.name + " from " + value;
    }

    auto output = root.elementsByTagName("Output").at(0).childNodes();
    for (auto &var : data.outputVars) {
        int ind = find(output, var.name);
        if (ind == -1) throw QString("No records in ") + filename + " for output " + var.name;
        auto el = output.at(ind).toElement();
        var.desc = el.attribute("desc");
        var.unit = el.attribute("unit");
        var.isNew = var.isValid = false;
    }

    auto findItem = [&data](QString name, bool isOutput) {
        auto vars = &data.inputVars;
        if (isOutput) vars = &data.outputVars;
        for (auto& var : *vars) {
           if (var.name == name) return &var;
        }
        return static_cast<Variable*>(nullptr);
    };

    auto soutputs = root.elementsByTagName("ScreenOutput").at(0).childNodes();
    for (int i = 0; i < soutputs.size(); i++) {
        auto sout = soutputs.at(i);
        data.screenOutputs.emplace_back();
        auto& wgt = data.screenOutputs.back();
        auto name = sout.nodeName();
        if (name == "TextField") wgt.type = ScreenTypes::TEXTFIELD;
        else if (name == "Plot") wgt.type = ScreenTypes::PLOT;
        else throw QString("Unknown ScreenOutput type ") + sout.nodeName();

        auto attrmap = sout.attributes();
        for (int k = 0; k < attrmap.size(); k++) {
            wgt.attributes.insert(attrmap.item(k).nodeName(), attrmap.item(k).nodeValue());
        }

        auto content = sout.childNodes();
        for (int j = 0; j < content.size(); j++) {
            auto rec = content.at(j);
            wgt.items.emplace_back();
            OutputItem &item = wgt.items.back();
            bool isOutput = true;
            if (rec.nodeName() == "InputVar") isOutput = false;
            item.mem = findItem(rec.toElement().attribute("name"), isOutput);
            if (item.mem == nullptr) throw QString("Cannot find variable '") +
                    rec.toElement().attribute("name") + "' for ScreenOutput '" + name + "'";
            attrmap = rec.attributes();
            for (int k = 0; k < attrmap.size(); k++) {
                item.attributes.insert(attrmap.item(k).nodeName(), attrmap.item(k).nodeValue());
            }
        }
    }
}
