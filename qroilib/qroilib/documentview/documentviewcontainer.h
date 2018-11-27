// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2011 Aurélien Gâteau <agateau@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Cambridge, MA 02110-1301, USA.

*/
#ifndef DOCUMENTVIEWCONTAINER_H
#define DOCUMENTVIEWCONTAINER_H

#include <roilib_export.h>

// Local
#include <qroilib/documentview/documentview.h>

// Qt
#include <QGraphicsView>
#include <QUrl>

namespace Qroilib
{

class Document;

struct DocumentViewContainerPrivate;
/**
 * A container for DocumentViews which will arrange them to make best use of
 * available space.
 */
class ROIDSHARED_EXPORT DocumentViewContainer : public QGraphicsView
{
    Q_OBJECT
public:
    DocumentViewContainer(QWidget* parent = 0);
    ~DocumentViewContainer();

    /**
     * Create a DocumentView in the DocumentViewContainer scene
     */
    DocumentView* createView(const QUrl url);

    /**
     * Delete view. Note that the view will first be faded to black before
     * being destroyed.
     */
    void deleteView(DocumentView* view);

    /**
     * Immediately delete all views
     */
    void reset();

    /**
     * Returns saved Setup configuration for a previously viewed document
     */
    //DocumentView::Setup savedSetup(const QUrl &url) const;

    /**
     * Updates setupForUrl hash with latest setup values
     */
    void updateSetup(DocumentView* view);

    void showMessageWidget(QGraphicsWidget*, Qt::Alignment);

public Q_SLOTS:
    void resizeUpdateLayout();
    void updateLayout();
private slots:
    void adjustScale(qreal scale);

protected:
    void showEvent(QShowEvent*) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent*) Q_DECL_OVERRIDE;

private:
    friend class ViewItem;
    DocumentViewContainerPrivate* const d;

};

} // namespace

#endif /* DOCUMENTVIEWCONTAINER_H */
