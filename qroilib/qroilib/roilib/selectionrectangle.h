/*
 * selectionrectangle.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QGraphicsItem>

namespace Qroilib {

class DocumentView;

/**
 * The rectangle used for indicating the dragged area when selecting items.
 */
class SelectionRectangle : public QGraphicsItem
{
public:
    SelectionRectangle(QGraphicsItem *parent = nullptr);

    void setRectangle(const QRectF &rectangle);

    QRectF boundingRect() const override;
    void paint(QPainter *painter,
               const QStyleOptionGraphicsItem *option,
               QWidget *widget = nullptr) override;

    void setMapDocument(DocumentView *roimap) { mRoiDocument = roimap; }
    DocumentView *mapDocument() const { return mRoiDocument; }

    //void setOffset(QPointF offsetIn) { offset = offsetIn; }
    //void setZoom(qreal zoomIn) { zoom = zoomIn; }

private:
    DocumentView *mRoiDocument;

    //qreal zoom;
    QRectF mRectangle;
    QPointF offset;
};

} // namespace Qroilib
