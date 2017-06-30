#ifndef DELPRO_H
#define DELPRO_H

#include "delpro_global.h"
#include <QThread>
#include <QtWidgets>
#include "mainwindow.h"

int registerInput(const char* name, const char* type, void *var, void *pdata);
int registerOutput(const char* name, const char* type, void *var, void *pdata);
int execute(int argc, char** argv, int (*f)(), void* data);
void updateOutput(int index, void *data);
void validateOutput(int index, bool isValid, void *data);

enum class Types {INT, DOUBLE, INTVECTOR, DOUBLEVECTOR};
enum class ScreenTypes {TEXTFIELD, PLOT};

using namespace std;

struct OutputItem {
    bool isOutput;
    int index;
    QHash<QString,QString> attributes;
};

struct ScreenOutput {
    ScreenTypes type;
    QHash<QString,QString> attributes;
    vector<OutputItem> items;
};

class MainWindow;

struct Data {
    MainWindow *window;

    vector<QString> inputNames;
    vector<void*> inputVars;
    vector<Types> inputTypes;
    vector<QString> inputDescriptions;
    vector<QString> inputUnits;

    vector<QString> outputNames;
    vector<void*> outputVars;
    vector<Types> outputTypes;
    vector<QString> outputDescriptions;
    vector<QString> outputUnits;
    vector<bool> outputIsNew;
    vector<bool> outputIsValid;

    vector<ScreenOutput> screenOutputs;
};

class CalcThread : public QThread
{
private:
    int (*fun)();
public:
    CalcThread(int (*f)()) : QThread() {fun = f;}
    void run() override {fun();}
};

void parseInput(QString &filename, Data &data);

#endif // DELPRO_H
