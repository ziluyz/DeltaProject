#include "main.h"
#include <cmath>

int maincalc() {
    using namespace output;
    auto arrs = OutputVectorCollection(traj_x, traj_y);
    time = 0;
    double scale = 1;
    while (time < 100) {
        double vy = input::vy * scale;
        arrs.setValid(false);
        arrs.clear();
        double x = 0;
        double y = input::h;
        double t = 0;
        traj_x.push_back(x);
        traj_y.push_back(y);
        while (y > 0) {
            t += 0.1;
            x = input::vx * t;
            y = input::h + vy*t - 9.8*t*t/2;
            traj_x.push_back(x);
            traj_y.push_back(y);
        }
        arrs.setValid();
        time = t;
        double vfy = vy - 9.8*t;
        speed = std::sqrt(input::vx*input::vx + vfy*vfy);
        scale *= 1.0001;
    }
    return 0;
}
