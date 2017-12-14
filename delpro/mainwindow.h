#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <memory>
#include <set>
#include "delpro.h"
#include "wgt.h"

using namespace std;

struct Data; // declared in delpro.h
class Wgt; // declared in wgt.h

//Main window containing all the outputs
class MainWindow : public QWidget
{
    Q_OBJECT
private:
    Data *data; // pointer to main data block
    set<shared_ptr<Wgt>> toDraw; // set of widgets which are necessary to redraw during timerEvent
    QPushButton *pauseButton;
    QLabel *timeLabel;
public:
    explicit MainWindow(Data *data);
    bool needUpdate; // global flag set to true when any variable gets update
    vector<shared_ptr<Wgt>> content; // ScreenOutput widgets
    unique_ptr<QDir> outputFolder(); // generates pointer to the output folder (different for every run of calling program)
protected:
    void timerEvent(QTimerEvent *event) override; // event used for the screen update

signals:

public slots:
    void calcFinished();
    void pauseClicked();
};

#endif // MAINWINDOW_H
