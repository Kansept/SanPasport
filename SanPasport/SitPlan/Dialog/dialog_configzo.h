#ifndef DIALOG_CONFIGZO_H
#define DIALOG_CONFIGZO_H

#include <QDialog>
#include <QSettings>
#include <QMetaType>
#include "SitPlan/config.h"

namespace Ui {
class Dialog_ConfigZo;
}

class Dialog_ConfigZo : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_ConfigZo(QWidget *parent = 0);
    ~Dialog_ConfigZo();

private:
    Ui::Dialog_ConfigZo *ui;

private slots:
    void saveSettings();
    void setZoColor();

public slots:
    void loadSettings(ConfigSitPlan confZoConfig);

signals:
    void sendOkDialog_ConfigZo(QVariant);
};

#endif // DIALOG_CONFIGZO_H
