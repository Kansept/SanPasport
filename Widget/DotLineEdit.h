#ifndef DOTLINEEDIT_H
#define DOTLINEEDIT_H

#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>

class DotLineEdit : public QLineEdit
{
    Q_OBJECT

public:
    explicit DotLineEdit(QWidget* = NULL);

public slots:
    void numUpdate(QString);
    void validatorSpase(const bool b);
};

#endif // DOTLINEEDIT_H
