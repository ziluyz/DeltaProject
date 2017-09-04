#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "wgt.h"

//Capable of printing variables as text
class TextField : public Wgt {
private:
    QLabel *label; // All data are printed through this label
public:
    TextField(ScreenOutput *sout);
    void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
};

#endif // TEXTFIELD_H
