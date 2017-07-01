#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include "delpro.h"
#include <memory>

using namespace std;

struct Data;
class ScreenOutput;
class Wgt;

class MainWindow : public QWidget
{
    Q_OBJECT
private:
    Data *data;
public:
    explicit MainWindow(Data *data);
    bool needUpdate;
    vector<shared_ptr<Wgt>> content;
protected:
    void timerEvent(QTimerEvent *event) override;

signals:

public slots:
};

class Wgt {
protected:
    QLayout *base;
    ScreenOutput *source;
public:
    Wgt(ScreenOutput *sout) : source(sout) {}
    virtual void draw()=0;
    virtual void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) = 0;
};

class TextField : public Wgt {
private:
    QLabel *label;
public:
    TextField(ScreenOutput *sout);
    void attach(QGridLayout& c, int row, int col, int rowspan, int colspan) override;
    void draw() override;
};

#endif // MAINWINDOW_H
