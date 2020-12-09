#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "wgt.h"
#include <gtkmm/label.h>
#include <gtkmm/box.h>

//Capable of printing variables as text
class TextField : public Wgt {
private:
    Gtk::Box box; // Container for Title and Content
    Gtk::Label title; // Title
    Gtk::Label content; // Content : all data are printed through this label
public:
    TextField(MainWindow*, ScreenOutput*);
    void attach(Gtk::Grid& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
};

#endif // TEXTFIELD_H
