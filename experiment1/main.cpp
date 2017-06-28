#include "main.h"

int main(int argc, char** argv) {
    return execute(maincalc, data);
}

int maincalc() {
    double mul = 5*input::b;
    output::a = input::a + input::b;
    output::c = mul;
    for (int i = 0; i < 10; i++) output::r = i * 0.2;
    return 0;
}
