#include "main.h"
#include <cmath>

int maincalc() {
    using namespace std;
    delta::OutputVectorCollection vout(output::z, output::A, output::B, output::C);
    vout.setValid(false);
    vector<double> z, A, B, C, vB;
    delta::submeshVectors({&input::zA, &input::zB, &input::zC},
            {&input::A, &input::B, &input::C}, input::dz,
            z, {&A, &B, &C}, {&vB, nullptr, nullptr});
    for (auto t : z) output::z.push_back(t);
    for (auto t : A) output::A.push_back(t);
    for (auto t : B) output::B.push_back(t);
    for (auto t : vB) output::C.push_back(t);
    vout.setValid();
    return 0;
}
