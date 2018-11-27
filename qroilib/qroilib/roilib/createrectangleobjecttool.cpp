/*
 * createrectangleobjecttool.cpp
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

#include "createrectangleobjecttool.h"

#include "roiobject.h"
#include "utils.h"

using namespace Qroilib;

CreateRectangleObjectTool::CreateRectangleObjectTool(QObject *parent)
    : CreateScalableObjectTool(parent)
{
    setIcon(QIcon(QLatin1String(":images/insert-rectangle.png")));
    setThemeIcon(this, "insert-rectangle");
    languageChanged();
}

void CreateRectangleObjectTool::languageChanged()
{
    setName(tr("Insert Rectangle"));
    setShortcut(QKeySequence(tr("R")));
}

RoiObject *CreateRectangleObjectTool::createNewRoiObject()
{
    RoiObject *newRoiObject = new RoiObject;
    newRoiObject->setShape(RoiObject::Rectangle);
    return newRoiObject;
}
