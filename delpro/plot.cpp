#include "plot.h"
#include <algorithm>
#include <giomm/simpleactiongroup.h>
#include <gtkmm/builder.h>
#include <fstream>

const unordered_map<string, int> Plot::colorind {
    {"black", 0},
    {"red", 1},
    {"yellow", 2},
    {"blue", 9},
    {"green", 3},
    {"magenta", 13},
    {"brown", 8}
};

Plot::Plot(MainWindow *wnd, ScreenOutput *sout) : Wgt(wnd, sout), minmax1(8), foundminmax1(8 ,0), needlefty(false), needrighty(false) {
    //Find Graph with given xtag
    auto find = [](const string &xtag, vector<Graph> &gs) {
        for (auto &g : gs) {
            if (g.xtag == xtag) return &g; // and return pointer to it
        }
        return static_cast<Graph*>(nullptr); // or nullptr
    };

    //Populate graphs depending on xtag and xref attributes
    for (DisplyedItem &item : source->items) {
        if (item.attributes.count("xtag")) {
            auto xtag = item.attributes["xtag"];
            auto pg = find(xtag, graphs);
            if (pg == nullptr) {
                graphs.emplace_back();
                auto &g = graphs.back();
                g.xtag = xtag;
                pg = &g;
            }
            else if (pg->x != nullptr) throw string("Double xtag for Plot");
            pg->x = &item;
            xtitle = (pg->x->var->desc + " (" + pg->x->var->unit + ")");
        }
        else if (item.attributes.count("xref")) {
            auto xtag = item.attributes["xref"];
            Graph *pg = find(xtag, graphs);
            if (pg == nullptr) {
                graphs.emplace_back();
                auto &g = graphs.back();
                g.xtag = xtag;
                pg = &g;
            }
            pg->ys.emplace_back();
            auto &y = pg->ys.back();
            y.y = &item;
            //extract color
            y.color = 0;
            if (item.attributes.count("color")) {
                auto &color = item.attributes["color"];
                if (colorind.count(color)) y.color = colorind.at(color);
            }
            //extract width
            y.width = 1;
            if (item.attributes.count("width")) {
                y.width = stod(item.attributes["width"]);
            }
            //extract style
            if (item.attributes.count("style")) {
                auto &style = item.attributes["style"];
                if (style == "dot") {
                    y.draws = {200};
                    y.spaces = {2000};
                }
                else if (style == "dash") {
                    y.draws = {2000};
                    y.spaces = {2000};
                }
                else if (style == "dashdot") {
                    y.draws = {2000, 200};
                    y.spaces = {2000, 2000};
                }
                else if (style == "dashdotdot") {
                    y.draws = {2000, 200, 200};
                    y.spaces = {2000, 2000, 2000};
                }
            }
            //extract yaxis position
            y.yright = false;
            if (item.attributes.count("yaxis")) {
                if (item.attributes["yaxis"] == "right") y.yright = true;
            }
            if (y.yright) needrighty = true;
            else needlefty = true;
            //extract ylimits
            if (item.attributes.count("ymin")) {
                int ind = y.yright ? 6 : 2;
                if (!foundminmax1[ind]) {
                    minmax1[ind] = stod(item.attributes["ymin"]);
                    foundminmax1[ind] = 2;
                }
            }
            if (item.attributes.count("ymax")) {
                int ind = y.yright ? 7 : 3;
                if (!foundminmax1[ind]) {
                    minmax1[ind] = stod(item.attributes["ymax"]);
                    foundminmax1[ind] = 2;
                }
            }
            if (item.attributes.count("ysymmetric") && item.attributes["ysymmetric"] == "true") {
                int ind = y.yright ? 6 : 2;
                auto min = abs(minmax1[ind]);
                auto max = abs(minmax1[ind + 1]);
                if (foundminmax1[ind] && foundminmax1[ind + 1]) {
                    if (min > max) max = min;
                    minmax1[ind] = -max;
                    minmax1[ind + 1] = max;
                }
                else if (foundminmax1[ind]) {
                    if (minmax1[ind] < 0) minmax1[ind + 1] = min;
                }
                else if (foundminmax1[ind + 1]) {
                    if (minmax1[ind + 1] > 0) minmax1[ind] = -max;
                }
            }
            //extract title
            if (y.yright) y.title = "[R] ";
            else y.title = "[L] ";
            y.title += y.y->var->desc;
            if (y.y->var->type == Types::DOUBLEVECTORSET) y.title += "#" + y.y->attributes["index"];
            y.title += " (" + y.y->var->unit + ")";
            //fill Legend
            legend.opt_array.push_back(PL_LEGEND_LINE);
            legend.text_colors.push_back(0);
            auto txt = const_cast<char*>(y.title.data());
            legend.texts.push_back(txt);
            legend.line_colors.push_back(y.color);
            legend.line_styles.push_back(1);
            legend.line_widths.push_back(y.width);
        }
        else throw string("Neither xtag nor xref for item in Plot");
    }
    for (auto &g : graphs) {
        if (g.x == nullptr) throw string("No xtag for xref ") + g.xtag;
        if (g.ys.size() == 0) throw string("No xref for xtag") + g.xtag;
    }
    legend.nlegend = legend.opt_array.size();
    minmax2 = minmax1;
    foundminmax2 = foundminmax1;
    scrollaxis = needlefty ? 1 : 2;
}

