#include "textfield.h"
#include <cstdio>

TextField::TextField(MainWindow *wnd, ScreenOutput *sout) : Wgt(wnd, sout) {}

void TextField::attach(Gtk::Grid& c, int row, int col, int rowspan, int colspan) {
    box.set_orientation(Gtk::ORIENTATION_VERTICAL);
    box.pack_start(title, Gtk::PACK_SHRINK, 10);
    box.pack_start(content, Gtk::PACK_SHRINK, 0);
    c.attach(box, col, row, colspan, rowspan);
    title.set_use_markup();
    title.set_label("<big><b>" + source->attributes["title"] + "</b></big>");
    title.set_xalign(0.5);
    content.set_padding(10,0);
    box.show_all();
}

void TextField::draw() {
    string output;// = source->attribute contains("title") ? source->attributes["title"] + "\n" : "";
    char buf[20];
    auto printdouble = [&buf](double d) {
        sprintf(buf, "%g", d);
        return string(buf);
    };
    for (DisplyedItem& item : source->items) {
        if (!item.var->isValid) continue;
        output.append(item.var->desc + ": ");
        switch(item.var->type) {
        case Types::INT:
            output += to_string(*static_cast<int*>(item.var->mem));
            break;
        case Types::DOUBLE:
            output += printdouble(*static_cast<double*>(item.var->mem));
            break;
        case Types::INTVECTOR: {
            auto &vec = *static_cast<vector<int>*>(item.var->mem);
            for (auto d : vec) output += to_string(d) + ", ";
            //output.truncate(output.size() - 2);
            break;}
        case Types::DOUBLEVECTOR: {
            auto &vec = *static_cast<vector<double>*>(item.var->mem);
            bool nfirst = false;
            for (auto d : vec) {
                if (nfirst) output += ", ";
                nfirst = true;
                output += printdouble(d);
            }
            //output.truncate(output.size() - 2);
            break;}
        }
        output.append(" " + item.var->unit + "\n");
    }
    content.set_text(output);
    //label->adjustSize(); // resize to fit the new data
}
