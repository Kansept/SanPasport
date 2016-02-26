#ifndef QDATA_H
#define QDATA_H

#include <qwt_series_data.h>

class QData : public QwtSeriesData<QwtPointPolar>
{
public:
    QData(double *,double *,size_t);
    ~QData();

    virtual size_t size() const;
    virtual QwtPointPolar sample(size_t) const;
    virtual QRectF boundingRect() const;

protected:
    double *azimuth;
    double *radial;
    size_t d_size;
};

#endif // QDATA_H
