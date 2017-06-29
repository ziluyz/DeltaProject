#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>
#include "delpro.h"

struct Data;

class MainWindow : public QWidget
{
    Q_OBJECT
private:
    Data *data;
public:
    explicit MainWindow(void *data);
    bool needUpdate;
    QLabel *label;
protected:
    void timerEvent(QTimerEvent *event) override;

signals:

public slots:
};

#endif // MAINWINDOW_H
