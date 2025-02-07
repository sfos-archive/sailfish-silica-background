/****************************************************************************************
**
** Copyright (C) 2014 Jolla Ltd.
** Contact: Mikko Harju <mikko.harju@jolla.com>
** All rights reserved.
**
** This file is part of Sailfish Silica UI component package.
**
** You may use this file under the terms of the GNU Lesser General
** Public License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file license.lgpl included in the packaging
** of this file.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
** Lesser General Public License for more details.
**
****************************************************************************************/

#ifndef COLORLOOKUP_H
#define COLORLOOKUP_H

#include <QVector>
#include <QColor>
#include <QImage>

class ColorLookup
{
public:
    ColorLookup(const QVector<QColor> &lookup);
    ColorLookup(const QImage &lookuptable);

    QColor remap(const QColor &color) const;

private:
    QVector<QColor> m_hueToSV;
};

#endif // COLORLOOKUP_H
