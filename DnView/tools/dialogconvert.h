#ifndef DIALOGCONVERT_H
#define DIALOGCONVERT_H

#include <QDialog>
#include "../Widget/DotLineEdit.h"

namespace Ui {
class DialogConvert;
}

class DialogConvert : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogConvert(QWidget *parent = 0);
    ~DialogConvert();
    
private slots:

    void on_pushButton_clicked();

    void wattToDBm(QString arg1);
    void dBmToWatt(QString arg1);
    void freqToDlvoln(QString arg1);
    void dlvolnToFreq(QString arg1);

private:
    Ui::DialogConvert *ui;
};

#endif // DIALOGCONVERT_H
