/*
 * grouplayer.cpp
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

#include "grouplayer.h"

#include "roimap.h"

namespace Qroilib {

GroupLayer::GroupLayer(const QString &name, int x, int y):
    Layer(GroupLayerType, name, x, y)
{
}

GroupLayer::~GroupLayer()
{
    qDeleteAll(mLayers);
}

void GroupLayer::addLayer(Layer *layer)
{
    adoptLayer(layer);
    mLayers.append(layer);
}

void GroupLayer::insertLayer(int index, Layer *layer)
{
    adoptLayer(layer);
    mLayers.insert(index, layer);
}

void GroupLayer::adoptLayer(Layer *layer)
{
    layer->setParentLayer(this);

    if (roimap())
        roimap()->adoptLayer(layer);
    else
        layer->setMap(nullptr);
}

Layer *GroupLayer::takeLayerAt(int index)
{
    Layer *layer = mLayers.takeAt(index);
    layer->setMap(nullptr);
    layer->setParentLayer(nullptr);
    return layer;
}

bool GroupLayer::isEmpty() const
{
    return mLayers.isEmpty();
}

GroupLayer *GroupLayer::clone() const
{
    return initializeClone(new GroupLayer(mName, mX, mY));
}

void GroupLayer::setMap(RoiMap *roimap)
{
    Layer::setMap(roimap);

    if (roimap) {
        for (Layer *layer : mLayers)
            roimap->adoptLayer(layer);
    } else {
        for (Layer *layer : mLayers)
            layer->setMap(nullptr);
    }
}

GroupLayer *GroupLayer::initializeClone(GroupLayer *clone) const
{
    Layer::initializeClone(clone);
    for (const Layer *layer : mLayers)
        clone->addLayer(layer->clone());
    return clone;
}

} // namespace Qroilib
