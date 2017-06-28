#ifndef DELPRO_H
#define DELPRO_H

#include "delpro_global.h"

#include <string>
#include <vector>

enum class Types {INT, DOUBLE, INTVECTOR, DOUBLEVECTOR};

using namespace std;

struct Data {
    vector<string> inputNames;
    vector<void*> inputVars;
    vector<Types> inputTypes;
    vector<string> outputNames;
    vector<void*> outputVars;
    vector<Types> outputTypes;
};

int registerInput(const char* name, const char* type, void *var, void *pdata);
int registerOutput(const char* name, const char* type, void *var, void *pdata);
int execute(int (*f)(), void* data);
void updateOutput(int index, void *data);

#endif // DELPRO_H
