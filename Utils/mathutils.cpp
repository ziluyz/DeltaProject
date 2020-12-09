#include <mathutils.h>

using namespace std;

//Find zero by bisection
bool mut::findRoot(const function<double(double)> &fcn, double xMin, double xMax, double absErr, double &res)
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
vector<double> mut::findRootsSequence(const function<double(double)> &fcn, double xStart, double minRootDist, int n)
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
