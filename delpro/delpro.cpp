#include "delpro.h"
#include <iostream>
#include <fstream>
#include <gtkmm/application.h>
#include <glibmm/main.h>
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"

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
    d.startTime = Glib::DateTime::create_now_local();
    d.chronoStart = chrono::steady_clock::now();
    d.inputFile = "input.ixml"; // use input.ixml as default
    if (argc > 1) d.inputFile = argv[1]; // if not provided in arguments
    try { // all exceptions should be in the form of std::string
        parseInput(d); // fill Data from ixml

        argc = 1; // command line arguments are handled in main(), not by the Gtk::Application
        auto app = Gtk::Application::create(argc, argv, "ziluyz.gamil.com"); // GTK application

        MainWindow window(&d);
        d.window = &window;
        window.maximize();
        Glib::signal_timeout().connect(sigc::mem_fun(window, &MainWindow::timerEvent), 500);// Timer works as screen updater with 2 Hz frequency

        d.thread = new CalcThread(f, window); // run maincalc
        d.thread->run();

        cout << "Starting execution..." << endl;
        app->run(window); // run execution cycle
        if (d.thread->isRunning()) {
            if (d.termFlag == nullptr) throw 1;
            else if (*d.termFlag == 2) throw 2;
            *d.termFlag = 0;
        }
        delete d.thread;
    }
    catch (string str) {
        cout << str << endl;
        return 1;
    }

    return 0;
}

void updateOutput(int index, void *data) { // mark variable as recently changed
    auto &d = *static_cast<Data*>(data);
    volatile bool &paused = d.paused;
    if (d.termFlag == nullptr) while (paused);
    else {
        volatile int &term = *d.termFlag;
        while (term && paused);
    }
    d.outputVars[index].isNew = true;
}

void validateOutput(int index, bool isValid, void *data) { // mark variable as (in)valid
    auto &d = *static_cast<Data*>(data);
    volatile bool &paused = d.paused;
    if (d.termFlag == nullptr) while (paused);
    else {
        volatile int &term = *d.termFlag;
        while (term && paused);
    }
    d.outputVars[index].isValid = isValid;
    d.window->needUpdate = d.window->needUpdate || isValid; // update global MainWindow flag
}

//Common variable registration function
//Updates corresponding Data containers
int registerVar(string name, string type, void *mem, vector<Variable> &container) {
    container.emplace_back();
    auto& item = container.back();
    item.name = name;
    cout << "..registering " << name << " of " << type << endl;
    if (type == "int") item.type = Types::INT;
    else if (type == "double") item.type = Types::DOUBLE;
    else if (type == "intvector") item.type = Types::INTVECTOR;
    else if (type == "doublevector") item.type = Types::DOUBLEVECTOR;
    else if (type == "doublevectorset") item.type = Types::DOUBLEVECTORSET;
    else if (type == "sysflag") item.type = Types::SYSFLAG;
    else throw string("Unknown input type");
    item.mem = mem;

    return container.size() - 1;
}

