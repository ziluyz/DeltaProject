#ifndef _DELTA_P
#define _DELTA_P

extern int registerInput(const char*, const char*, void* , void*);
extern int registerOutput(const char*, const char*, void* , void*);
extern int execute(int (*)(), void*);
extern void updateOutput(int, void*);

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
        OutputDouble(double x) {
            val = x;
            updateOutput(index, data);
        }
        OutputDouble(const OutputDouble& src) {
            val = src.val;
            updateOutput(index, data);
        }
};

#endif