void Plot::attach(Gtk::Grid &c, int row, int col, int rowspan, int colspan) {
    c.attach(dynamic_cast<Gtk::DrawingArea&>(*this), col, row, colspan, rowspan);
    set_hexpand();
    set_vexpand();

    auto saveData = [this]() {
            auto volatile &psd = mainwindow->data->paused;
            auto state = psd;
            psd = true;
            auto dir = mainwindow->outputFolder();
            fstream file;
            auto time = Glib::DateTime::create_now_local();
            string fname = time.format("%y%m%d:%H%M%S") + "_" + source->attributes["title"] + ".dat";
            file.open(dir + "/" + fname, ios::out);
            if (file.is_open()) {
                string str = "";
                for (auto &g : graphs) {
                    if (str.size() > 0) str += ", ";
                    str += g.x->var->desc;
                    for (auto &y : g.ys) {
                        str += ", " + y.y->var->desc;
                    }
                }
                str += "\n";
                file.write(str.data(), str.length());

                long int ind = 0;
                char buf[20];
                bool neof = true;
                while (neof) {
                    neof = false;
                    string str = "";
                    for (auto &g : graphs) {
                        if (g.xdata.size() <= ind) continue;
                        neof = true;
                        if (str.size() > 0) str += ", ";
                        if (g.x->var->isValid) {
                            sprintf(buf,"%e",g.xdata[ind]);
                            str += string(buf);
                        }
                        for (auto &y : g.ys) {
                            str += ", ";
                            if (y.y->var->isValid) {
                                sprintf(buf,"%e",y.ydata[ind]);
                                str += string(buf);
                            }
                        }
                    }
                    str += "\n";
                    file.write(str.data(), str.length());
                    ind++;
                }
                file.close();
            }
            psd = state;
        };

    auto savepng = [this]() {
        auto volatile &psd = mainwindow->data->paused;
        auto state = psd;
        psd = true;
        auto dir = mainwindow->outputFolder();
        auto time = Glib::DateTime::create_now_local();
        string fname = dir + "/" + time.format("%y%m%d:%H%M%S") + "_" + source->attributes["title"] + ".png";
        auto surf = Cairo::ImageSurface::create(Cairo::Format::FORMAT_ARGB32, 800, 600);
        auto cr = Cairo::Context::create(surf);
        makeplot(cr, 800, 600);
        surf->write_to_png(fname);
        psd = state;
    };

    auto savepdf = [this]() {
        auto volatile &psd = mainwindow->data->paused;
        auto state = psd;
        psd = true;
        auto dir = mainwindow->outputFolder();
        auto time = Glib::DateTime::create_now_local();
        string fname = dir + "/" + time.format("%y%m%d:%H%M%S") + "_" + source->attributes["title"] + ".pdf";
        auto surf = Cairo::PdfSurface::create(fname, 800, 600);
        auto cr = Cairo::Context::create(surf);
        makeplot(cr, 800, 600);
        cr->show_page();
        psd = state;
    };

    auto setscroll = [this](int axis) {
        scrollaxis = axis;
        queue_draw();
    };

    auto reset = [this]() {
        minmax2 = minmax1;
        foundminmax2 = foundminmax1;
        queue_draw();
    };

    auto pactiongroup = Gio::SimpleActionGroup::create();
    pactiongroup->add_action("save_data", saveData);
    pactiongroup->add_action("save_png", savepng);
    pactiongroup->add_action("save_pdf", savepdf);
    pactiongroup->add_action("scrollbottom", sigc::bind(setscroll, 0));
    pactiongroup->add_action("scrollleft", sigc::bind(setscroll, 1));
    pactiongroup->add_action("scrollright", sigc::bind(setscroll, 2));
    pactiongroup->add_action("reset", reset);
    insert_action_group("pop", pactiongroup);

    auto pbuilder = Gtk::Builder::create();
    Glib::ustring ui_info =
  "<interface>"
  "  <menu id='popupmenu'>"
  "    <section>"
  "      <item>"
  "        <attribute name='label' translatable='no'>Save as data</attribute>"
  "        <attribute name='action'>pop.save_data</attribute>"
  "      </item>"
  "      <item>"
  "        <attribute name='label' translatable='no'>Save as png</attribute>"
  "        <attribute name='action'>pop.save_png</attribute>"
  "      </item>"
  "      <item>"
  "        <attribute name='label' translatable='no'>Save as pdf</attribute>"
  "        <attribute name='action'>pop.save_pdf</attribute>"
  "      </item>"
  "    </section>"
  "    <section>"
  "      <item>"
  "        <attribute name='label' translatable='no'>Scale x</attribute>"
  "        <attribute name='action'>pop.scrollbottom</attribute>"
  "      </item>"
  "      <item>"
  "        <attribute name='label' translatable='no'>Scale left</attribute>"
  "        <attribute name='action'>pop.scrollleft</attribute>"
  "      </item>"
  "      <item>"
  "        <attribute name='label' translatable='no'>Scale right</attribute>"
  "        <attribute name='action'>pop.scrollright</attribute>"
  "      </item>"
  "      <item>"
  "        <attribute name='label' translatable='no'>Reset view</attribute>"
  "        <attribute name='action'>pop.reset</attribute>"
  "      </item>"
  "    </section>"
  "  </menu>"
  "</interface>";
    pbuilder->add_from_string(ui_info);
    auto object = pbuilder->get_object("popupmenu");
    auto gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
    pmenu = new Gtk::Menu(gmenu);
    add_events(Gdk::BUTTON_PRESS_MASK);
    add_events(Gdk::SCROLL_MASK);
}

