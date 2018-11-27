// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2008 Aurélien Gâteau <agateau@kde.org>

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
#ifndef ABSTRACTDOCUMENTVIEWADAPTER_H
#define ABSTRACTDOCUMENTVIEWADAPTER_H

#include <roilib_export.h>

// Qt
#include <QObject>
#include <QPoint>

// Local
#include <qroilib/document/document.h>

class QCursor;
class QGraphicsWidget;
class QRectF;

namespace Qroilib
{

class ImageView;
class RasterImageView;

/**
 * Classes inherit from this class so that they can be used inside the
 * DocumentPanel.
 */
class ROIDSHARED_EXPORT AbstractDocumentViewAdapter : public QObject
{
    Q_OBJECT
public:
    AbstractDocumentViewAdapter();
    virtual ~AbstractDocumentViewAdapter();

    QGraphicsWidget* widget() const
    {
        return mWidget;
    }

    virtual MimeTypeUtils::Kind kind() const = 0;

    virtual ImageView* imageView() const
    {
        return 0;
    }

    virtual RasterImageView* rasterImageView() const
    {
        return 0;
    }

    virtual QCursor cursor() const;

    virtual void setCursor(const QCursor&);

    /**
     * @defgroup zooming functions
     * @{
     */
    virtual bool canZoom() const
    {
        return false;
    }

    // Implementation must emit zoomToFitChanged()
    virtual void setZoomToFit(bool)
    {}

    virtual bool zoomToFit() const
    {
        return false;
    }

    virtual qreal zoom() const
    {
        return 0;
    }

    virtual void setZoom(qreal /*zoom*/, const QPointF& /*center*/ = QPointF(-1, -1))
    {}

    virtual qreal computeZoomToFit() const
    {
        return 1.;
    }
    /** @} */

    virtual Document::Ptr document() const = 0;
    virtual void setDocument(Document::Ptr) = 0;

    virtual void loadConfig()
    {}

    virtual QPointF scrollPos() const
    {
        return QPointF(0, 0);
    }
    virtual void setScrollPos(const QPointF& /*pos*/)
    {}

    /**
     * Rectangle within the item which is actually used to show the document.
     * In item coordinates.
     */
    virtual QRectF visibleDocumentRect() const;

protected:
    void setWidget(QGraphicsWidget* widget)
    {
        mWidget = widget;
    }

Q_SIGNALS:
    /**
     * @addgroup zooming functions
     * @{
     */
    void zoomChanged(qreal);

    void zoomToFitChanged(bool);

    void zoomInRequested(const QPointF&);

    void zoomOutRequested(const QPointF&);
    /** @} */

    void scrollPosChanged();

    /**
     * Emitted when the adapter is done showing the document for the first time
     */
    void completed();

    //void previousImageRequested();

    //void nextImageRequested();

    //void toggleFullScreenRequested();

private:
    QGraphicsWidget* mWidget;
};

/**
 * An empty adapter, used when no document is displayed
 */
class EmptyAdapter : public AbstractDocumentViewAdapter
{
    Q_OBJECT
public:
    EmptyAdapter();
    virtual MimeTypeUtils::Kind kind() const Q_DECL_OVERRIDE
    {
        return MimeTypeUtils::KIND_UNKNOWN;
    }
    virtual Document::Ptr document() const Q_DECL_OVERRIDE
    {
        return Document::Ptr();
    }
    virtual void setDocument(Document::Ptr) Q_DECL_OVERRIDE
    {}
};

} // namespace

#endif /* ABSTRACTDOCUMENTVIEWADAPTER_H */
