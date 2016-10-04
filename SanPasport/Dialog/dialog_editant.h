#ifndef DIALOG_EDITANT_H
#define DIALOG_EDITANT_H
#include "prto.h"

#include <qwt_polar_grid.h>
#include <qwt_polar_curve.h>
#include <qwt_polar_panner.h>
#include <qwt_polar_magnifier.h>
#include <qwt_polar_marker.h>
#include <qwt_symbol.h>

#include <QDialog>
#include <QSqlTableModel>

namespace Ui {
class Dialog_EditAnt;
}

class Dialog_EditAnt : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_EditAnt(QWidget *parent = 0);
    ~Dialog_EditAnt();

signals:
    void sendEditAntFromDlgEdit(Antenna adnEditFromDlgEdit);

private slots:
    void on_pushButton_Ok_clicked();

    void dnCw();
    void dnCcw();
    void dnMirror();
    void dnSymmetryTop();
    void dnSymmetryBottom();
    void dnPreload();
    void dnReset();
    void getDataForm();

public slots:
    void insertDataForm(Antenna adnEdit);
    void initPolarGraph(Antenna dan);
    void powerCalc();

private:
    Ui::Dialog_EditAnt *ui;
    QwtPolarGrid *gridPolar;                       // координатная сетка
    QwtPolarCurve *CurvPolarHoriz,*CurvPolarVert,*CurvPolar3db;   // кривые, отображаемые на графике
};

#endif // DIALOG_EDITANT_H
