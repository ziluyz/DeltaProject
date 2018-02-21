#ifndef _DELTA_P
#define _DELTA_P
#define V(name) name(#name);

#include <vector>
#include <string>
#include <iostream>
#include <cmath>

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
        RuntimeException(const std::string &str) : std::string(str) {};
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
class InputNumberVector : public std::vector<T> {
    private:
        int index;
    public:
        InputNumberVector(const char* name) {throw 1;}
        InputNumberVector(const InputNumberVector&) = delete;
        InputNumberVector& operator=(const InputNumberVector&) = delete;
        T operator[](size_t i) {return std::vector<T>::operator[](i);}
};

template<>
InputNumberVector<int>::InputNumberVector(const char *name) {
    index = registerInput(name, "intvector", this, &::data);
}

template<>
InputNumberVector<double>::InputNumberVector(const char *name) {
    index = registerInput(name, "doublevector", this, &::data);
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

class OutputMultiValue {
    public:
        virtual void clear() = 0;
        virtual void setValid(bool val = true) = 0;
        virtual void resize(size_t n) = 0;
};

class OutputDoubleVector : public OutputMultiValue {
    private:
        std::vector<double> vec;
        int index;
    public:
        OutputDoubleVector(const char* name) {
            index = registerOutput(name, "doublevector", &vec, &data);
        }
        OutputDoubleVector(const OutputDoubleVector&) = delete;
        OutputDoubleVector& operator=(const OutputDoubleVector&) = delete;
        void set(size_t ind, double val) {
            vec[ind] = val;
            updateOutput(index, data);
        }
        double get(size_t ind) {return vec[ind];}
        void push_back(double x) {
            vec.push_back(x);
            updateOutput(index, data);
        }
        double back() {return vec.back();}
        void clear() override {vec.clear();}
        unsigned long size() {return vec.size();}
        void resize(size_t n) override {vec.resize(n);}
        void setValid(bool val = true) override {validateOutput(index, val, data);}
};

class OutputDoubleVectorSet : public OutputMultiValue {
    private:
        std::vector<std::vector<double>> vvec;
        int index;
    public:
        OutputDoubleVectorSet(const char* name) {
            index = registerOutput(name, "doublevectorset", &vvec, &data);
        }
        OutputDoubleVectorSet(const OutputDoubleVectorSet&) = delete;
        OutputDoubleVectorSet& operator=(const OutputDoubleVectorSet&) = delete;
        void setNumberOfVectors(size_t n) {vvec.resize(n);}
        void set(size_t vec, size_t ind, double val) {
            vvec[vec][ind] = val;
            updateOutput(index, data);
        }
        double get(size_t vec, size_t ind) {return vvec[vec][ind];}
        void pushZero() {for (auto &vec : vvec) vec.push_back(0);}
        void set_back(size_t vec, double val) {
            vvec[vec].back() = val;
            updateOutput(index, data);
        }
        double back(size_t vec) {return vvec[vec].back();}
        void clear() override {for (auto &vec : vvec) vec.clear();}
        void resize(size_t n) override {for (auto &vec : vvec) vec.resize(n);}
        void setValid(bool val = true) override {validateOutput(index, val, data);}
};

namespace delta {

class OutputVectorCollection : public OutputMultiValue {
    private:
        std::vector<OutputMultiValue*> vecs;
        void addVector() {}
        template<class... T>
        void addVector(OutputMultiValue &fv, T&... vectors) {
            vecs.push_back(&fv);
            addVector(vectors...);
        }
    public:
        template<class... T>
        OutputVectorCollection(T&... vectors) {addVector(vectors...);}
        void clear() override {for (auto v : vecs) v->clear();}
        void setValid(bool val = true) override {for (auto v : vecs) v->setValid(val);}
        void resize(size_t n) override {for (auto v : vecs) v->resize(n);}
};

class BundleIterator;

class Bundle {
    friend BundleIterator begin(const Bundle&);
    friend BundleIterator end(const Bundle&);
    private:
        size_t len;
        void addContainer() {}
        template <class R, class... T>
            void addContainer(R &c, T&... cs) {
                if (len) {
                    if (len != c.size()) throw RuntimeException("Not equal vector sizes in Bundle");
                }
                else {
                    len = c.size();
                }
                addContainer(cs...);
            }
    public:
        template <class... T>
            Bundle(T&... cs) : len(0) {addContainer(cs...);}
        Bundle operator-(size_t dec) {
            Bundle b;
            b.len = len - dec;
            return b;
        }
};

class BundleIterator {
    private:
        size_t index;
    public:
        BundleIterator(size_t ind) : index(ind) {}
        BundleIterator operator*() {return *this;}
        operator size_t() {return index;}
        bool operator !=(BundleIterator &it) {return (index != it.index);}
        void operator ++() {index++;}
};

BundleIterator begin(const Bundle &b) {
    return BundleIterator(0);
}

BundleIterator end(const Bundle &b) {
    return BundleIterator(b.len);
}

void submeshVectors(std::vector<std::vector<double>*> x0s, std::vector<std::vector<double>*> y0s,
        double dx, std::vector<double> &x, std::vector<std::vector<double>*> ys,
        std::vector<std::vector<double>*> vys) {
    using namespace std;
    struct Channel {
        vector<double> *x0;
        vector<double> *y0;
        unsigned long index;
        vector<double> *y;
        vector<double> *vy;
        double tx, ty, tvy;
        double steptvy;
    };
    vector<Channel> chs;
    double tx = (*x0s[0])[0];
    x.clear();

	for (size_t i = 0; i < x0s.size(); i++) chs.push_back(Channel{x0s[i], y0s[i],
            0, ys[i], vys[i]});

    do {
        double nx = (*chs[0].x0).back();
        for (auto &ch : chs) {
            while ((*ch.x0)[ch.index] <= tx) ch.index++;
            double ny = (*ch.y0)[ch.index--];
            ch.ty = (*ch.y0)[ch.index];
            ch.tx = (*ch.x0)[ch.index];
            ch.tvy = (ny - ch.ty) / ((*ch.x0)[ch.index + 1] - ch.tx);
            if ((*ch.x0)[ch.index + 1] < nx) nx = (*ch.x0)[ch.index + 1];
        }
        int np = (nx - tx) / (dx * 1.00001) + 1;
        double rdx = (nx - tx) / np;
        for (int i = 0; i < np; i++) {
            x.push_back(tx + rdx * i);
            for (auto &ch : chs) {
                (*ch.y).push_back(ch.ty + ch.tvy * (x.back() - ch.tx));
                if (ch.vy != nullptr) (*ch.vy).push_back(ch.tvy);
            }
        }
        tx = nx;
        bool step = false;
        for (auto &ch : chs) {
            ch.steptvy = ch.tvy;
            if (ch.index < ch.x0->size() - 2 && abs((*ch.x0)[ch.index + 2] - tx) < 1e-7 &&
                    abs((*ch.y0)[ch.index + 2] - (*ch.y0)[ch.index + 1]) > 1e-7) {
                step = true;
                ch.steptvy = INFINITY;
            }
        }
        if (step) {
            x.push_back(tx);
            for (auto &ch : chs) {
                (*ch.y).push_back(ch.ty + ch.tvy * (tx - ch.tx));
                if (ch.vy != nullptr) (*ch.vy).push_back(ch.steptvy);
            }
        }
    }
    while (tx < (*chs[0].x0).back());
    x.push_back(tx);
    for (auto &ch : chs) (*ch.y).push_back(ch.ty + ch.tvy * (x.back() - ch.tx));
}

}

#endif