//ixml parser
void parseInput(Data &data) {
    rapidxml::file<> mfData(data.inputFile.data());
    rapidxml::xml_document<> doc;
    try {
        doc.parse<0>(mfData.data());
    }
    catch (rapidxml::parse_error err) {
        throw err.what();
    }
    auto proot = doc.first_node("Project");
    if (proot == nullptr) throw string("No Project node in the ixml");
    auto &root = *proot;

    auto find = [](const rapidxml::xml_node<> *node, const string &name) { // find "Var" node wih name in list
        auto el = node->first_node("Var");
        while (el != nullptr) {
            if (el->first_attribute("name")->value() == name) return el; // return node if found
            el = el->next_sibling("Var");
        }
        return el; // return nullptr if not found
    };

    auto strip = [](string &s) -> string & {
        size_t b = 0;
        while (s[b] == ' ' || s[b] == '\t' || s[b] == '\n' || s[b] == '\r') b++;
        s.erase(0, b);
        b = s.size() - 1;
        while (s[b] == ' ' || s[b] == '\t' || s[b] == '\n' || s[b] == '\r') b--;
        s.erase(b + 1, -1);
        return s;
    };

    //------------------Input Section--------------------------------------------------------------
    auto input = root.first_node("Input");
    for (auto &var : data.inputVars) { // find data for every registered input variable
        auto el = find(input, var.name);
        if (el == nullptr) throw string("No input for ") + var.name;
        auto pattr = el->first_attribute("desc");
        var.desc = pattr == nullptr ? "" : pattr->value();
        pattr = el->first_attribute("unit");
        var.unit = pattr == nullptr ? "" : pattr->value();
        var.isNew = var.isValid = true; // inputs are initially new and valid
        string value = el->value();

        //exclude comments placed between % from data
        size_t pos = 0;
        while ((pos = value.find('%')) != string::npos) {
            value.erase(pos, value.find('%', pos + 1) - pos + 1);
        }
        strip(value);

        try {
            switch (var.type)
            {
            case Types::INT:
                *static_cast<int *>(var.mem) = stoi(value);
                break;
            case Types::DOUBLE:
                *static_cast<double *>(var.mem) = stod(value);
                break;
            case Types::INTVECTOR:
            {
                auto list = split(value, " \t\r,");
                auto &vec = *static_cast<vector<int> *>(var.mem);
                for (auto v : list) vec.push_back(stoi(v));
                break;
            }
            case Types::DOUBLEVECTOR:
            {
                auto &vec = *static_cast<vector<double> *>(var.mem);
                auto list = split(value, " \t\r,");
                auto detectFile = split(value, "#");
                if (detectFile.size() > 1)
                {
                    auto col = stoi(strip(detectFile[1]));
                    auto filename = strip(detectFile[0]);
                    fstream file;
                    file.open(filename, ios::in);
                    if (file.is_open()) {
                        list.clear();
                        int line = 0;
                        string ts;
                        while (getline(file, ts)) {
                            line++;
                            if (ts == "")
                                continue;
                            auto cols = split(ts, ",");
                            if (col > cols.size())
                                throw string("Cannot read column ") + to_string(col) + " in line " + to_string(line) + " of " + filename;
                            list.push_back(strip(cols[col - 1]));
                        }
                        file.close();
                        data.filesToSave.insert(filename);
                    }
                    else throw string("Cannot open file ") + filename;
                }
                for (auto v : list)
                {
                    auto parts = split(v, ":");
                    if (parts.size() > 1)
                    {
                        FormulaParser form(strip(parts[1]));
                        auto limits = split(strip(parts[0]), "_");
                        auto imin = stoi(strip(limits[0]));
                        auto imax = stoi(strip(limits[1]));
                        for (int i = imin; i <= imax; i++)
                            vec.push_back(form.eval(i));
                    }
                    else vec.push_back(stod(v));
                }
                break;
            }
            }
        }
        catch (...) {
            throw string("Error parsing value of ") + var.name + " from " + value;
        }
    }

    //------------------Output Section-------------------------------------------------------------
    auto output = root.first_node("Output");
    for (auto &var : data.outputVars) {
        if (var.type == Types::SYSFLAG) {
            if (var.name == "__system_noquit") data.termFlag = reinterpret_cast<int*>(var.mem);
            continue;
        }
        auto el = find(output, var.name);
        if (el == nullptr) throw string("No records in ") + data.inputFile + " for output " + var.name;
        auto pattr = el->first_attribute("desc");
        var.desc = pattr == nullptr ? "" : pattr->value();
        pattr = el->first_attribute("unit");
        var.unit = pattr == nullptr ? "" : pattr->value();
        var.isNew = var.isValid = false;
    }

    //------------------ScreenOutput Section-------------------------------------------------------
    auto findItem = [&data](const string name, bool isOutput) { // find variable in Data by name
        auto vars = &data.inputVars;
        if (isOutput) vars = &data.outputVars;
        for (auto& var : *vars) {
           if (var.name == name) return &var; // return pointer to variable if found
        }
        return static_cast<Variable*>(nullptr); // return nullptr if not found
    };

    auto sout = root.first_node("ScreenOutput")->first_node();
    while (sout != nullptr) {
        data.screenOutputs.emplace_back();
        auto& wgt = data.screenOutputs.back();
        string name = sout->name();
        if (name == "TextField") wgt.type = ScreenTypes::TEXTFIELD;
        else if (name == "Plot") wgt.type = ScreenTypes::PLOT;
        else throw string("Unknown ScreenOutput type ") + name;

        //Reading attributes of ScreenOutput
        auto attr = sout->first_attribute();
        while (attr != nullptr) {
            wgt.attributes[attr->name()] = attr->value();
            attr = attr->next_attribute();
        }

        auto rec = sout->first_node();
        while (rec != nullptr) {
            wgt.items.emplace_back();
            DisplyedItem &item = wgt.items.back();
            bool isOutput = true;
            if (string(rec->name()) == "InputVar") isOutput = false;
            auto pattr = rec->first_attribute("name");
            if (pattr == nullptr) throw string("No name for variable in ") + sout->name();
            item.var = findItem(pattr->value(), isOutput);
            if (item.var == nullptr) throw string("Cannot find variable '") +
                    pattr->value() + "' for ScreenOutput '" + name + "'";
            //Reading attributes of OutputItem
            attr = rec->first_attribute();
            while (attr != nullptr) {
                item.attributes[attr->name()] = attr->value();
                attr = attr->next_attribute();
            }
            rec = rec->next_sibling();
        }
        sout = sout->next_sibling();
    }
}

