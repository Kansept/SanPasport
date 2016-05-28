#ifndef DIALOG_TASKVS_H
#define DIALOG_TASKVS_H

#include <QDialog>
#include <QSettings>
#include <QShowEvent>

#include "prto.h"

namespace Ui {
class Dialog_TaskVs;
}

class Dialog_TaskVs : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_TaskVs(QWidget *parent = 0);
    ~Dialog_TaskVs();

private:
    Ui::Dialog_TaskVs *ui;

private slots:
    void pushButton_Ok();
    void pushButton_Cancel();
    void clearUI();

public slots:
    void insertData(Task tsk);

signals:
    void sendTaskVs();

protected:
    void showEvent(QShowEvent *event);
};

#endif // DIALOG_TASKVS_H
