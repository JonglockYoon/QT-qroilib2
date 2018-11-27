// vim: set tabstop=4 shiftwidth=4 expandtab:
/*
Gwenview: an image viewer
Copyright 2012 Aurélien Gâteau <agateau@kde.org>

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
#ifndef CMSPROFILE_H
#define CMSPROFILE_H

#include <roilib_export.h>

// Local

// Qt
#include <QExplicitlySharedDataPointer>
#include <QSharedData>

// Exiv2
//#include <exiv2/image.hpp>

class QByteArray;
class QString;

typedef void* cmsHPROFILE;

namespace Qroilib
{

namespace Cms
{

struct ProfilePrivate;
/**
 * Wrapper for lcms color profile
 */
class ROIDSHARED_EXPORT Profile : public QSharedData
{
public:
    typedef QExplicitlySharedDataPointer<Profile> Ptr;

    Profile();
    ~Profile();

    QString description() const;
    QString manufacturer() const;
    QString model() const;
    QString copyright() const;

    cmsHPROFILE handle() const;

    static Profile::Ptr loadFromImageData(const QByteArray& data, const QByteArray& format);
//    static Profile::Ptr loadFromExiv2Image(const Exiv2::Image* image);
    static Profile::Ptr getMonitorProfile();
    static Profile::Ptr getSRgbProfile();

private:
    Profile(cmsHPROFILE);
    ProfilePrivate* const d;
};

} // namespace Cms
} // namespace Qroilib

#endif /* CMSPROFILE_H */
