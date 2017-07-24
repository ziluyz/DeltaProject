#ifndef _DELTA_P
#define _DELTA_P

#include <vector>

extern int registerInput(const char*, const char*, void* , void*);
extern int registerOutput(const char*, const char*, void* , void*);
extern int execute(int, char**, int (*)(), void*);
extern void updateOutput(int, void*);
extern void validateOutput(int, bool, void*);

void *data = nullptr;

class InputInt {
    private:
        double val;
        int index;
    public:
        InputInt(const char *name) {
            index = registerInput(name, "int", &val, &data);
        }
        operator double() {
            return val;
        }
};

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

class InputIntVector {
    private:
        std::vector<int> vec;
        int index;
    public:
        InputIntVector(const char* name) {
            index = registerInput(name, "intvector", &vec, &data);
        }
        int operator[](int i) {return vec[i];}
        int size() {return vec.size();}
};

class InputDoubleVector {
    private:
        std::vector<double> vec;
        int index;
    public:
        InputDoubleVector(const char* name) {
            index = registerInput(name, "doublevector", &vec, &data);
        }
        int operator[](int i) {return vec[i];}
        int size() {return vec.size();}
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

class OutputVectorCollection {
    private:
        std::vector<OutputDoubleVector*> vecs;
        void addVector() {}
        template<class... T>
        void addVector(OutputDoubleVector &fv, T&... vectors) {
            vecs.push_back(&fv);
            addVector(vectors...);
        }
    public:
        template<class... T>
        OutputVectorCollection(OutputDoubleVector &fv, T&... vectors) {addVector(fv, vectors...);}
        void clear() {for (auto v : vecs) v->clear();}
        void setValid(bool val = true) {for (auto v : vecs) v->setValid(val);}
};

#endif
