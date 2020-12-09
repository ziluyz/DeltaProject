#ifndef DELPRO_H
#define DELPRO_H

#include <memory>
#include "mainwindow.h"
#include "wgt.h"
#include <chrono>
#include <string>
#include <glibmm/datetime.h>
#include <unordered_map>
#include <set>
#include <thread>

//interface to libdelpro.so
//-------------------------
//Register input variable
//var is a pointer to variable data
//pdata is pointer to pointer to Data communication block (inner .so structure, see below)
//returns the index of registered variable in the inner .so structure
int registerInput(const char* name, const char* type, void *var, void *pdata);
//Register output variable
int registerOutput(const char* name, const char* type, void *var, void *pdata);
//Begin execution cycle
//f is pointer to maincalc
//data is pointer to Data block
int execute(int argc, char** argv, int (*f)(), void* data);
//Register changes in output variables
//index is the one provided during registration
void updateOutput(int index, void *data);
//Report wether an output variable is valid
void validateOutput(int index, bool isValid, void *data);

//Available types
//-----------------
//Types of variables
enum class Types {INT, DOUBLE, INTVECTOR, DOUBLEVECTOR, DOUBLEVECTORSET, SYSFLAG};
//Types of screen output widgets
enum class ScreenTypes {TEXTFIELD, PLOT};

using namespace std;

class Wgt; // Abstract class of output widget
class MainWindow; // Main window class

//Inner structure of input and output variables
struct Variable {
    string name; // provided by calling program via register(In)(Out)put
    void *mem; // pointer to real variable data, provided by calling program
    Types type; // one of available
    string desc; // verbal description, provided by input ixml
    string unit; // variable physical units, provided by input ixml
    bool isNew; // shows if data were recently changed, controlled by updateOutput
    bool isValid; // shows if data are valid, controlled by validateOutput
    bool needUpdate; // shows if data is waiting for screen update, controlled by MainWindow timer event
    vector<shared_ptr<Wgt>> wgts; // all widgets showing this variable
};

//Item of the output widget along with its attributes as is got from input ixml
struct DisplyedItem {
    Variable *var;
    unordered_map<string,string> attributes;
};

//Record for output widget as is got from input ixml
struct ScreenOutput {
    ScreenTypes type;
    unordered_map<string,string> attributes;
    vector<DisplyedItem> items;
};

//Thread for maincalc
class CalcThread
{
private:
    int (*fun)();
    bool finished;
    thread *pthread;
    MainWindow &window;
    static int calc(CalcThread *);
public:
    CalcThread(int (*f)(), MainWindow &win) : fun(f), finished(false), window(win) {}; // f should point to maincalc
    ~CalcThread();
    void run();
    bool isRunning() {return !finished;};
};

//Main data block
//used to communicate between libdelpro.so and calling program when registering and updating variables
//initialized by the first variable registering
struct Data {
    MainWindow *window;
    CalcThread *thread;
    Glib::DateTime startTime; // used to name folder when saving data
    chrono::time_point<chrono::steady_clock> chronoStart; // used for elapsed time calculation;
    chrono::time_point<chrono::steady_clock> chronoEnd;
    string inputFile; // name of input ixml
    set<string> filesToSave; // processed input ixml
    vector<Variable> inputVars;
    vector<Variable> outputVars;
    vector<ScreenOutput> screenOutputs;
    bool paused = false;
    int *termFlag = nullptr;
};

//Inner functions
//---------------
//Common variable registrator
int registerVar(string name, string type, void *mem, vector<Variable> &container);
//ixml parser
void parseInput(Data &data);
//split string
vector<string> split(const string&, const string&);
//Formula parser
class FormulaParser {
    private:
        std::string formula;
        size_t index;
        char peek() {
            while (formula[index] == ' ') index++;
            return formula[index];
        }
        char get() {
            while (formula[index] == ' ') index++;
            return formula[index++];
        }
        double number();
        double power();
        double factor();
        double term();
        double expression();
        double x;
    public:
        FormulaParser(std::string f) : formula(f) {}
        double eval(double x) {
            index = 0;
            this->x = x;
            return expression();
        }
};

#endif // DELPRO_H
