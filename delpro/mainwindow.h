#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <memory>
#include <set>
#include "delpro.h"
#include "wgt.h"
#include <gtkmm/window.h>
#include <gtkmm/button.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>

using namespace std;

struct Data; // declared in delpro.h
class Wgt; // declared in wgt.h

//Main window containing all the outputs
class MainWindow : public Gtk::Window
{
private:
    set<shared_ptr<Wgt>> toDraw; // set of widgets which are necessary to redraw during timerEvent
    Gtk::Box mainBox, controlPanel;
    Gtk::Grid lay;
    Gtk::Button pauseButton;
    Gtk::Label timeLabel;
    chrono::time_point<chrono::steady_clock> timePaused;
public:
    explicit MainWindow(Data *data);
    Data *data; // pointer to main data block
    bool needUpdate; // global flag set to true when any variable gets update
    vector<shared_ptr<Wgt>> content; // ScreenOutput widgets
    bool timerEvent(); // event used for the screen update
    string outputFolder();
    void calcStarted();
    void calcFinished();
    void pauseClicked(Glib::ustring);
};

#endif // MAINWINDOW_H
