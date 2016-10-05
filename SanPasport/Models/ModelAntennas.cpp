#include "ModelAntennas.h"

ModelAntennas::ModelAntennas(QObject * parent, QSqlDatabase db)
    : QSqlTableModel(parent, db)
{
}

ModelAntennas::~ModelAntennas()
{
}

QVariant ModelAntennas::data(const QModelIndex & index, int role) const
{
    QVariant value = QSqlTableModel::data(index, role);
    // цвет текста
    if (role == Qt::TextColorRole) {
        QModelIndex controlIndex ;
        controlIndex = index.sibling(index.row(), 1);
        // Выключенный элемент
        if (controlIndex.data(Qt::DisplayRole).toString() == QObject::trUtf8("0"))
            return QColor(150,150,150);
    }
    if (role == Qt::BackgroundColorRole){
        QModelIndex controlIndex ;
        controlIndex = index.sibling(index.row(), 1);
        // Выключенный элемент
        if (controlIndex.data(Qt::DisplayRole).toString() == QObject::trUtf8("0"))
            return QColor(250,250,250);
    }

    return value;
}
