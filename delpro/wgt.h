#ifndef WGT_H
#define WGT_H

#include <vector>
#include "delpro.h"

struct ScreenOutput;

class Wgt {
protected:
    ScreenOutput *source;
public:
    Wgt(ScreenOutput *sout) : source(sout) {}
    virtual void draw()=0;
    virtual void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) = 0;
};

#include "textfield.h"
#include "plot.h"

#endif // WGT_H
