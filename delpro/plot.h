#ifndef PLOT_H
#define PLOT_H

#include "wgt.h"
#include <gtkmm/drawingarea.h>
#include <valarray>
#include <vector>
#include <unordered_map>
#include <plstream.h>
#include <gtkmm/menu.h>
#include <iostream>

using namespace std;

struct DisplyedItem;

//Plotting graphs based on DoubleVectors
class Plot : public Wgt, public Gtk::DrawingArea {
private:
    //A set of QCPGraphs with the same x data
    struct Graph {
        //A variable, its plot data buffer and corresponding QCPGraph
        struct gtuple {
            DisplyedItem *y;
            valarray<double> ydata;
            bool yright;
            int color;
            vector<PLINT> draws;
            vector<PLINT> spaces;
            PLFLT width;
            string title;
        };

        string xtag; // tag for x value in ixml
        DisplyedItem *x; // x variable
        valarray<double> xdata; // buffer for x variable
        vector<gtuple> ys; // from the set of y variable referencing the xtag (xref="xtag") in ixml
    };
    struct Legend {
        PLFLT legend_width, legend_height;
        PLINT nlegend;
        vector<PLINT> opt_array;
        vector<PLINT> text_colors;
        vector<PLINT> line_colors;
        vector<PLINT> line_styles;
        vector<PLFLT> line_widths;
        vector<char*> texts;
    };
    vector<double> minmax;
    vector<int> foundminmax;
    bool needlefty, needrighty;
    const static unordered_map<string, int> colorind;
    vector<Graph> graphs; // vector.size = number of different xtags in this ScreenOutput
    Legend legend;
    string xtitle;
    Gtk::Menu *pmenu;
    bool scrollright;
    void makeplot(const Cairo::RefPtr<Cairo::Context>&, int, int, bool = false);

protected:
    bool on_draw(const Cairo::RefPtr<Cairo::Context> &cr) override;
    bool on_button_press_event(GdkEventButton*) override;
    bool on_scroll_event(GdkEventScroll*) override;

public:
    Plot(MainWindow*, ScreenOutput*);
    void attach(Gtk::Grid& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
    void setscroll(bool right) {scrollright = right; queue_draw();};
    ~Plot() {if (pmenu!=nullptr) delete pmenu;};
};

#endif // PLOT_H
