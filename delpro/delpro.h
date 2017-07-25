#ifndef DELPRO_H
#define DELPRO_H

#include "delpro_global.h"
#include <QThread>
#include <QtWidgets>
#include <memory>
#include "mainwindow.h"
#include "wgt.h"

int registerInput(const char* name, const char* type, void *var, void *pdata);
int registerOutput(const char* name, const char* type, void *var, void *pdata);
int execute(int argc, char** argv, int (*f)(), void* data);
void updateOutput(int index, void *data);
void validateOutput(int index, bool isValid, void *data);

enum class Types {INT, DOUBLE, INTVECTOR, DOUBLEVECTOR};
enum class ScreenTypes {TEXTFIELD, PLOT};

using namespace std;

class Wgt;
class MainWindow;

struct Variable {
    QString name;
    void *mem;
    Types type;
    QString desc;
    QString unit;
    bool isNew;
    bool isValid;
    bool needUpdate;
    vector<shared_ptr<Wgt>> wgts;
    void *supply;
};

struct OutputItem {
    Variable *var;
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
    QDateTime startTime;
    QString inputFile;
    vector<Variable> inputVars;
    vector<Variable> outputVars;
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

int registerVar(QString name, QString type, void *mem, vector<Variable> &container);
void parseInput(Data &data);

#endif // DELPRO_H
