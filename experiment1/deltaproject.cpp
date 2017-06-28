#include <iostream>
#include <string>
#include <vector>

using namespace std;

enum class Types {INT, DOUBLE, INTVECTOR, DOUBLEVECTOR};

struct Data {
    vector<string> inputNames;
    vector<void*> inputVars;
    vector<Types> inputTypes;
    vector<string> outputNames;
    vector<void*> outputVars;
    vector<Types> outputTypes;
};

int registerInput(const char* name, const char* type, void *var, void *pdata) {
    Data *&data = *((Data**)pdata);
    if (data == nullptr) data = new Data;

    data->inputNames.push_back(name);
    string stype(type);
    if (stype == "int") data->inputTypes.push_back(Types::INT);
    else if (stype == "double") data->inputTypes.push_back(Types::DOUBLE);
    else throw string("Unknown input type");
    data->inputVars.push_back(var);

    *((double*)var) = data->inputNames.size();

    return data->inputNames.size() - 1;
}

int registerOutput(const char* name, const char* type, void *var, void *pdata) {
    Data *&data = *((Data**)pdata);
    if (data == nullptr) data = new Data;

    data->outputNames.push_back(name);
    string stype(type);
    if (stype == "int") data->outputTypes.push_back(Types::INT);
    else if (stype == "double") data->outputTypes.push_back(Types::DOUBLE);
    else throw string("Unknown output type");
    data->outputVars.push_back(var);

    return data->outputNames.size() - 1;
}

int execute(int (*f)(), void* data) {
    return f();
}

void updateOutput(int index, void *data) {
    Data &d = *(Data*)data;
    cout << "Printing output: " << d.outputNames[index] << " = " << *(double*)d.outputVars[index] << endl;
}