void Plot::draw() {
    for (Graph &g : graphs) {
        if (!g.x->var->isValid) continue;
        if (g.x->var->needUpdate) {
            auto &xv = *static_cast<vector<double>*>(g.x->var->mem);
            g.xdata = valarray<double>(xv.data(), xv.size()); // xdata to buffer
        }
        for (auto &y : g.ys) {
            if (!y.y->var->isValid) continue;
            if (y.y->var->needUpdate) {
                vector<double> *vec;
                if (y.y->var->type == Types::DOUBLEVECTORSET) {
                    auto &vecs = *static_cast<vector<vector<double>>*>(y.y->var->mem);
                    size_t ind = stoi(y.y->attributes["index"]);
                    vec = &vecs[ind];
                }
                else vec = static_cast<vector<double>*>(y.y->var->mem);
                y.ydata = valarray<double>(vec->data(), vec->size()); // ydata to buffer
            }
        }
    }
    queue_draw();
}

bool Plot::on_draw(const Cairo::RefPtr<Cairo::Context> &cr)
{
    auto allocation = get_allocation();
    auto wid = allocation.get_width();
    auto hei = allocation.get_height();
    makeplot(cr, wid, hei, true);
}

bool Plot::on_button_press_event(GdkEventButton *event) {
    cout << "Pressed: " << event->type << " : " << event->button << endl;
    if ((event->type == GDK_BUTTON_PRESS) && (event->button == 3)) {
        if (!pmenu->get_attach_widget()) pmenu->attach_to_widget(*this);
        pmenu->popup(event->button, event->time);
        return true; //It has been handled.
    }
    else
        return false;
}

bool Plot::on_scroll_event(GdkEventScroll* event) {
    if (event->x < vxmin || event->x > vxmax || event->y < vymin || event->y > vymax) return false;
    double mu = 2;
    if (event->direction == 0) mu = 0.5;
    double s;
    int ind = 0;
    if (scrollaxis) {
        s = 1. - (event->y - vymin) / (vymax - vymin);
        ind = scrollaxis == 2 ? 6 : 2;
    }
    else {
        s = (event->x - vxmin) / (vxmax - vxmin);
    }
    int ind1 = ind + 1;
    foundminmax2[ind] = foundminmax2[ind1] = 2;
    auto range = minmax2[ind1] - minmax2[ind];
    minmax2[ind] = minmax2[ind] + s * (1 - mu) * range;
    minmax2[ind1] = minmax2[ind1] + (mu - 1 + s * (1 - mu)) * range;
    queue_draw();
    return false;
}

