/*
 * moveroiobjecttogroup.h
 * Copyright 2010, Jeff Bland <jksb@member.fsf.org>
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

#include <QUndoCommand>

namespace Qroilib {

class RoiObject;
class ObjectGroup;
class DocumentView;

class MoveRoiObjectToGroup : public QUndoCommand
{
public:
    MoveRoiObjectToGroup(DocumentView *mapDocument,
                         RoiObject *roiObject,
                         ObjectGroup *objectGroup);

    void undo() override;
    void redo() override;

private:
    DocumentView *mRoiDocument;
    RoiObject *mRoiObject;
    ObjectGroup *mOldObjectGroup;
    ObjectGroup *mNewObjectGroup;
};

} // namespace Qroilib
