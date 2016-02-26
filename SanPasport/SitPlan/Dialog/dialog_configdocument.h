#ifndef DIALOG_CONFIGDOCUMENT_H
#define DIALOG_CONFIGDOCUMENT_H
#include "SitPlan/config.h"

#include <QDialog>
#include <QVariant>

namespace Ui {
class Dialog_ConfigDocument;
}

class Dialog_ConfigDocument : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_ConfigDocument(QWidget *parent = 0);
    ~Dialog_ConfigDocument();

private:
    Ui::Dialog_ConfigDocument *ui;

private slots:
    void saveSettings();

public slots:
    void loadSettings(ConfigSitPlan confPageConfig);

signals:
    void sendOkDialog_ConfigPage(QVariant);
};

#endif // DIALOG_CONFIGDOCUMENT_H
