#ifndef WINDOW_PROJECTION_PKAEMO4_H
#define WINDOW_PROJECTION_PKAEMO4_H

#include <QMainWindow>
#include <QLabel>

namespace Ui {
class Window_projection_PKAEMO4;
}

class Window_projection_PKAEMO4 : public QMainWindow
{
    Q_OBJECT

public:
    explicit Window_projection_PKAEMO4(QWidget *parent = 0);
    ~Window_projection_PKAEMO4();

    QLabel *sb0;
    QLabel *sb1;
    QLabel *sb2;

private:
    Ui::Window_projection_PKAEMO4 *ui;

private slots:
     bool openFiles();
     void loadFiles(QStringList qslFiles);
     bool unionFiles();
     bool setPath();
     QByteArray strMaxItem(QByteArray qbaSrez, QByteArray qbaRead);
     void clearData();

};

#endif // WINDOW_PROJECTION_PKAEMO4_H

