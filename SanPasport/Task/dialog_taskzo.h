#ifndef DIALOG_TASKZO_H
#define DIALOG_TASKZO_H

#include <QDialog>
#include <QShowEvent>

#include "prto.h"

namespace Ui {
class Dialog_TaskZo;
}

class Dialog_TaskZo : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_TaskZo(QWidget *parent = 0);
    ~Dialog_TaskZo();

private:
    Ui::Dialog_TaskZo *ui;

private slots:
    void pushButton_Ok();
    void pushButton_Cancel();
    void clearUI();

public slots:
    void insertData(task tsk);
    void insertHeight(const QString strZ);

signals:
    void sendTaskZo();

protected:
    void showEvent(QShowEvent *event);
};

#endif // DIALOG_TASKZO_H
