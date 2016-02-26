#ifndef DIALOG_CONFIZGRAPH_H
#define DIALOG_CONFIZGRAPH_H

#include <QDialog>
#include <QSettings>
#include "MyWidget/toolbuttoncolor.h"

namespace Ui {
class Dialog_ConfizGraph;
}

class Dialog_ConfizGraph : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_ConfizGraph(QWidget *parent = 0);
    ~Dialog_ConfizGraph();

protected:
    void accept();
    void showEvent(QShowEvent *event);

private slots:
    void gridColor();
    void zozColor();

private:
    Ui::Dialog_ConfizGraph *ui;
    QSettings *stngViewPlot;

};

#endif // DIALOG_CONFIZGRAPH_H
