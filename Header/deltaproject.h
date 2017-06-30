#ifndef _DELTA_P
#define _DELTA_P

#include <vector>

extern int registerInput(const char*, const char*, void* , void*);
extern int registerOutput(const char*, const char*, void* , void*);
extern int execute(int, char**, int (*)(), void*);
extern void updateOutput(int, void*);
extern void validateOutput(int, bool, void*);

void *data = nullptr;

class InputDouble {
    private:
        double val;
        int index;
    public:
        InputDouble(const char *name) {
            index = registerInput(name, "double", &val, &data);
        }
        operator double() {
            return val;
        }
};

class OutputDouble {
    private:
        double val;
        int index;
    public:
        OutputDouble(const char* name) {
            index = registerOutput(name, "double", &val, &data);
        }
        OutputDouble(const OutputDouble&) = delete;
        OutputDouble& operator=(double x) {
            val = x;
            updateOutput(index, data);
            validateOutput(index, true, data);
            return *this;
        }
        OutputDouble& operator=(const OutputDouble &src) {
            val = src.val;
            updateOutput(index, data);
            validateOutput(index, true, data);
            return *this;
        }
};

class OutputDoubleVector {
    private:
        std::vector<double> vec;
        int index;
    public:
        OutputDoubleVector(const char* name) {
            index = registerOutput(name, "doublevector", &vec, &data);
        }
        OutputDoubleVector(const OutputDoubleVector&) = delete;
        OutputDoubleVector& operator=(const OutputDoubleVector&) = delete;
        double& operator[](int i) {return vec[i];}
        void push_back(double x) {
            vec.push_back(x);
            updateOutput(index, data);
        }
        void clear() {vec.clear();}
        int size() {return vec.size();}
        void setValid(bool val = true) {validateOutput(index, val, data);}
};

#endif
