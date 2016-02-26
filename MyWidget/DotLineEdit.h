#ifndef DOTLINEEDIT_H
#define DOTLINEEDIT_H

#include <QLineEdit>
#include <QRegExp>
#include <QRegExpValidator>

class DotLineEdit : public QLineEdit
{
    Q_OBJECT

public:

    explicit DotLineEdit(QWidget* = NULL)
    {
        connect( this, SIGNAL(textChanged(QString)), this, SLOT(numUpdate(QString)) );

        QRegExpValidator *cardvalidator = new QRegExpValidator;
        QRegExp rxcard("-?[0-9]*[,/./]{1}[0-9]{4}");
        cardvalidator = new QRegExpValidator(rxcard, this);
        setValidator(cardvalidator);
    }

public slots:
    void numUpdate(QString)
    {
        int numPosition(cursorPosition());
        setText( text().replace(",",".") );
        setCursorPosition(numPosition);
    }

    void validatorSpase(const bool b)
    {
        if(b)
        {
            QRegExpValidator *cardvalidator = new QRegExpValidator;
            QRegExp rxcard("[0-9\\s,/./]*");
            cardvalidator = new QRegExpValidator(rxcard, this);
            setValidator(cardvalidator);
        }
    }
};

#endif // DOTLINEEDIT_H
