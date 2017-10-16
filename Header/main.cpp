#include "main.h"
#include <cmath>

int maincalc() {
    using namespace output;
    using namespace std;
    auto arrs = delta::OutputVectorCollection(traj_x, traj_y);
    traj_y.setNumberOfVectors(2);
    output::time = 0;
    double scale = 1;

    vector<double> v;
    delta::Bundle b(input::probe, input::probeV);
    for (auto m : b) v.push_back(input::probe[m] + input::probeV[m]);
    for (auto m : b - 1) cout << m << " : " << input::probe[m] << " + " << input::probeV[m] << " = " << v[m] << endl;

    while (output::time < 100) {
        double vy = input::vy * scale;
        arrs.setValid(false);
        arrs.clear();
        double x = 0;
        double y = input::h;
        double t = 0;
        traj_x.push_back(x);
        traj_y.pushZero();
        traj_y.set_back(0, y);
        while (y > 0) {
            t += 0.1;
            x = input::vx * t;
            y = input::h + vy*t - 9.8*t*t/2;
            traj_x.push_back(x);
            traj_y.pushZero();
            traj_y.set_back(0, y);
            traj_y.set_back(1, y * 2);
        }
        arrs.setValid();
        output::time = t;
        double vfy = vy - 9.8*t;
        speed = std::sqrt(input::vx*input::vx + vfy*vfy);
        scale *= 1.0001;
    }
    return 0;
}
