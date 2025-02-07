#include "colorlookup.h"
#include <QDebug>

namespace {
QVector<QColor> colorLookup_helper(const QVector<QColor>& colors) {
    if (colors.isEmpty()) {
        return QVector<QColor>();
    }

    QVector<QColor> sorted;
    sorted.reserve(colors.size() + 2);
    // Add space for wrap-around colors at start/end
    sorted.append(QColor());

    int lastHue = 0;
    for (int i = 0; i < colors.size(); ++i) {
        const QColor& color = colors[i];
        int hue = color.hue();
        
        if (hue < 0) {
            qWarning() << "color remap table has achromatic color" << color << "at index" << i;
            return QVector<QColor>();
        }

        if (hue < lastHue) {
            qWarning() << "color remap table is not sorted according to hue at index" << i;
            return QVector<QColor>();
        }

        lastHue = hue;
        sorted.append(color);
    }

    // Add wrap-around colors at start/end
    sorted.first() = sorted.last();
    sorted.append(sorted[1]);

    return sorted;
}
}

ColorLookup::ColorLookup(const QVector<QColor>& lookup) 
    : m_hueToSV()
{
    m_hueToSV = colorLookup_helper(lookup);
}

ColorLookup::ColorLookup(const QImage& lookuptable)
    : m_hueToSV()
{
    if (lookuptable.isNull()) {
        return;
    }

    if (lookuptable.format() != QImage::Format_ARGB32 && lookuptable.format() != QImage::Format_RGB32) {
        qWarning() << "Invalid image format for color lookup table" << lookuptable.format();
        return;
    }

    QVector<QColor> lookup;
    lookup.reserve(lookuptable.width());

    const QRgb* scanline = reinterpret_cast<const QRgb*>(lookuptable.scanLine(0));
    for (int x = 0; x < lookuptable.width(); ++x) {
        QColor color = QColor::fromRgb(scanline[x]);
        lookup.append(color.toHsv());
    }

    m_hueToSV = colorLookup_helper(lookup);
}

QColor ColorLookup::remap(const QColor& color) const
{
    if (m_hueToSV.isEmpty() || color.hue() < 0) {
        return color;
    }

    const int hue = color.hue();
    int index = 1;
    
    // Find the color range that contains this hue
    while (index < m_hueToSV.size()-1 && hue > m_hueToSV[index].hue()) {
        ++index;
    }

    const QColor& c1 = m_hueToSV[index-1];
    const QColor& c2 = m_hueToSV[index];

    int h1 = c1.hue();
    int h2 = c2.hue();

    // Handle wrap-around case
    if (hue <= h1) {
        if (hue < h2) {
            h2 -= 360;
        }
    } else if (hue > h1) {
        h2 += 360;
    }

    // Calculate interpolation factor
    double t = (hue - h1) / double(h2 - h1);

    // Interpolate saturation and value
    int s = c1.saturation() * (1.0-t) + c2.saturation() * t;
    int v = c1.value() * (1.0-t) + c2.value() * t;

    return QColor::fromHsv(hue, s, v);
}