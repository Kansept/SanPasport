#ifndef DIALOG_GROUPCHANGE_H
#define DIALOG_GROUPCHANGE_H

#include <QDialog>
#include <QShowEvent>

namespace Ui {
class Dialog_GroupChange;
}

class Dialog_GroupChange : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_GroupChange(QWidget *parent = 0);
    ~Dialog_GroupChange();

private:
    Ui::Dialog_GroupChange *ui;

private slots:
    void setReplase(const bool b);
    bool openFiles();
    void loadFiles(const QStringList qslFiles);
    void clearData();
    bool setChangeDir();
    void startConvert();
    void removePatterns();

protected:
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent * event);
    void showEvent(QShowEvent *event);
};

#endif // DIALOG_GROUPCHANGE_H
