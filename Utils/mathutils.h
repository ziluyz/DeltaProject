#include <functional>
#include <vector>

namespace mut {

//Find zero by bisections
bool findRoot(const std::function<double(double)> &fcn, double xMin, double xMax, double absErr, double &res);

//Find a sequence of roots
std::vector<double> findRootsSequence(const std::function<double(double)> &fcn, double xStart, double minRootDist, int n);

class Runge4 {
    private:
        std::vector<std::vector<double>> kys;
        size_t Neq;
        const std::function<int(double, const double*, double*, void*)> rights;
        void *pP;
        void vpav(double *, double, std::vector<double> &, std::vector<double> &);
    public:
        Runge4(size_t n, std::function<int(double, const double*, double*, void*)> rights, void *pP);
        void step(double &z0, double z1, double *y);
};

}