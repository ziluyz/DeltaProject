#ifndef DELPRO_H
#define DELPRO_H

#include "delpro_global.h"
#include <QThread>
#include <QtWidgets>
#include "mainwindow.h"

#include <string>
#include <vector>

int registerInput(const char* name, const char* type, void *var, void *pdata);
int registerOutput(const char* name, const char* type, void *var, void *pdata);
int execute(int argc, char** argv, int (*f)(), void* data);
void updateOutput(int index, void *data);
void validateOutput(int index, bool isValid, void *data);

enum class Types {INT, DOUBLE, INTVECTOR, DOUBLEVECTOR};

using namespace std;

class MainWindow;

struct Data {
    MainWindow *window;
    vector<QString> inputNames;
    vector<void*> inputVars;
    vector<Types> inputTypes;
    vector<QString> outputNames;
    vector<void*> outputVars;
    vector<Types> outputTypes;
    vector<bool> outputIsNew;
    vector<bool> outputIsValid;
};

class CalcThread : public QThread
{
private:
    int (*fun)();
public:
    CalcThread(int (*f)()) : QThread() {fun = f;}
    void run() override {fun();}
};

#endif // DELPRO_H
