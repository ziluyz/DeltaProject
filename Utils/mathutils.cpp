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

bool ArgumentZeroSearcher::BinCoord::operator[](size_t ind) const {
    if (ind < coord.size()) return coord[ind];
    else return coord.back();
}

unsigned int ArgumentZeroSearcher::BinCoord::hash() const {
    unsigned int res = 0;
    for (size_t i = coord.size() - 1; i < SIZE_MAX; i--) res = res * 2 + coord[i];
    return res;
}

int ArgumentZeroSearcher::BinCoord::compare(const BinCoord &c1, const BinCoord &c2) {
    auto len = max(c1.coord.size(), c2.coord.size());
    for (size_t i = 0; i < len; i++) {
        if (c1[i] > c2[i]) return 1;
        else if (c1[i] < c2[i]) return -1;
    }
    return 0;
}

ArgumentZeroSearcher::BinCoord ArgumentZeroSearcher::BinCoord::middle(const BinCoord &c1, const BinCoord &c2) {
    auto c = compare(c1, c2);
    if (!c) return BinCoord(c1);
    auto smaller = &c1;
    if (c == 1) smaller = &c2;
    BinCoord res(*smaller);
    if (res.coord.size() > 1) {
        res.coord.pop_back();
        res.coord.pop_back();
        res.coord.push_back(true);
        res.coord.push_back(false);
    }
    auto len = max(c1.coord.size(), c2.coord.size());
    while (res.coord.size() < len)
        res.coord.push_back(false);
    res.coord.push_back(true);
    return res;
}

void ArgumentZeroSearcher::Point::operator=(const Point &p) {
    bx = p.bx;
    by = p.by;
    inx = p.x;
    iny = p.y;
}

unsigned long long ArgumentZeroSearcher::Point::hash() const {
    union mapper {
        unsigned long long res;
        unsigned int parts[2];
    } res;
    res.parts[0] = bx.hash();
    res.parts[1] = by.hash();
    return res.res;
}

void ArgumentZeroSearcher::Point::middle(const ArgumentZeroSearcher::Point &p1, const ArgumentZeroSearcher::Point &p2, ArgumentZeroSearcher::Point &res) {
    res.inx = (p1.x + p2.x) / 2;
    res.iny = (p1.y + p2.y) / 2;
    res.bx = BinCoord::middle(p1.bx, p2.bx);
    res.by = BinCoord::middle(p1.by, p2.by);
}

ArgumentZeroSearcher::Cell::Cell(const ArgumentZeroSearcher::Point &lb, const ArgumentZeroSearcher::Point &rt) : points() {
    points[0] = lb;
    points[2] = rt;
    Point &rb = points[1];
    rb = Point(rt.x, lb.y, false, false);
    rb.bx = rt.bx;
    rb.by = lb.by;
    Point &lt = points[3];
    lt = Point(lb.x, rt.y, false, false);
    lt.bx = lb.bx;
    lt.by = rt.by;
}

ArgumentZeroSearcher::Edge ArgumentZeroSearcher::Cell::edge(size_t ind) const {
    switch (ind) {
    case 0:
        return Edge{points[0], points[1]};
    case 1:
        return Edge{points[1], points[2]};
    case 2:
        return Edge{points[2], points[3]};
    case 3:
        return Edge{points[3], points[0]};
    default:
        throw 1;// RuntimeException("Bad edge index in ZeroSearcher Cell::edge");
    }
}

ArgumentZeroSearcher::Cell ArgumentZeroSearcher::Cell::cell(size_t ind) const {
    Point m, m2;
    switch (ind) {
    case 0:
        Point::middle(points[0], points[2], m);
        return Cell(points[0], m);
    case 1:
        Point::middle(points[0], points[1], m);
        Point::middle(points[1], points[2], m2);
        return Cell(m, m2);
    case 2:
        Point::middle(points[0], points[2], m);
        return Cell(m, points[2]);
    case 3:
        Point::middle(points[0], points[3], m);
        Point::middle(points[2], points[3], m2);
        return Cell(m, m2);
    default:
        throw 1;//RuntimeException("Bad cell index in ZeroSearcher Cell::cell");
    }
}

