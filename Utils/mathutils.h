#include <functional>
#include <vector>

class mut {
public:

//Find zero by bisections
static bool findRoot(const std::function<double(double)> &fcn, double xMin, double xMax, double absErr, double &res);

//Find a sequence of roots
static std::vector<double> findRootsSequence(const std::function<double(double)> &fcn, double xStart, double minRootDist, int n);

};