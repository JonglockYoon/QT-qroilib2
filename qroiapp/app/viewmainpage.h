/*
Gwenview: an image viewer
Copyright 2007 Aurélien Gâteau <agateau@kde.org>

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
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#pragma once

#ifndef VIEWMAINPAGE_H
#define VIEWMAINPAGE_H

// Local
#include <qroilib/document/document.h>

// Qt
#include <QUrl>
#include <QToolButton>
#include <QWidget>
#include <QLabel>

#include "capturethread.h"

class QGraphicsWidget;

//using namespace Qroilib;

#include <qroilib/document/document.h>
#include <qroilib/documentview/abstractdocumentviewadapter.h>
#include <qroilib/documentview/documentview.h>
#include <qroilib/documentview/rasterimageview.h>
#include <qroilib/documentview/documentviewcontainer.h>
#include <qroilib/documentview/documentviewcontroller.h>
#include "roiscene.h"
//#include "roipropertyeditor.h"

class RoiPropertyEditor;
class Controller;

struct ViewMainPagePrivate;

class ImageBuffer;
/**
 * Holds the active document view and associated widgetry.
 */
class ViewMainPage : public QWidget
{
    Q_OBJECT
public:
    static const int MaxViewCount;

    ViewMainPage(QWidget* parent);
    ~ViewMainPage();

    void loadConfig();

    void updateLayout();

    /**
     * Reset the view
     */
    void reset();

    int statusBarHeight() const;

    //virtual QSize sizeHint() const;

    /**
     * Returns the url of the current document, or an invalid url if unknown
     */
    QUrl url() const;


    /**
     * Opens up to MaxViewCount urls, and set currentUrl as the current one
     */
    void openUrls(const QList<QUrl>& urls, const QUrl &currentUrl);

    bool isEmpty() const;

    /**
     * Returns the image view, if the current adapter has one.
     */
    Qroilib::RasterImageView* imageView() const;

    /**
     * Returns the document view
     */
    Qroilib::DocumentView* currentView() const;

    Qroilib::DocumentView* view(int n) const;

Q_SIGNALS:

    /**
     * Emitted when the part has finished loading
     */

    //void previousImageRequested();

    //void nextImageRequested();

    void toggleFullScreenRequested();

    void goToBrowseModeRequested();

public Q_SLOTS:
    //void slotViewFocused(Qroilib::DocumentView*);
    void setCurrentView(Qroilib::DocumentView* view);
    void editRoiProperty();

private Q_SLOTS:
    void completed(int seq);

    void showContextMenu();

    //void updatePlayerUI(const QImage& img, int seq);
    void updateFrame(const QImage &frame, int windowNumber);

private:
    friend struct ViewMainPagePrivate;
    ViewMainPagePrivate* const d;
protected:
    bool eventFilter(QObject *, QEvent *);

public:
    QLabel* mStatusLabel;
    Controller* myCamController[2]; // DocumentView 와 갯수동일하게 유지 필요.
    QObject* objContainer;
    RoiPropertyEditor *roipropertyeditor;
    ImageBuffer *imageBuffer;

    void connectToCamera(int windowNumber,int deviceNumber);
    void disconnectCamera(int windowNumber);

    int imageBufferSize = 1;
};

#endif /* VIEWMAINPAGE_H */
