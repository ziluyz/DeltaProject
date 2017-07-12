#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include <memory>
#include <set>
#include "delpro.h"
#include "wgt.h"

using namespace std;

struct Data;
class Wgt;

class MainWindow : public QWidget
{
    Q_OBJECT
private:
    Data *data;
    set<shared_ptr<Wgt>> toDraw;
public:
    explicit MainWindow(Data *data);
    bool needUpdate;
    vector<shared_ptr<Wgt>> content;
    unique_ptr<QDir> outputFolder();
protected:
    void timerEvent(QTimerEvent *event) override;

signals:

public slots:
};

#endif // MAINWINDOW_H
