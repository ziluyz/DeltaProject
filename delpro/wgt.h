#ifndef WGT_H
#define WGT_H

#include <vector>
#include "delpro.h"
#include "mainwindow.h"
#include "gtkmm/grid.h"

class MainWindow;
struct ScreenOutput;

//Abstract class for ScreenOutput widget
class Wgt {
protected:
    MainWindow *mainwindow;
    ScreenOutput *source; // corresponds to ScreenOutput record in ixml
public:
    Wgt(MainWindow *wnd, ScreenOutput *sout) : mainwindow(wnd), source(sout) {}
    virtual void draw()=0; // draws itself
    virtual void attach(Gtk::Grid& c, int row, int col, int rowspan, int colspan) = 0; // places itself into layout system
};

#include "textfield.h"
#include "plot.h"

#endif // WGT_H
