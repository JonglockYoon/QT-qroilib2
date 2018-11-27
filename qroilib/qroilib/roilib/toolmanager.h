/*
 * toolmanager.h
 * Copyright 2009-2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include <QObject>

class QAction;
class QActionGroup;

namespace Qroilib {


class AbstractTool;
class DocumentView;

/**
 * The tool manager provides a central place to register editing tools.
 *
 * It creates actions for the tools that can be placed on a tool bar. All the
 * actions are put into a QActionGroup to make sure only one tool can be
 * selected at a time.
 */
class ToolManager : public QObject
{
    Q_OBJECT

public:
    ToolManager(QObject *parent = nullptr);
    ~ToolManager();

    void setMapDocument(DocumentView *mapDocument);

    QAction *registerTool(AbstractTool *tool);

    void selectTool(AbstractTool *tool);
    AbstractTool *selectedTool() const;

    void retranslateTools();

signals:
    void selectedToolChanged(AbstractTool *tool);

    /**
     * Emitted when the status information of the current tool changed.
     * @see AbstractTool::setStatusInfo()
     */
    void statusInfoChanged(const QString &info);

private slots:
    void actionTriggered(QAction *action);
    void toolEnabledChanged(bool enabled);
    void selectEnabledTool();

private:
    Q_DISABLE_COPY(ToolManager)

    AbstractTool *firstEnabledTool() const;

    QActionGroup *mActionGroup;
    AbstractTool *mSelectedTool;
    AbstractTool *mDisabledTool;
    AbstractTool *mPreviouslyDisabledTool;
    DocumentView *mRoiDocument;

    bool mSelectEnabledToolPending;
public:
    void setSelectedTool(AbstractTool *tool);
};


/**
 * Returns the selected tool.
 */
inline AbstractTool *ToolManager::selectedTool() const
{
    return mSelectedTool;
}

} // namespace Qroilib
