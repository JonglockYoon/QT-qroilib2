/*
 * variantpropertymanager.cpp
 * Copyright (C) 2006 Trolltech ASA. All rights reserved. (GPLv2)
 * Copyright 2013, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "variantpropertymanager.h"

#include <qroilib/documentview/documentview.h>

#include <QFileInfo>

namespace Qroilib {

VariantPropertyManager::VariantPropertyManager(QObject *parent)
    : QtVariantPropertyManager(parent)
    , mSuggestionsAttribute(QStringLiteral("suggestions"))
    , mMultilineAttribute(QStringLiteral("multiline"))
    , mImageMissingIcon(QStringLiteral("://images/16x16/image-missing.png"))
{
    mImageMissingIcon.addPixmap(QPixmap(QStringLiteral("://images/32x32/image-missing.png")));
}

QVariant VariantPropertyManager::value(const QtProperty *property) const
{
    if (mValues.contains(property))
        return mValues[property].value;
    return QtVariantPropertyManager::value(property);
}

bool VariantPropertyManager::isPropertyTypeSupported(int propertyType) const
{
    if (propertyType == filePathTypeId())
        return true;
    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int VariantPropertyManager::valueType(int propertyType) const
{
    if (propertyType == filePathTypeId())
        return QVariant::String;
    return QtVariantPropertyManager::valueType(propertyType);
}

QStringList VariantPropertyManager::attributes(int propertyType) const
{
    if (propertyType == filePathTypeId()) {
        return QStringList {
            QStringLiteral("filter")
        };
    }
    return QtVariantPropertyManager::attributes(propertyType);
}

int VariantPropertyManager::attributeType(int propertyType,
                                          const QString &attribute) const
{
    if (propertyType == filePathTypeId()) {
        if (attribute == QLatin1String("filter"))
            return QVariant::String;
        return 0;
    }
    return QtVariantPropertyManager::attributeType(propertyType, attribute);
}

QVariant VariantPropertyManager::attributeValue(const QtProperty *property,
                                                const QString &attribute) const
{
    if (mValues.contains(property)) {
        if (attribute == QLatin1String("filter"))
            return mValues[property].filter;
        return QVariant();
    }
    if (mStringAttributes.contains(property)) {
        if (attribute == mSuggestionsAttribute)
            return mStringAttributes[property].suggestions;
        if (attribute == mMultilineAttribute)
            return mStringAttributes[property].multiline;
    }

    return QtVariantPropertyManager::attributeValue(property, attribute);
}

QString VariantPropertyManager::valueText(const QtProperty *property) const
{
    if (mValues.contains(property)) {
        QVariant value = mValues[property].value;
        int typeId = propertyType(property);

        if (typeId == filePathTypeId()) {
            FilePath filePath = value.value<FilePath>();
            QString path = filePath.absolutePath;
            if (path.endsWith(QLatin1Char('/')))
                path.chop(1);
            return QFileInfo(path).fileName();
        }

        return value.toString();
    }

     return QtVariantPropertyManager::valueText(property);
}

QIcon VariantPropertyManager::valueIcon(const QtProperty *property) const
{
    if (mValues.contains(property)) {
        QVariant value = mValues[property].value;
        QString filePath;
        int typeId = propertyType(property);

        if (typeId == filePathTypeId())
            filePath = value.value<FilePath>().absolutePath;

        // TODO: This assumes the file path is an image reference. It should be
        // replaced with a more generic icon.
        if (filePath.isEmpty() || !QFile::exists(filePath))
            return QIcon::fromTheme(QLatin1String("image-missing"), mImageMissingIcon);
        else
            return mIconProvider.icon(QFileInfo(filePath));
    }

    return QtVariantPropertyManager::valueIcon(property);
}

void VariantPropertyManager::setValue(QtProperty *property, const QVariant &val)
{
    if (mValues.contains(property)) {
        Data d = mValues[property];
        if (d.value == val)
            return;
        d.value = val;
        mValues[property] = d;
        emit propertyChanged(property);
        emit valueChanged(property, val);
        return;
    }
    QtVariantPropertyManager::setValue(property, val);
}

void VariantPropertyManager::setAttribute(QtProperty *property,
                                          const QString &attribute,
                                          const QVariant &val)
{
    if (mValues.contains(property)) {
        if (attribute == QLatin1String("filter")) {
            if (val.type() != QVariant::String && !val.canConvert(QVariant::String))
                return;
            QString str = val.toString();
            Data d = mValues[property];
            if (d.filter == str)
                return;
            d.filter = str;
            mValues[property] = d;
            emit attributeChanged(property, attribute, str);
        }
        return;
    }

    if (mStringAttributes.contains(property)) {
        if (attribute == mSuggestionsAttribute) {
            mStringAttributes[property].suggestions = val.toStringList();
            return;
        }
        if (attribute == mMultilineAttribute) {
            mStringAttributes[property].multiline = val.toBool();
            return;
        }
    }

    QtVariantPropertyManager::setAttribute(property, attribute, val);
}

void VariantPropertyManager::initializeProperty(QtProperty *property)
{
    const int type = propertyType(property);
    if (type == filePathTypeId())
        mValues[property] = Data();
    else if (type == QVariant::String)
        mStringAttributes[property] = StringAttributes();

    QtVariantPropertyManager::initializeProperty(property);
}

void VariantPropertyManager::uninitializeProperty(QtProperty *property)
{
    mValues.remove(property);
    mStringAttributes.remove(property);
    QtVariantPropertyManager::uninitializeProperty(property);
}

} // namespace Qroilib
