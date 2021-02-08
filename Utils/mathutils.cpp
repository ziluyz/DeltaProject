#include <mathutils.h>

using namespace std;

namespace mut {

//Find zero by bisection
bool findRoot(const function<double(double)> &fcn, double xMin, double xMax, double absErr, double &res)
{
    if (xMin >= xMax)
        throw 1; //RuntimeException("xMin greater than xMax in findRoot()");
    double f1 = fcn(xMin);
    double f2 = fcn(xMax);
    if (f1 * f2 > 0)
        return false;
    res = (xMin + xMax) / 2;
    while (xMax - xMin > absErr)
    {
        double fm = fcn(res);
        if (f1 * fm <= 0)
        {
            xMax = res;
            f2 = fm;
        }
        else
        {
            xMin = res;
            f1 = fm;
        }
        res = (xMin + xMax) / 2;
    }
    return true;
}

//Find a sequence of roots
vector<double> findRootsSequence(const function<double(double)> &fcn, double xStart, double minRootDist, int n)
{
    std::vector<double> res;
    double xMin = xStart;
    double d = minRootDist * 0.55;
    double xStep = minRootDist * 0.5;
    for (int i = 0; i < n; i++)
    {
        double a = xMin;
        double b = a + d;
        res.emplace_back();
        while (!findRoot(fcn, a, b, 1e-7, res.back()))
        {
            a += xStep;
            b += xStep;
        }
        xMin = res.back() + xStep;
    }
    return res;
}

void Runge4::vpav(double *v1, double a, vector<double> &v2, vector<double> &res) {
    for (size_t i = 0; i < Neq; i++) res[i] = v1[i] + a * v2[i];
}

Runge4::Runge4(size_t n, function<int(double, const double*, double*, void*)> rights, void *pP) : Neq(n), rights(rights), pP(pP) {
    for (int i = 0; i < 5; i++) kys.emplace_back(Neq);
}

void Runge4::step(double &z0, double z1, double *y) {
    auto zm = (z0 + z1) / 2;
    auto h = z1 - z0;
    auto hd2 = h / 2;
    auto hd6 = h / 6;
    rights(z0, y, kys[1].data(), pP);
    vpav(y, hd2, kys[1], kys[0]);
    rights(zm, kys[0].data(), kys[2].data(), pP);
    vpav(y, hd2, kys[2], kys[0]);
    rights(zm, kys[0].data(), kys[3].data(), pP);
    vpav(y, h, kys[3], kys[0]);
    rights(z1, kys[0].data(), kys[4].data(), pP);
    for (size_t i = 0; i < Neq; i++) y[i] += hd6 * (kys[1][i] + 2 * kys[2][i] + 2 * kys[3][i] + kys[4][i]);
    z0 = z1;
}

}