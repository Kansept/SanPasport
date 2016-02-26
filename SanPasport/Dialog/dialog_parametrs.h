#ifndef DIALOG_PARAMETRS_H
#define DIALOG_PARAMETRS_H

#include <QDialog>

namespace Ui {
class Dialog_Parametrs;
}

class Dialog_Parametrs : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Parametrs(QWidget *parent = 0);
    ~Dialog_Parametrs();

private slots:
    void setParam();

public slots:
    void loadParam(const float koef);

signals:
    void SendOptionsFromDlgOption(QString qsSendOptions);

private:
    Ui::Dialog_Parametrs *ui;
};

#endif // DIALOG_PARAMETRS_H