void Plot::makeplot(const Cairo::RefPtr<Cairo::Context>& cr, int width, int height, bool toscr) {
    auto &mm = minmax2;
    auto found = foundminmax2;
    bool totfound = false;
    double mt;
    for (auto &g : graphs) {
        if (g.xdata.size()) {
            if (g.x->var->isValid) {
                mt = g.xdata.min();
                if (!found[0]) {
                    mm[0] = mt;
                    found[0] = 1;
                }
                else if (found[0] == 1 && mm[0] > mt) mm[0] = mt;
                mt = g.xdata.max();
                if (!found[1]) {
                    mm[1] = mt;
                    found[1] = 1;
                }
                else if (found[1] == 1 && mm[1] < mt) mm[1] = mt;
                for (auto &y : g.ys) {
                    if (y.y->var->isValid) {
                        totfound = true;
                        int ind = y.yright ? 6 : 2;
                        mt = y.ydata.min();
                        if (!found[ind]) {
                            mm[ind] = mt;
                            found[ind] = 1;
                        }
                        else if (found[ind] == 1 && mm[ind] > mt) mm[ind] = mt;
                        ind++;
                        mt = y.ydata.max();
                        if (!found[ind]) {
                            mm[ind] = mt;
                            found[ind] = 1;
                        }
                        else if (found[ind] == 1 && mm[ind] < mt) mm[ind] = mt;
                    }
                }
            }
        }
    }

    if (totfound) {
        auto pls = new plstream;
        pls->sdev("extcairo");
        pls->spage(0, 0, width, height, 0, 0);
        pls->init();
        pls->cmd(PLESC_DEVINIT, cr->cobj());

        pls->col0(0);
        pls->schr(3.5, 1);
        pls->adv(0);
        pls->vsta();
        if (toscr) {
            double xmin, xmax, ymin, ymax;
            pls->gvpd(xmin, xmax, ymin, ymax);
            vxmin = width * xmin;
            vxmax = width * xmax;
            vymin = height * ymin;
            vymax = height * ymax;
        }
        pls->wind(mm[0],mm[1],mm[2],mm[3]);
        if (scrollaxis) pls->width(1);
        else pls->width(2);
        pls->box("bnts", 0, 0, "", 0, 0);
        if (needlefty) {
            pls->wind(mm[0],mm[1],mm[2],mm[3]);
            if (scrollaxis != 1 || !toscr) pls->width(1);
            else pls->width(2);
            pls->box("", 0, 0, "bnts", 0, 0);
        }
        if (needrighty) {
            pls->wind(mm[0],mm[1],mm[6],mm[7]);
            if (scrollaxis == 2 && toscr) pls->width(2);
            else pls->width(1);
            pls->box("", 0, 0, "cmts", 0, 0);
        }
        pls->sfont(0, 0, 1);
        pls->lab("", "", source->attributes["title"].data());
        pls->sfont(0, 0, 0);
        pls->lab(xtitle.data(), "", "");
        pls->col0(0);
        PLINT m1(200), s1(2000);
        pls->styl(1, &m1, &s1);
        if (needlefty) pls->wind(mm[0],mm[1],mm[2],mm[3]);
        pls->width(1);
        pls->box("g", 0, 0, "g", 0, 0);

        for (auto &g : graphs) {
            auto &xvec = g.xdata;
            auto sz = xvec.size();
            if (g.x->var->isValid && sz) {
                for (auto &y : g.ys) {
                    if (y.y->var->isValid) {
                        int ind = y.yright ? 6 : 2;
                        pls->wind(mm[0], mm[1], mm[ind], mm[ind + 1]);
                        auto &yvec = y.ydata;
                        pls->col0(y.color);
                        pls->styl(y.draws.size(), &y.draws[0], &y.spaces[0]);
                        pls->width(y.width);
                        pls->line(sz, &xvec[0], &yvec[0]);
                    }
                }
            }
        }
        pls->width(1);
        pls->legend(&legend.legend_width, &legend.legend_height,
            //0, PL_POSITION_BOTTOM | PL_POSITION_OUTSIDE,
            PL_LEGEND_BOUNDING_BOX | PL_LEGEND_BACKGROUND, PL_POSITION_RIGHT | PL_POSITION_TOP,
            0.01, 0.01, 0.1, 15,
            0, 1, 1, 0,
            legend.nlegend, legend.opt_array.data(),
            1.0, 1.0, 2.0,
            1., legend.text_colors.data(), legend.texts.data(),
            NULL, NULL, NULL, NULL,
            legend.line_colors.data(), legend.line_styles.data(), legend.line_widths.data(),
            NULL, NULL,NULL, NULL);
        delete pls;
    }
}