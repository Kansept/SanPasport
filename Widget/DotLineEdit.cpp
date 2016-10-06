#include "DotLineEdit.h"


DotLineEdit::DotLineEdit(QWidget *)
{
    connect( this, SIGNAL(textChanged(QString)), this, SLOT(numUpdate(QString)) );

    QRegExpValidator *cardvalidator = new QRegExpValidator;
    QRegExp rxcard("-?[0-9]*[,/./]{1}[0-9]{4}");
    cardvalidator = new QRegExpValidator(rxcard, this);
    setValidator(cardvalidator);
}


void DotLineEdit::numUpdate(QString)
{
    int numPosition(cursorPosition());
    setText( text().replace(",",".") );
    setCursorPosition(numPosition);
}


void DotLineEdit::validatorSpase(const bool b)
{
    if(b) {
        QRegExpValidator *cardvalidator = new QRegExpValidator;
        QRegExp rxcard("[0-9\\s,/./]*");
        cardvalidator = new QRegExpValidator(rxcard, this);
        setValidator(cardvalidator);
    }
}
