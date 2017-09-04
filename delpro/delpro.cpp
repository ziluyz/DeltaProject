#include "delpro.h"
#include <iostream>
#include <QApplication>

using namespace std;

int registerInput(const char* name, const char* type, void *var, void *pdata) { // register input variable
    auto *&data = *static_cast<Data**>(pdata);
    if (data == nullptr) data = new Data; // initialize Data by first call
    return registerVar(name, type, var, data->inputVars);
}

int registerOutput(const char* name, const char* type, void *var, void *pdata) { // register output variable
    auto *&data = *static_cast<Data**>(pdata);
    if (data == nullptr) data = new Data; // initialize Data by first call
    return registerVar(name, type, var, data->outputVars);
}

int execute(int argc, char** argv, int (*f)(), void* data) { // begin main execution cycle
    auto &d = *static_cast<Data*>(data);
    d.startTime = QDateTime::currentDateTime();
    d.inputFile = "input.ixml"; // use input.ixml as default
    if (argc > 1) d.inputFile = argv[1]; // if not provided in arguments
    try { // all exceptions should be in the form of QString
        parseInput(d); // fill Data from ixml

        QApplication app(argc, argv); // Qt application

        MainWindow window(&d);
        d.window = &window;
        window.showMaximized();
        window.startTimer(500); // Timer works as screen updater with 2 Hz frequency

        CalcThread thread(f); // run maincalc
        thread.start(QThread::HighPriority);

        app.exec(); // run Qt execution cycle
    }
    catch (QString str) {
        cout << str.toStdString() << endl;
        return 1;
    }

    return 0;
}

void updateOutput(int index, void *data) { // mark variable as recently changed
    auto &d = *static_cast<Data*>(data);
    d.outputVars[index].isNew = true;
}

void validateOutput(int index, bool isValid, void *data) { // mark variable as (in)valid
    auto &d = *static_cast<Data*>(data);
    d.outputVars[index].isValid = isValid;
    d.window->needUpdate = d.window->needUpdate || isValid; // update global MainWindow flag
}

//Common variable registration function
//Updates corresponding Data containers
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

//ixml parser
void parseInput(Data &data) {
    QFile file(data.inputFile);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) throw QString("Error opening input file");
    QDomDocument &doc = data.inputIXML;
    QString parsError;
    int errorLine;
    if (!doc.setContent(&file, false, &parsError, &errorLine)) {
        file.close();
        throw QString("Error parsing input file\n") + parsError + "\nLine: " + QString::number(errorLine);
    }
    file.close();

    //Processing all imports
    bool imported;
    do {
        auto list = doc.elementsByTagName("import");
        int len = list.size();
        imported = false;
        for (int i = 0; i < len; i++) {
            imported = true;
            auto el = list.at(i).toElement();
            auto par = el.parentNode();

            QString filename = el.attribute("filename","");
            file.setFileName(filename);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) throw QString("Error opening import file " + filename);

            QDomDocument idoc("to_import");
            if (!idoc.setContent(&file, false, &parsError, &errorLine)) {
                file.close();
                throw QString("Error parsing import file\n") + parsError + "\nLine: " + QString::number(errorLine);
            }
            file.close();
            auto iroot = idoc.documentElement();
            auto iel = iroot.firstChild();
            while (!iel.isNull()) {
                auto nel = iel.nextSibling();
                par.insertBefore(iel, el);
                iel = nel;
            }
            par.removeChild(el);
        }
    }
    while (imported);

    auto root = doc.documentElement();

    auto find = [](QDomNodeList &list, QString &name) { // find "Var" node wih name in list
        for (int i = 0, len = list.size(); i < len; i++) {
            auto node = list.at(i);
            if (node.nodeName() != "Var") continue;
            if (!node.toElement().hasAttribute("name")) continue;
            if (node.toElement().attribute("name") == name) return i; // return index if found
        }
        return -1; // return -1 if not found
    };

    //------------------Input Section--------------------------------------------------------------
    auto input = root.elementsByTagName("Input").at(0).childNodes();
    for (auto &var : data.inputVars) { // find data for every registered input variable
        int ind = find(input, var.name);
        if (ind == -1) throw QString("No input for ") + var.name;
        auto el = input.at(ind).toElement();
        var.desc = el.attribute("desc");
        var.unit = el.attribute("unit");
        var.isNew = var.isValid = true; // inputs are initially new and valid
        bool ok;
        auto value = el.firstChild().nodeValue();

        //exclude comments placed between % from data
        auto comlist = value.split("%");
        value.clear();
        bool incl = true;
        for (auto el : comlist) {
            if (incl) value = value + el;
            incl = !incl;
        }

        switch (var.type) {
        case Types::INT:
            *static_cast<int*>(var.mem)=value.toInt(&ok);
            break;
        case Types::DOUBLE:
            *static_cast<double*>(var.mem)=value.toDouble(&ok);
            break;
        case Types::INTVECTOR: {
            auto list = value.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            auto &vec = *static_cast<vector<int>*>(var.mem);
            for (auto v : list) {
                vec.push_back(v.toInt(&ok));
                if (!ok) break;
            }
            break;}
        case Types::DOUBLEVECTOR: {
            auto list = value.split(QRegExp("\\s+"), QString::SkipEmptyParts);
            auto &vec = *static_cast<vector<double>*>(var.mem);
            for (auto v : list) {
                vec.push_back(v.toDouble(&ok));
                if (!ok) break;
            }
            break;}
        }
        if (!ok) throw QString("Error parsing value of ") + var.name + " from " + value;
    }

    //------------------Output Section-------------------------------------------------------------
    auto output = root.elementsByTagName("Output").at(0).childNodes();
    for (auto &var : data.outputVars) {
        int ind = find(output, var.name);
        if (ind == -1) throw QString("No records in ") + data.inputFile + " for output " + var.name;
        auto el = output.at(ind).toElement();
        var.desc = el.attribute("desc");
        var.unit = el.attribute("unit");
        var.isNew = var.isValid = false;
    }

    //------------------ScreenOutput Section-------------------------------------------------------
    auto findItem = [&data](QString name, bool isOutput) { // find variable in Data by name
        auto vars = &data.inputVars;
        if (isOutput) vars = &data.outputVars;
        for (auto& var : *vars) {
           if (var.name == name) return &var; // return pointer to variable if found
        }
        return static_cast<Variable*>(nullptr); // return nullptr if not found
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

        //Reading attributes of ScreenOutput
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
            item.var = findItem(rec.toElement().attribute("name"), isOutput);
            if (item.var == nullptr) throw QString("Cannot find variable '") +
                    rec.toElement().attribute("name") + "' for ScreenOutput '" + name + "'";
            //Reading attributes of OutputItem
            attrmap = rec.attributes();
            for (int k = 0; k < attrmap.size(); k++) {
                item.attributes.insert(attrmap.item(k).nodeName(), attrmap.item(k).nodeValue());
            }
        }
    }
}
