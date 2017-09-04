#ifndef WGT_H
#define WGT_H

#include <vector>
#include "delpro.h"

struct ScreenOutput;

//Abstract class for ScreenOutput widget
class Wgt {
protected:
    ScreenOutput *source; // corresponds to ScreenOutput record in ixml
public:
    Wgt(ScreenOutput *sout) : source(sout) {}
    virtual void draw()=0; // draws itself
    virtual void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) = 0; // places itself into layout system
};

#include "textfield.h"
#include "plot.h"

#endif // WGT_H
