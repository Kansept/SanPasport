#ifndef DIALOG_TASKPOINT_H
#define DIALOG_TASKPOINT_H

#include <QDialog>
#include "prto.h"

namespace Ui {
class Dialog_TaskPoint;
}

class Dialog_TaskPoint : public QDialog
{
    Q_OBJECT
    
public:
    explicit Dialog_TaskPoint(QWidget *parent = 0);
    ~Dialog_TaskPoint();
    
private slots:
    void on_pushButton_Ok_clicked();
    void clearUI();
    void on_pushButton_Cancel_clicked();

public slots:
    void insertData(Task tsk);

private:
    Ui::Dialog_TaskPoint *ui;

signals:
    void sendTaskPoint();

};

#endif // DIALOG_TASKPOINT_H
