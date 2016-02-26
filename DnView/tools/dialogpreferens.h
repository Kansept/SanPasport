#ifndef DIALOGPREFERENS_H
#define DIALOGPREFERENS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class DialogPreferens;
}

class DialogPreferens : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogPreferens(QWidget *parent = 0);
    ~DialogPreferens();

signals:
    void sendPrefOk();
    
private slots:
    void on_toolButton_ColorHorizDN_clicked();
    void on_toolButton_ColorVertDN_clicked();
    void action_pushButton_Ok();

private:
    Ui::DialogPreferens *ui;
    QSettings *settingsPref;

};

#endif // DIALOGPREFERENS_H
