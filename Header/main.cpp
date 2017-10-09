#include "main.h"
#include <cmath>

int maincalc() {
    using namespace output;
    using namespace std;
    auto arrs = OutputVectorCollection(traj_x, traj_y);
    traj_y.setNumberOfVectors(2);
    output::time = 0;
    double scale = 1;
    vector<double> p2, vp2;
    p2.push_back(0);
    vp2.push_back(0);
    for (auto v : input::probe) p2.push_back(v + 0.3);
    for (auto v : input::probeV) vp2.push_back(2 * v);
    std::vector<double> X, Y1, Y2, dY1, dY2;
    submeshInputVectors(std::vector<std::vector<double>*>{&input::probe, &p2}, 
            std::vector<std::vector<double>*>{&input::probeV, &vp2}, 0.5,
            X, std::vector<std::vector<double>*>{&Y1, &Y2},
            std::vector<std::vector<double>*>{&dY1, &dY2});
    for (auto v : X) std::cout << v << std::endl;
    std::cout << std::endl;
    for (auto v : Y1) std::cout << v << std::endl;
    std::cout << std::endl;
    for (auto v : Y2) std::cout << v << std::endl;
    std::cout << std::endl;
    for (auto v : dY1) std::cout << v << std::endl;
    std::cout << std::endl;
    for (auto v : dY2) std::cout << v << std::endl;
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
