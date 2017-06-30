# DeltaProject
Shell and libraries for scientific simulations

The goal is to work out an easy-to-create framework for writing C++ code for scientific simulations. Suppose it is necessary to model the motion in the uniform gravity field. We want the corresponding realization be as short and straightforward as possible. Ideally, it should look something like the following (0-approximation):

*sim.h*:
```c++
#include <necessary_headers>

namespace input {
  double height; // initial height
  double vx; // initial horizontal velocity
  double vy; // initial vertical velocity
}

namespace output {
  double speed; // speed at hitting the ground
  double time; // time of motion before hitting
  std::vector<std::pair<double,double>> trajectory; // trajectory
}
```

*sim.cpp*:
```c++
#include "sim.h"
#include <necessary_headers>

int maincalc() {
  output::trajectory.clear();
  double x = 0;
  double y = input::height;
  double t = 0;
  output::trajectory.push_back(std::pair<double,double>(x, y));
  while (y > 0) {
    t += 0.1;
    x = input::vx * t;
    y = input::height + input::vy*t - 9.8*t*t/2;
    output::trajectory.push_back(std::pair<double,double>(x, y));
  }
  output::time = t;
  double vfy = input::vy - 9.8*t;
  output::speed = std::sqrt(input::vx*input::vx + vfy*vfy);
  return 0; // normal termination
}
```

That is, the code writer should know nothing about of how the code is fed with the input data (`height`, `vx` and `vy` initialization) or how the output is processed (it may be saved to the datafiles or output to the screen or both or enything else). All this work lies upon the developed shell and libraries. The above code should be compiled once and shouldn't require recompilation for changing the input values of representation of the output.
