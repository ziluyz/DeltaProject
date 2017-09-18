#ifndef _DELTA_P
#define _DELTA_P

#include <vector>
#include <string>
#include <iostream>

extern int registerInput(const char*, const char*, void* , void*);
extern int registerOutput(const char*, const char*, void* , void*);
extern int execute(int, char**, int (*)(), void*);
extern void updateOutput(int, void*);
extern void validateOutput(int, bool, void*);

int maincalc();

void *data = nullptr;

class RuntimeException : public std::string {
    public:
        RuntimeException(const char* str) : std::string(str) {};
};


int main(int argc, char** argv) {
    return execute(argc, argv, maincalc, data);
}

template<class T>
class InputNumber {
    private:
        T val;
        int index;
    public:
        InputNumber(const char *name) {throw 1;}
        operator T() {
            return val;
        }
};

template<>
InputNumber<int>::InputNumber(const char *name) {
    index = registerInput(name, "int", &val, &data);
}

template<>
InputNumber<double>::InputNumber(const char *name) {
    index = registerInput(name, "double", &val, &data);
}

typedef InputNumber<int> InputInt;
typedef InputNumber<double> InputDouble;

template<class T>
class InputNumberVector {
    private:
        std::vector<T> vec;
        int index;
    public:
        InputNumberVector(const char* name) {throw 1;}
        T operator[](unsigned long i) {return vec[i];}
        unsigned long size() {return vec.size();}
        T back() {return vec.back();}
};

template<>
InputNumberVector<int>::InputNumberVector(const char *name) {
    index = registerInput(name, "intvector", &vec, &data);
}

template<>
InputNumberVector<double>::InputNumberVector(const char *name) {
    index = registerInput(name, "doublevector", &vec, &data);
}

typedef InputNumberVector<int> InputIntVector;
typedef InputNumberVector<double> InputDoubleVector;

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
        operator double() {return val;}
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
        double& operator[](unsigned long i) {
            updateOutput(index, ::data);
            return vec[i];
        }
        void push_back(double x) {
            vec.push_back(x);
            updateOutput(index, ::data);
        }
        double& back() {return vec.back();}
        void clear() {vec.clear();}
        unsigned long size() {return vec.size();}
        void resize(int n) {vec.resize(n);}
        void setValid(bool val = true) {validateOutput(index, val, ::data);}
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
        void resize(int n) {for (auto v : vecs) v->resize(n);}
};

void submeshInputVectors(std::vector<InputDoubleVector*> x0s, std::vector<InputDoubleVector*> y0s,
        double dx, std::vector<double> &x, std::vector<std::vector<double>*> ys,
        std::vector<std::vector<double>*> vys) {
    using namespace std;
    struct Channel {
        InputDoubleVector *x0;
        InputDoubleVector *y0;
        unsigned long index;
        vector<double> *y;
        vector<double> *vy;
        double tx, ty, tvy;
    };
    vector<Channel> chs;
    double tx = (*x0s[0])[0];
    x.clear();

	for (unsigned long i = 0; i < x0s.size(); i++) chs.push_back(Channel{x0s[i], y0s[i],
            0, ys[i], vys[i]});

    do {
        double nx = (*chs[0].x0)[chs[0].index + 1];
        for (auto &ch : chs) {
            while ((*ch.x0)[ch.index] <= tx) ch.index++;
            double ny = (*ch.y0)[ch.index--];
            ch.ty = (*ch.y0)[ch.index];
            ch.tx = (*ch.x0)[ch.index];
            ch.tvy = (ny - ch.ty) / ((*ch.x0)[ch.index + 1] - ch.tx);
            if ((*ch.x0)[ch.index + 1] < nx) nx = (*ch.x0)[ch.index + 1];
        }
        int np = (nx - tx) / dx + 1;
        double rdx = (nx - tx) / np;
        for (int i = 0; i < np; i++) {
            x.push_back(tx + rdx * i);
            for (auto &ch : chs) {
                (*ch.y).push_back(ch.ty + ch.tvy * (x.back() - ch.tx));
                if (ch.vy != nullptr) (*ch.vy).push_back(ch.tvy);
            }
        }
        x.push_back(nx);
        for (auto &ch : chs) (*ch.y).push_back(ch.ty + ch.tvy * (x.back() - ch.tx));
        tx = nx;
    }
    while (tx < (*chs[0].x0).back());
}

#endif