double ArgumentZeroSearcher::eval(const ArgumentZeroSearcher::Point &p, bool lookHash) {
    auto hash = p.hash();
    if (lookHash) {
        if (mem.count(hash)) return mem[hash];
    }
    neval++;
    auto res = func(p);
    if (res.real() < acc || neval > 3000) {
        found = true;
        zsol = p;
    }
    if (lookHash) mem[hash] = res.imag();
    return res.imag();
}

bool ArgumentZeroSearcher::addPhaseOnEdge(ArgumentZeroSearcher::Edge e, double &phase, bool lookHash) {
    double ph1 = eval(e.p1, lookHash);
    if (found) return true;
    double ph2 = eval(e.p2, lookHash);
    if (found) return true;
    double tph = ph2 - ph1;
    if (tph > M_PI) tph -= 2 * M_PI;
    if (tph < -M_PI) tph += 2 * M_PI;
    if (abs(tph) > 1) {
        Point m;
        Point::middle(e.p1, e.p2, m);
        if (m.order() > 30) {
            Point p1(e.p1.x, e.p1.y, false, false);
            Point p2(e.p2.x, e.p2.y, true, true);
            if (addPhaseOnEdge(Edge{p1, p2}, phase, false)) return true;
        } else {
            if (addPhaseOnEdge(Edge{e.p1, m}, phase, lookHash)) return true;
            if (addPhaseOnEdge(Edge{m, e.p2}, phase, lookHash)) return true;
        }
    } else
        phase += tph;
    return false;
}

bool ArgumentZeroSearcher::checkCell(const ArgumentZeroSearcher::Cell &c, bool &result) {
    double phase = 0;
    for (size_t i = 0; i < 4; i++)
        if (addPhaseOnEdge(c.edge(i), phase)) return true;
    result = abs(phase) > 1;
    return false;
}

bool ArgumentZeroSearcher::checkCellHard(size_t level, const ArgumentZeroSearcher::Cell &c, bool &result) {
    if (level == 0) return checkCell(c, result);
    for (size_t i = 0; i < 4; i++) {
        auto cell = c.cell(i);
        if (checkCellHard(level - 1, cell, result)) return true;
        if (result) return false;
    }
    return false;
}

void ArgumentZeroSearcher::iterate(size_t lev, const ArgumentZeroSearcher::Cell &c) {
    if (lev == 30) throw 1;//RuntimeException("Max level in ZeroSearcher reached");
    bool containZero;
    for (size_t hardness = 0; hardness < 3; hardness++) {
        if (hardness + lev == 30) break;
        for (size_t i = 0; i < 4; i++) {
            auto cell = c.cell(i);
            if (checkCellHard(hardness, cell, containZero)) return;
            if (containZero) {
                iterate(lev + 1, cell);
                if (found) break;
            }
        }
        if (found) break;
    }
    if (!(lev || found)) throw 1;//RuntimeException("ZeroSearcher error");
}

bool ArgumentZeroSearcher::findZero() {
    bool exists = false;
    Cell maincell(Point(minmax.xmin, minmax.ymin, false, false), Point(minmax.xmax, minmax.ymax, true, true));
    for (size_t i = 0; i < 2; i++) {
        if (checkCellHard(i, maincell, exists)) return true;
        if (exists) break;
    }
    if (!exists) return false;
    iterate(0, maincell);
    return true;
}

bool ArgumentZeroSearcher::find(const function<CMPLX(CMPLX)> &func, ArgumentZeroSearcher::Rect minmax, double acc, CMPLX &z) {
    ArgumentZeroSearcher sr(func, minmax, acc);
    auto ok = sr.findZero();
    if (ok) z = sr.zsol;
    //cout << "Number of calls: " << sr.neval << endl;
    return ok;
}

}