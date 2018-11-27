/*
 * abstractobjecttool.cpp
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *Qroilib : QT ROI Lib
 * Copyright 2018 Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>

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

#include "abstractobjecttool.h"

#include "roimap.h"
#include "document.h"
#include "roiobject.h"
#include "roiobjectitem.h"
#include "roirenderer.h"
#include "roiscene.h"
#include "objectgroup.h"
#include "resizeroiobject.h"
#include "utils.h"
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>

#include <QKeyEvent>
#include <QMenu>
#include <QUndoStack>
#include <QDebug>

#include <cmath>

using namespace Qroilib;

AbstractObjectTool::AbstractObjectTool(const QString &name,
                                       const QIcon &icon,
                                       const QKeySequence &shortcut,
                                       QObject *parent)
    : AbstractTool(name, icon, shortcut, parent)
    , mRoiScene(nullptr)
{
}

void AbstractObjectTool::activate(RoiScene *scene)
{
    mRoiScene = scene;
}

void AbstractObjectTool::deactivate(RoiScene *)
{
    mRoiScene = nullptr;
}

void AbstractObjectTool::keyPressed(QKeyEvent *event)
{
    switch (event->key()) {
    //case Qt::Key_PageUp:    raise(); return;
    //case Qt::Key_PageDown:  lower(); return;
    //case Qt::Key_Home:      raiseToTop(); return;
    //case Qt::Key_End:       lowerToBottom(); return;
    case Qt::Key_D:
        if (event->modifiers() & Qt::ControlModifier) {
            duplicateObjects();
            return;
        }
        break;
    }

    event->ignore();
}

void AbstractObjectTool::mouseLeft()
{
    setStatusInfo(QString());
}

void AbstractObjectTool::mouseMoved(const QPointF &pos,
                                    Qt::KeyboardModifiers)
{
    //QPointF offset = mapDocument()->imageView()->imageOffset();
    //const QPointF pt = mapDocument()->imageView()->scrollPos();

    // Take into account the offset of the current layer
    QPointF offsetPos = pos;
    if (Layer *layer = currentLayer())
        offsetPos -= layer->totalOffset();

#if 1
    //const QPoint pixelPos = offsetPos.toPoint();
    //qDebug() << "AbstractObjectTool pixelPos = " << pixelPos.x()-pt.x()+offset.x() << pixelPos.y()-pt.y()+offset.x();
//    const QPointF tilePosF = mapDocument()->renderer()->screenToRoiCoords(offsetPos);
//    const int x = (int) std::floor(tilePosF.x());
//    const int y = (int) std::floor(tilePosF.y());
//    setStatusInfo(QString(QLatin1String("%1, %2 (%3, %4)")).arg(x).arg(y).arg(pixelPos.x()).arg(pixelPos.y()));
#endif
}

void AbstractObjectTool::mousePressed(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::RightButton) {
        showContextMenu(topMostObjectItemAt(event->scenePos()),
                        event->screenPos());
    }
}

void AbstractObjectTool::updateEnabledState()
{
    setEnabled(currentObjectGroup() != nullptr);
}

ObjectGroup *AbstractObjectTool::currentObjectGroup() const
{
    if (!mapDocument())
        return nullptr;

    DocumentView *pDoc = mapDocument();
    return dynamic_cast<ObjectGroup*>(pDoc->currentLayer());
}

QList<RoiObjectItem*> AbstractObjectTool::objectItemsAt(QPointF pos) const
{
    const QList<QGraphicsItem *> &items = mRoiScene->items(pos);

    QList<RoiObjectItem*> objectList;
    for (auto item : items) {
        if (RoiObjectItem *objectItem = qgraphicsitem_cast<RoiObjectItem*>(item))
            objectList.append(objectItem);
    }
    return objectList;
}

RoiObjectItem *AbstractObjectTool::topMostObjectItemAt(QPointF pos) const
{
    const QList<QGraphicsItem *> &items = mRoiScene->items(pos);

    for (QGraphicsItem *item : items) {
        if (RoiObjectItem *objectItem = qgraphicsitem_cast<RoiObjectItem*>(item))
            return objectItem;
    }
    return nullptr;
}

void AbstractObjectTool::duplicateObjects()
{
    mapDocument()->duplicateObjects(mapDocument()->selectedObjects());
}

void AbstractObjectTool::removeObjects()
{
    mapDocument()->removeObjects(mapDocument()->selectedObjects());
}

void AbstractObjectTool::resetTileSize()
{
}

void AbstractObjectTool::flipHorizontally()
{
//    mapDocument()->flipSelectedObjects(FlipHorizontally);
}

void AbstractObjectTool::flipVertically()
{
//    mapDocument()->flipSelectedObjects(FlipVertically);
}

void AbstractObjectTool::raise()
{
//    RaiseLowerHelper(mRoiScene).raise();
}

void AbstractObjectTool::lower()
{
//    RaiseLowerHelper(mRoiScene).lower();
}

void AbstractObjectTool::raiseToTop()
{
//    RaiseLowerHelper(mRoiScene).raiseToTop();
}

void AbstractObjectTool::lowerToBottom()
{
//    RaiseLowerHelper(mRoiScene).lowerToBottom();
}

/**
 * Shows the context menu for roimap objects. The menu allows you to duplicate and
 * remove the roimap objects, or to edit their properties.
 */
