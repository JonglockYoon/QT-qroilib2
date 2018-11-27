/*
 * properties.cpp
 * Copyright 2010, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 *
 * This file is part of qroilib.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *    1. Redistributions of source code must retain the above copyright notice,
 *       this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "properties.h"

#include <QColor>

namespace Qroilib {

void Properties::merge(const Properties &other)
{
    // Based on QMap::unite, but using insert instead of insertMulti
    const_iterator it = other.constEnd();
    const const_iterator b = other.constBegin();
    while (it != b) {
        --it;
        insert(it.key(), it.value());
    }
}


QString typeToName(int type)
{
    switch (type) {
    case QVariant::String:
        return QStringLiteral("string");
    case QVariant::Double:
        return QStringLiteral("float");
    case QVariant::Color:
        return QStringLiteral("color");
    default:
        if (type == filePathTypeId())
            return QStringLiteral("file");
    }
    return QLatin1String(QVariant::typeToName(type));
}

int nameToType(const QString &name)
{
    if (name == QLatin1String("string"))
        return QVariant::String;
    if (name == QLatin1String("float"))
        return QVariant::Double;
    if (name == QLatin1String("color"))
        return QVariant::Color;
    if (name == QLatin1String("file"))
        return filePathTypeId();

    return QVariant::nameToType(name.toLatin1().constData());
}

static QString colorToString(const QColor &color)
{
    if (!color.isValid())
        return QString();

    return color.name(QColor::HexArgb);
}

QVariant toExportValue(const QVariant &value)
{
    int type = value.userType();

    if (type == QVariant::Color)
        return colorToString(value.value<QColor>());
    if (type == filePathTypeId())
        return value.value<FilePath>().absolutePath;

    return value;
}

int filePathTypeId()
{
    return qMetaTypeId<FilePath>();
}

QVariant fromExportValue(const QVariant &value, int type)
{
    if (type == QVariant::Invalid)
        return value;

    if (value.userType() == type)
        return value;

    if (type == filePathTypeId())
        return QVariant::fromValue(FilePath { value.toString() });

    QVariant variant(value);
    variant.convert(type);
    return variant;
}

} // namespace Qroilib
