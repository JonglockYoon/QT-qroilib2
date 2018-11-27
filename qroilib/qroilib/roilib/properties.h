/*
 * properties.h
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

#pragma once

#include "roid_global.h"

#include <QMap>
#include <QString>
#include <QVariant>

namespace Qroilib {

struct FilePath {
    QString absolutePath;
};

/**
 * Collection of properties and their values.
 */
class ROIDSHARED_EXPORT Properties : public QMap<QString,QVariant>
{
public:
    void merge(const Properties &other);
};

ROIDSHARED_EXPORT int filePathTypeId();

ROIDSHARED_EXPORT QString typeToName(int type);
ROIDSHARED_EXPORT int nameToType(const QString &name);

ROIDSHARED_EXPORT QVariant toExportValue(const QVariant &value);
ROIDSHARED_EXPORT QVariant fromExportValue(const QVariant &value, int type);

} // namespace Qroilib

Q_DECLARE_METATYPE(Qroilib::FilePath)
