#include "main.h"

int main(int argc, char** argv) {
    return execute(maincalc, data);
}

int maincalc() {
    double sum = input::a + input::b;
    double mul = 5*input::b;
    output::a = sum;
    output::c = mul;
    return 0;
}
