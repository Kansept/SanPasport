#include "qdata.h"

QData::QData(double *az, double *rad, size_t sz)
{
    azimuth = (double *)malloc(2*sz*sizeof(double));
    radial = azimuth + sz;
    for (int k=0; k < (int)sz; k++)
    {
        azimuth[k] = az[k];
        radial[k] = rad[k];
    }
    d_size = sz;
}

QData::~QData() {
    free((void *)azimuth);
}

size_t QData::size() const {
    return d_size;
}

QwtPointPolar QData::sample(size_t i) const {
    return QwtPointPolar(azimuth[i],radial[i]);
}

QRectF QData::boundingRect() const
{
    if (d_boundingRect.width() < 0)
        d_boundingRect = qwtBoundingRect(*this);

    return d_boundingRect;
}
