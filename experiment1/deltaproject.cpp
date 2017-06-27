#include <iostream>

int registerInput(const char* name, const char* type, void *var, void *pdata) {
    std::cout << "Registering " << name << std::endl;
    *((double*)var) = 1;
    return 1;
}

int registerOutput(const char* name, const char* type, void *var, void *pdata) {
    std::cout << "Registering output " << name << std::endl;
    return 2;
}

int execute(int (*f)(), void* data) {
    return f();
}

void updateOutput(int index, void *data) {
    std::cout << "Updating " << index << std::endl;
}
