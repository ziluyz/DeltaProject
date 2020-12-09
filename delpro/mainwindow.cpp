#include "mainwindow.h"
#include <glib.h>
#include <glib/gstdio.h>
#include <glibmm/miscutils.h>
#include <giomm/file.h>
#include <locale.h>

// populates itself with ScreenOutput widgets according to their positions
MainWindow::MainWindow(Data *data) : pauseButton("Pause"), needUpdate(true)
{
    this->data = data;
    Data &d = *data;
    //Main Layout
    setlocale(LC_ALL, "en_US.UTF-8");
    add(mainBox);
    controlPanel.set_orientation(Gtk::ORIENTATION_VERTICAL);
    mainBox.pack_end(controlPanel, Gtk::PACK_SHRINK, 10);
    pauseButton.signal_clicked().connect(sigc::bind<Glib::ustring>(sigc::mem_fun(*this, &MainWindow::pauseClicked), "pause"));
    controlPanel.pack_start(pauseButton, Gtk::PACK_SHRINK, 10);
    pauseButton.show();
    controlPanel.pack_start(timeLabel, Gtk::PACK_SHRINK);
    timeLabel.show();
    controlPanel.show();
    mainBox.pack_end(lay);
    lay.set_row_homogeneous(true);
    // cycle over all ScrrenOutputs
    for (auto &sout : d.screenOutputs) {
        // constructing widgets according to types
        switch (sout.type) {
        case ScreenTypes::TEXTFIELD:
            content.emplace_back(new TextField(this, &sout));
            break;
        case ScreenTypes::PLOT:
            content.emplace_back(new Plot(this, &sout));
            break;
        default:
            throw string("Error initializing ScreenOutput");
            break;
        }
        // extracting widget position in the layout
        auto pos = sout.attributes["position"];
        auto sp = split(pos, ":");
        if (sp.size() != 2) throw string("Invalid position ") + pos + " of ScreenOutput";
        auto coords = split(sp[0], ",");
        if (coords.size() != 2) throw string("Invalid position ") + pos + " of ScreenOutput";
        auto spans = split(sp[1], ",");
        if (spans.size() != 2) throw string("Invalid position ") + pos + " of ScreenOutput";
        // attaching widget to the layout
        content.back()->attach(lay, stoi(coords[1]) - 1, stoi(coords[0]) - 1, stoi(spans[1]), stoi(spans[0]));
        // each variable knows all the widgets it is drawn by
        for (auto &item : sout.items) item.var->wgts.push_back(content.back());
    }
    lay.show_all();
    mainBox.show();
}

bool MainWindow::timerEvent() {
    if (data->paused) return true;
    auto end = chrono::steady_clock::now();
    if (!data->thread->isRunning()) end = data->chronoEnd;
    double tdur = chrono::duration_cast<chrono::duration<double>>(end - data->chronoStart).count();
    int hours = tdur / 3600;
    tdur -= hours * 3600;
    int minutes = tdur / 60;
    tdur -= minutes * 60;
    int seconds = tdur;
    tdur -= seconds;
    int millis = tdur * 1000;
    auto time = to_string(hours) + ":";
    if (minutes < 10) time += "0";
    time += to_string(minutes) + ":";
    if (seconds < 10) time += "0";
    time += to_string(seconds) + ":";
    if (millis < 100) time += "0";
    if (millis < 10) time += "0";
    time += to_string(millis);
    timeLabel.set_text(time);
    if (!needUpdate) return true; // gllobal flag shows if redraw is necessary at all
    toDraw.clear(); // set should contain only those which really contain recently updated data
    for (auto &var : data->inputVars) {
        if (var.isNew) { // input variables are always valid
            var.isNew = false;
            var.needUpdate = true;
            for (auto wgt : var.wgts) toDraw.insert(wgt);
        }
        else var.needUpdate = false;
    }
    for (auto &var : data->outputVars) {
        if (var.isNew && var.isValid) { // output variable can be updated but invalid
            var.isNew = false;
            var.needUpdate = true;
            for (auto wgt : var.wgts) toDraw.insert(wgt);
        }
        else var.needUpdate = false;
    }
    // draw what should be drawn
    for (auto wgt : toDraw) {
        wgt->draw();
    }
    needUpdate = false; // waiting for new updates
    return true;
}

// generate output folder for this particular run or return existing, if already exists
string MainWindow::outputFolder() {
    // if output folder for the current run does not exist, create it...
    auto folder = data->startTime.format("%y%m%d:%H%M%S");
    auto fulfolder = Glib::build_filename("Output", folder);
    if (!g_file_test(fulfolder.data(), G_FILE_TEST_EXISTS)) {
        g_mkdir_with_parents(fulfolder.data(), 0755);
        auto dest = Gio::File::create_for_path(fulfolder + "/" + data->inputFile);
        auto inf = Gio::File::create_for_path(data->inputFile);
        inf->copy(dest);
        for (auto filename : data->filesToSave) {
            dest = Gio::File::create_for_path(fulfolder + "/" + filename);
            inf = Gio::File::create_for_path(filename);
            inf->copy(dest);
        }
    }
    return fulfolder;
}

void MainWindow::calcStarted() {
    data->chronoStart = chrono::steady_clock::now();
}

void MainWindow::calcFinished() {
    data->chronoEnd = chrono::steady_clock::now();
    pauseButton.set_sensitive(false);
}

void MainWindow::pauseClicked(Glib::ustring inf) {
    data->paused = !data->paused;
    if (data->paused) {
        pauseButton.set_label("Resume");
        timePaused = chrono::steady_clock::now();
    }
    else {
        pauseButton.set_label("Pause");
        data->chronoStart += chrono::steady_clock::now() - timePaused;
    }
}