void AbstractObjectTool::showContextMenu(RoiObjectItem *clickedObjectItem,
                                         QPoint screenPos)
{
#if 0
    QSet<RoiObjectItem *> selection = mRoiScene->selectedObjectItems();
    if (clickedObjectItem && !selection.contains(clickedObjectItem)) {
        selection.clear();
        selection.insert(clickedObjectItem);
        mRoiScene->setSelectedObjectItems(selection);
    }
    if (selection.isEmpty())
        return;

    const QList<RoiObject*> &selectedObjects = mapDocument()->selectedObjects();
    const QList<ObjectGroup*> objectGroups = mapDocument()->roimap()->objectGroups();

    QMenu menu;
    QAction *duplicateAction = menu.addAction(tr("Duplicate %n Object(s)", "", selection.size()),
                                              this, SLOT(duplicateObjects()));
    QAction *removeAction = menu.addAction(tr("Remove %n Object(s)", "", selection.size()),
                                           this, SLOT(removeObjects()));

    duplicateAction->setIcon(QIcon(QLatin1String(":/images/16x16/stock-duplicate-16.png")));
    removeAction->setIcon(QIcon(QLatin1String(":/images/16x16/edit-delete.png")));

    bool anyTileObjectSelected = std::any_of(selectedObjects.begin(),
                                             selectedObjects.end(),
                                             isTileObject);

    if (anyTileObjectSelected) {
        auto resetTileSizeAction = menu.addAction(tr("Reset Tile Size"), this, SLOT(resetTileSize()));
        resetTileSizeAction->setEnabled(std::any_of(selectedObjects.begin(),
                                                    selectedObjects.end(),
                                                    isResizedTileObject));
    }

    menu.addSeparator();
    menu.addAction(tr("Flip Horizontally"), this, SLOT(flipHorizontally()), QKeySequence(tr("X")));
    menu.addAction(tr("Flip Vertically"), this, SLOT(flipVertically()), QKeySequence(tr("Y")));

//    ObjectGroup *objectGroup = RaiseLowerHelper::sameObjectGroup(selection);
//    if (objectGroup && objectGroup->drawOrder() == ObjectGroup::IndexOrder) {
//        menu.addSeparator();
//        menu.addAction(tr("Raise Object"), this, SLOT(raise()), QKeySequence(tr("PgUp")));
//        menu.addAction(tr("Lower Object"), this, SLOT(lower()), QKeySequence(tr("PgDown")));
//        menu.addAction(tr("Raise Object to Top"), this, SLOT(raiseToTop()), QKeySequence(tr("Home")));
//        menu.addAction(tr("Lower Object to Bottom"), this, SLOT(lowerToBottom()), QKeySequence(tr("End")));
//    }

    if (objectGroups.size() > 1) {
        menu.addSeparator();
        QMenu *moveToLayerMenu = menu.addMenu(tr("Move %n Object(s) to Layer",
                                                 "", selectedObjects.size()));
        for (ObjectGroup *objectGroup : objectGroups) {
            QAction *action = moveToLayerMenu->addAction(objectGroup->name());
            action->setData(QVariant::fromValue(objectGroup));
        }
    }

    menu.addSeparator();
    QIcon propIcon(QLatin1String(":images/16x16/document-properties.png"));
    QAction *propertiesAction = menu.addAction(propIcon,
                                               tr("Object &Properties..."));

    setThemeIcon(removeAction, "edit-delete");
    setThemeIcon(propertiesAction, "document-properties");

    QAction *action = menu.exec(screenPos);
    if (!action)
        return;

//    if (action == propertiesAction) {
//        RoiObject *roiObject = selectedObjects.first();
//        mapDocument()->setCurrentObject(roiObject);
//        emit mapDocument()->editCurrentObject();
//        return;
//    }

//    if (ObjectGroup *objectGroup = action->data().value<ObjectGroup*>()) {
//        mapDocument()->moveObjectsToGroup(mapDocument()->selectedObjects(),
//                                          objectGroup);
//    }
#endif
}
