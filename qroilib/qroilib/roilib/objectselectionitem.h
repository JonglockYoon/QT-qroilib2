/*
 * objectselectionitem.h
 * Copyright 2015-2016, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include <QGraphicsObject>
#include <QHash>

namespace Qroilib {

//class GroupLayer;
class Layer;
class RoiObject;
class DocumentView;

//class MapDocument;
class RoiObjectLabel;
class RoiObjectOutline;

class ObjectSelectionItem : public QGraphicsObject
{
    Q_OBJECT

public:
    ObjectSelectionItem(DocumentView *mapDocument);

    // QGraphicsItem interface
    QRectF boundingRect() const override { return QRectF(); }
    void paint(QPainter *, const QStyleOptionGraphicsItem *, QWidget *) override {}

void addObjectOutline(RoiObject *roiObject);

private slots:
    void selectedObjectsChanged();
    void mapChanged();
    void layerAdded(Layer *layer);
    void syncOverlayItems(const QList<RoiObject *> &objects);
    void updateObjectLabelColors();
    void objectsAdded(const QList<RoiObject*> &objects);
    void objectsRemoved(const QList<RoiObject*> &objects);

    void objectLabelVisibilityChanged();

private:
    void addRemoveObjectLabels();
    void addRemoveObjectOutlines();

    DocumentView *mRoiDocument;
    QHash<RoiObject*, RoiObjectLabel*> mObjectLabels;
    QHash<RoiObject*, RoiObjectOutline*> mObjectOutlines;
};

} // namespace Qroilib
