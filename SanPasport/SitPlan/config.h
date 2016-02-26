#ifndef CONFIG_H
#define CONFIG_H

#include <QMetaType>
#include <QString>
#include <QColor>
#include <QFont>
#include <QPen>

class ConfigSitPlan
{
public:

    float ZoPenWidth;
    QColor ZoColor;
    QString ZoScale;
    bool ZoSmooth;

    QString PaperOrientation;
    QString PaperSize;
    float PaperDpi;
    int PaperMargin;

    bool VisibleTitle;
    bool VisibleZo;
    bool VisibleSitPlan;
    bool VisibleGrid;
    bool VisibleRosaW;
    bool SitPlanAuto;

    QFont TitleFont;
    QString TitleFontSize;
    QString TitleText;

    bool GridAxisVisibleLeft;
    bool GridAxisVisibleRight;
    bool GridAxisVisibleBottom;
    bool GridAxisVisibleTop;
    QFont GridAxisFont;
    int GridSize;
    QPen GridPen;
    int GridStep;

};
Q_DECLARE_METATYPE(ConfigSitPlan)

#endif // CONFIG_H
