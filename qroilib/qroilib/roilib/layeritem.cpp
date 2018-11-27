/*
 * layeritem.cpp
 * Copyright 2017, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "layeritem.h"

#include "layer.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

namespace Qroilib {

LayerItem::LayerItem(Layer *layer, QGraphicsItem *parent)
    : QGraphicsItem(parent)
    , mLayer(layer)
{
    setOpacity(layer->opacity());
    //setPos(layer->offset());
}

QRectF LayerItem::boundingRect() const
{
    return QRectF();
//    QSizeF size = mapDocument()->size();
//    const qreal zoom = mapDocument()->zoom();
//    return QRectF(QPointF(0,0), size*zoom);
}


} // namespace Qroilib
