#ifndef DIALOG_PREFERENS_H
#define DIALOG_PREFERENS_H

#include <QDialog>
#include <QSettings>
#include <QShowEvent>
#include <QModelIndex>
#include <QListWidgetItem>


namespace Ui {
class Dialog_Preferens;
}

class Dialog_Preferens : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_Preferens(QWidget *parent = 0);
    ~Dialog_Preferens();

protected:
    void showEvent(QShowEvent *event);

protected slots:
    void readSetting();
    void writeSetting();

private slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void dBPathChange();

private:
    Ui::Dialog_Preferens *ui;
    QSettings *stngSanPasport;

signals:
    void dbPath(QString);
};

#endif // DIALOG_PREFERENS_H
