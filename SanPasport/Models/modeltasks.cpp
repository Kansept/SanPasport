#include "ModelTasks.h"

 ModelTasks::ModelTasks(QObject * parent, QSqlDatabase db)
     : QSqlTableModel(parent,db)
 {
 }

 ModelTasks::~ModelTasks()
 {
 }

 QVariant ModelTasks::data(const QModelIndex &index, int role) const
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
     // Цвет фона
     if (role == Qt::BackgroundColorRole) {
         QModelIndex controlIndex ;
         controlIndex= index.sibling(index.row(), 1);
         // Выключенный элемент
         if (controlIndex.data(Qt::DisplayRole).toString() == QObject::trUtf8("0"))
             return QColor(250,250,250);

         controlIndex = index.sibling(index.row(), 2);
         // Горизонтальное сечение
         if (controlIndex.data(Qt::DisplayRole).toString() == QObject::trUtf8("1"))
             return QColor(255,230,219);
         // Вертикальное сечение
         else if (controlIndex.data(Qt::DisplayRole).toString() == QObject::trUtf8("2"))
             return QColor(219,247,255);
         // Точка
         else if (controlIndex.data(Qt::DisplayRole).toString() == QObject::trUtf8("3"))
             return QColor(226,255,219);
     }

     return value;
 }
