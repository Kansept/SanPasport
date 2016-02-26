#ifndef DIALOG_PROJECTION_PKAEMO4_H
#define DIALOG_PROJECTION_PKAEMO4_H

#include <QLabel>
#include <QDialog>

namespace Ui {
class Dialog_projection_PKAEMO4;
}

class Dialog_projection_PKAEMO4 : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QLabel *sb1;
    QLabel *sb2;

private:
    Ui::MainWindow *ui;

private slots:
     bool openFiles();
     void loadFiles(QStringList qslFiles);
     bool unionFiles();
     bool setPath();
     QByteArray strMaxItem(QByteArray qbaSrez, QByteArray qbaRead);

};


#endif // DIALOG_PROJECTION_PKAEMO4_H
