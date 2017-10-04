#include "deltaproject.h"

namespace input {
    InputDouble h("h");
    InputDouble vx("vx");
    InputDouble vy("vy");
}

namespace output {
    OutputDouble speed("speed");
    OutputDouble time("time");
    OutputDoubleVector traj_x("traj_x");
    OutputDoubleVectorSet traj_y("traj_y");
}
