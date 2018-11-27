/*
 * createpolygonobjecttool.cpp
 * Copyright 2014, Martin Ziel <martin.ziel.com>
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

#include "createpolygonobjecttool.h"

#include "roiobject.h"
#include "roiobjectitem.h"
#include "utils.h"

using namespace Qroilib;

CreatePolygonObjectTool::CreatePolygonObjectTool(QObject *parent)
    : CreateMultipointObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/24x24/insert-polygon.png")));
    languageChanged();
}

void CreatePolygonObjectTool::languageChanged()
{
    setName(tr("Insert Polygon"));
    setShortcut(QKeySequence(tr("P")));
}

RoiObject *CreatePolygonObjectTool::createNewRoiObject()
{
    RoiObject *newRoiObject = new RoiObject;
    newRoiObject->setShape(RoiObject::Polygon);
    return newRoiObject;
}

void CreatePolygonObjectTool::finishNewRoiObject()
{
    if (mNewRoiObjectItem->roiObject()->polygon().size() >= 3)
        CreateObjectTool::finishNewRoiObject();
    else
        CreateObjectTool::cancelNewRoiObject();
}
