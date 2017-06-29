#include "main.h"

int main(int argc, char** argv) {
    return execute(argc, argv, maincalc, data);
}

int maincalc() {
    using namespace output;
    a = input::a + input::b;
    for (int i = 0; i < 10; i++) r.push_back(i * 0.2);
    r.setValid();
    c = r[3];
    return 0;
}