vector<string> split(const string &s, const string &del) {
    vector<string> res;
    bool mode = false;
    size_t beg = 0;
    for (size_t ind = 0, len = s.size(); ind < len; ind++) {
        bool space = false;
        for (auto c : del) {
            if (c == s[ind]) {
                space = true;
                break;
            }
        }
        if (space && !mode) {
            mode = true;
            if (ind) res.push_back(s.substr(beg, ind - beg));
        }
        else if (!space && mode) {
            mode = false;
            beg = ind;
        }
    }
    if (!mode) res.push_back(s.substr(beg, -1));
    return res;
};

int CalcThread::calc(CalcThread *pthr) {
    pthr->window.calcStarted();
    auto res = pthr->fun();
    pthr->window.calcFinished();
    pthr->finished = true;
    return res;
}

void CalcThread::run() {
    pthread = new thread(calc, this);
}

CalcThread::~CalcThread() {
    if (pthread != nullptr) {
        if (pthread->joinable()) pthread->join();
        delete pthread;
    }
}

double FormulaParser::expression() {
    auto res = term();
    while (peek() == '+' || peek() == '-') {
        if (get() == '+') res += term();
        else res -= term();
    }
    return res;
}

double FormulaParser::term() {
    auto res = factor();
    while (peek() == '*' || peek() == '/') {
        if (get() == '*') res *= factor();
        else res /= factor();
    }
    return res;
}

double FormulaParser::factor() {
    auto res = power();
    if (peek() == '^') {
        get();
        res = pow(res, factor());
    }
    return res;
}

double FormulaParser::power() {
    double res = 0;
    if (peek() >= '0' && peek() <= '9') res = number();
    else if (peek() == '(') {
        get();
        res = expression();
        get();
    }
    else if (peek() >= 'a' && peek() <= 'z') {
        std::string lexem;
        while (peek() >='a' && peek() <= 'z') lexem += get();
        if (lexem == "sin") {
            get();
            res = sin(expression());
            get();
        }
        else if (lexem == "cos") {
            get();
            res = cos(expression());
            get();
        }
        else if (lexem =="sqrt") {
            get();
            res = sqrt(expression());
            get();
        }
        else if (lexem =="exp") {
            get();
            res = exp(expression());
            get();
        }
        else if (lexem =="ln") {
            get();
            res = log(expression());
            get();
        }
        else if (lexem == "pi") res = 3.14159265359;
        else if (lexem == "x") res = x;
    }
    return res;
}

double FormulaParser::number() {
    double res = get() - '0';
    while (peek() >= '0' && peek() <= '9') res = res * 10 + (get() - '0');
    if (peek() == '.') {
        get();
        double exp = 1;
        double frac = 0;
        while (peek() >= '0' && peek() <= '9') {
            frac = frac * 10 + (get() - '0');
            exp *= 10;
        }
        res += frac / exp;
    }
    if (peek() == 'e' || peek() == 'E') {
        get();
        int sgn = 1;
        if (peek() == '-') {
            get();
            sgn = -1;
        }
        int pok = 0;
        while (peek() >= '0' && peek() <= '9') pok = pok * 10 + (get() - '0');
        res = res * pow(10, sgn * pok);
    }
    return res;
}
