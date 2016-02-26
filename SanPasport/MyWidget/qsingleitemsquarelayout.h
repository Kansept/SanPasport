/*
 * Copyright (c) 2009 Nokia Corporation.
 */

#ifndef QASPECTRATIOLAYOUT_H_
#define QASPECTRATIOLAYOUT_H_

#include <QLayout>
#include <QPointer>
#include <QRect>
#include <QWidgetItem>
#include <QLayoutItem>

/**
 * QSingleItemSquareLayout is a layout which can hold one item,
 * keep its square form and center it.
 */
class QSingleItemSquareLayout : public QLayout
{
	Q_OBJECT

public:
	QSingleItemSquareLayout(QWidget* parent, int spacing =-1);
	QSingleItemSquareLayout(int spacing = -1);
	~QSingleItemSquareLayout();

	virtual void add(QLayoutItem* item);

	virtual void addItem(QLayoutItem* item);
	virtual void addWidget(QWidget* widget);
	virtual QLayoutItem* takeAt(int index);
	virtual QLayoutItem* itemAt(int index) const;
	virtual int count() const;

	virtual QLayoutItem* replaceItem(QLayoutItem* item);
	virtual QLayoutItem* take();
	virtual bool hasItem() const;

	virtual Qt::Orientations expandingDirections() const;


	virtual void setGeometry(const QRect& rect);
	virtual QRect geometry();

	virtual QSize sizeHint() const;
	virtual QSize minimumSize() const;
	virtual bool hasHeightForWidth() const;

private:

	void setLastReceivedRect(const QRect& rect);
	void init(int spacing);
	QSize calculateProperSize(QSize from) const;

	QPoint calculateCenterLocation(QSize from, QSize itemSize) const;
	bool areRectsEqual(const QRect& a, const QRect& b) const;
	QLayoutItem* item;

	QRect* lastReceivedRect;
	QRect* _geometry;

};

#endif /* QASPECTRATIOLAYOUT_H_ */
