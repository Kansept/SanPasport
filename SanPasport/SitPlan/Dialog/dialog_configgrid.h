#ifndef DIALOG_CONFIGGRID_H
#define DIALOG_CONFIGGRID_H

#include "SitPlan/config.h"

#include <QDialog>
#include <QVariant>

namespace Ui {
class Dialog_ConfigGrid;
}

class Dialog_ConfigGrid : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_ConfigGrid(QWidget *parent = 0);
    ~Dialog_ConfigGrid();

private:
    Ui::Dialog_ConfigGrid *ui;

private slots:
    void saveSettings();
    void setGridColor();

public slots:
    void loadSettings(ConfigSitPlan confZoConfig);

signals:
    void sendOkDialog_ConfigGrid(QVariant);

};

#endif // DIALOG_CONFIGGRID_H
