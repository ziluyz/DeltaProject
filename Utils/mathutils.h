#include <functional>
#include <vector>
#include <unordered_map>
#include <complex>
#include <cmath>

#define CMPLX std::complex<double>

namespace mut {

struct RectBound {double xmin, xmax, ymin, ymax;};

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

class ArgumentZeroSearcher {

private:
    class BinCoord {
    private:
        std::vector<bool> coord;
        bool operator[](size_t ind) const;

    public:
        BinCoord(bool val) {coord.push_back(val);}
        unsigned int hash() const;
        size_t order() const {return coord.size();}
        static int compare(const BinCoord &c1, const BinCoord &c2);
        static BinCoord middle(const BinCoord &c1, const BinCoord &c2);
    };

    class Cell;

    class Point {
        friend class Cell;

    private:
        BinCoord bx, by;
        double inx, iny;

    public:
        const double &x = inx, &y = iny;
        Point() : bx(false), by(false), inx(0), iny(0) {}
        Point(double x, double y, bool bx, bool by) : bx(bx), by(by), inx(x), iny(y) {}
        Point(const Point &p) = delete;
        void operator=(const Point &p);
        operator CMPLX() const { return CMPLX{inx, iny}; }
        unsigned long long hash() const;
        size_t order() const { return std::max(bx.order(), by.order());}
        static void middle(const Point &p1, const Point &p2, Point &res);
    };

    class Edge {
    public:
        const Point &p1, &p2;
    };

    class Cell {
    private:
        Point points[4];

    public:
        Cell(const Point &lb, const Point &rt);
        Cell(const Cell &c) {for (size_t i = 0; i < 4; i++) points[i] = c.points[i];}
        Edge edge(size_t ind) const;
        Cell cell(size_t ind) const;
    };

    const std::function<CMPLX(CMPLX)> &func;
    RectBound minmax;
    double acc;
    bool found = false;
    CMPLX zsol;
    int neval = 0;
    std::unordered_map<unsigned long long, double> mem;
    ArgumentZeroSearcher(const std::function<CMPLX(CMPLX)> &func, RectBound minmax, double acc) : func(func), minmax(minmax), acc(acc) {}
    double eval(const Point &p, bool lookHash);
    bool addPhaseOnEdge(Edge e, double &phase, bool lookHash = true);
    bool checkCell(const Cell &c, bool &result);
    bool checkCellHard(size_t level, const Cell &c, bool &result);
    void iterate(size_t lev, const Cell &c);
    bool findZero();

public:
    static bool find(const std::function<CMPLX(CMPLX)> &func, RectBound minmax, double acc, CMPLX &z);
};

class MullerZeroSearcher {
private:
    int n = 1;
    CMPLX z1, z2, f1, f2, f12;
    const std::function<CMPLX(CMPLX)> &func;
    double acc;
    bool found = false;
    MullerZeroSearcher(const std::function<CMPLX(CMPLX)> &func, double acc) : func(func), acc(acc) {}
    CMPLX proceed(CMPLX z);
public:
    static bool find(const std::function<CMPLX(CMPLX)> &func, RectBound minmax, double acc, CMPLX &z);
};

class Follower {
private:
    int npoints;
    RectBound minmax, gminmax;
    double acc;
    const std::function<double(int)> &freeVar;
    const std::function<void(double)> &prepare;
    const std::function<bool(const std::function<CMPLX(CMPLX)> &, RectBound, double, CMPLX &)> &method;
    const std::function<CMPLX(CMPLX)> &func;
    const std::function<void(double, CMPLX)> &process;
    CMPLX z;
    Follower(int N, RectBound minmax, RectBound gminmax, double acc,
            const std::function<double(int)> &freeVar,
            const std::function<void(double)> &prepare,
            const std::function<bool(const std::function<CMPLX(CMPLX)> &, RectBound, double, CMPLX &)> &method,
            const std::function<CMPLX(CMPLX)> &func,
            const std::function<void(double, CMPLX)> &process) : npoints(N), minmax(minmax), gminmax(gminmax), acc(acc),
                freeVar(freeVar), prepare(prepare), method(method), func(func), process(process) {}
    bool makeStep();
    void checkLimits();
    bool follow();

public:
    static void scan(int N,
            const std::function<double(int)> &freeVar,
            const std::function<void(double)> &prepare,
            const std::function<bool(const std::function<CMPLX(CMPLX)> &func, RectBound minmax, double acc, CMPLX &z)> &method,
            const std::function<CMPLX(CMPLX)> &func,
            const std::function<void(double, CMPLX)> &process,
            RectBound gminmax, RectBound minmax, double acc);
};

}