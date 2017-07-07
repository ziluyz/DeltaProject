#ifndef TEXTFIELD_H
#define TEXTFIELD_H

#include "wgt.h"

class TextField : public Wgt {
private:
    QLabel *label;
public:
    TextField(ScreenOutput *sout);
    void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
};

#endif // TEXTFIELD_H
