#include "textfield.h"

TextField::TextField(ScreenOutput *sout) : Wgt(sout) {
    label = new QLabel("");
}

void TextField::attach(QGridLayout& c, int row, int col, int rowspan, int colspan) {
    QVBoxLayout *holder = new QVBoxLayout();
    QLabel *title = new QLabel(source->attributes["title"]);
    title->setFont(QFont("Helvetica",11,QFont::Bold));
    holder->addWidget(title, 0, Qt::AlignHCenter);
    holder->addWidget(label, 0, Qt::AlignHCenter);
    c.addLayout(holder, row, col, rowspan, colspan);
}

void TextField::draw() {
    QString output = source->attributes.contains("title") ? source->attributes["title"] + "\n" : "";
    for (OutputItem& item : source->items) {
        if (!item.var->isValid) continue;
        output.append(item.var->desc + ": ");
        switch(item.var->type) {
        case Types::INT:
            output.append(QString::number(*static_cast<int*>(item.var->mem)));
            break;
        case Types::DOUBLE:
            output.append(QString::number(*static_cast<double*>(item.var->mem)));
            break;
        case Types::INTVECTOR: {
            auto &vec = *static_cast<QVector<int>*>(item.var->supply);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            break;}
        case Types::DOUBLEVECTOR: {
            auto &vec = *static_cast<QVector<double>*>(item.var->supply);
            for (auto d : vec) output.append(QString::number(d) + ", ");
            output.truncate(output.size() - 2);
            break;}
        }
        output.append(" " + item.var->unit + "\n");
    }
    label->setText(output);
    label->adjustSize();
    //label->setStyleSheet("QLabel { background-color : red; color : blue; }");
}
