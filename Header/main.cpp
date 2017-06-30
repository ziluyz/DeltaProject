#include "main.h"
#include <cmath>

int maincalc() {
    using namespace output;
    traj_x.clear();
    traj_y.clear();
    double x = 0;
    double y = input::h;
    double t = 0;
    traj_x.push_back(x);
    traj_y.push_back(y);
    while (y > 0) {
        t += 0.1;
        x = input::vx * t;
        y = input::h + input::vy*t - 9.8*t*t/2;
        traj_x.push_back(x);
        traj_y.push_back(y);
    }
    traj_x.setValid();
    traj_y.setValid();
    time = t;
    double vfy = input::vy - 9.8*t;
    speed = std::sqrt(input::vx*input::vx + vfy*vfy);
    return 0;
}
