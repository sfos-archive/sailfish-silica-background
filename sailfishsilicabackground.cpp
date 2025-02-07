#include "sailfishsilicabackground.h"

#include <cmath>
#include <mdconfgroup.h>

#include <QDebug>
#include <QDir>
#include <QImageReader>
#include <QImageWriter>
#include <QPainter>

#include "gaussianblurcalculator.h"

SailfishSilicaBackground::SailfishSilicaBackground(const QString& path) :
    m_whiteLevel(-1.0),
    m_pixelRatio(1.0),
    m_blurRounds(5),
    m_blurRadius(4),
    m_blurSigma(1.2),
    m_outputPath(path)
{
    // Read configuration (MDConfGroup silica-background)
    MDConfGroup conf("silica-background");
    m_blurRounds = conf.value("blur_rounds", m_blurRounds).toInt();
    m_blurRadius = conf.value("blur_radius", m_blurRadius).toInt();
    m_whiteLevel = conf.value("white_level", m_whiteLevel).toDouble();
    m_pixelRatio = conf.value("pixel_ratio", m_pixelRatio).toReal();

    // Create output directory if path not empty
    if (!m_outputPath.isEmpty()) {
        QDir dir(m_outputPath);
        if (!dir.mkpath(m_outputPath)) {
            qWarning() << "Failed to create directory:" << dir.path();
        }
    }

    setWhiteLevel(0.7);
}

SailfishSilicaBackground::~SailfishSilicaBackground()
{
}

void SailfishSilicaBackground::curves(QImage* image)
{
    int height = image->height();
    int width = image->width();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image->scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor color = QColor::fromRgb(line[x]);
            color.setHsv(color.hue(), color.saturation(), m_curveLookup[color.value()]);
            line[x] = color.rgb();
        }
    }
}

void SailfishSilicaBackground::darken(QImage* image) 
{
    const double darkenFactor = 0.85;
    int height = image->height();
    int width = image->width();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image->scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pixel = line[x];
            int r = qRed(pixel) * darkenFactor;
            int g = qGreen(pixel) * darkenFactor;
            int b = qBlue(pixel) * darkenFactor;
            line[x] = qRgb(r, g, b);
        }
    }
}

void SailfishSilicaBackground::lighten(QImage* image)
{
    // Not implemented in the original library
}

void SailfishSilicaBackground::darkenMore(QImage* image)
{
    const double darkenFactor = 0.5;
    int height = image->height();
    int width = image->width();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image->scanLine(y));
        for (int x = 0; x < width; ++x) {
            QRgb pixel = line[x];
            int r = qRed(pixel) * darkenFactor;
            int g = qGreen(pixel) * darkenFactor;
            int b = qBlue(pixel) * darkenFactor;
            line[x] = qRgb(r, g, b);
        }
    }
}

void SailfishSilicaBackground::saturate(QImage* image)
{
    const double saturationFactor = 1.5;
    int height = image->height();
    int width = image->width();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image->scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor color = QColor::fromRgb(line[x]);
            int saturation = color.saturation() * saturationFactor;
            saturation = std::min(saturation, 255);
            color.setHsv(color.hue(), saturation, color.value());
            line[x] = color.rgb();
        }
    }
}

void SailfishSilicaBackground::addNoise(QImage* image)
{
    int height = image->height();
    int width = image->width();

    for (int y = 0; y < height; ++y) {
        QRgb* line = reinterpret_cast<QRgb*>(image->scanLine(y));
        for (int x = 0; x < width; ++x) {
            int noise = (rand() % 50) - 25;
            QRgb pixel = line[x];
            int r = std::clamp(qRed(pixel) + noise, 0, 255);
            int g = std::clamp(qGreen(pixel) + noise, 0, 255);
            int b = std::clamp(qBlue(pixel) + noise, 0, 255);
            line[x] = qRgb(r, g, b);
        }
    }
}

void SailfishSilicaBackground::blur(QImage* image)
{
    if (!image) return;

    int width = image->width();
    int height = image->height();

    if (width <= 50 || height < 50) return;

    GaussianBlurCalculator blurCalculator(m_blurRadius, m_blurSigma);
    QImage tempImage;

    for (int i = 0; i < m_blurRounds; ++i) {
        blurCalculator.blurAndTranspose(image, &tempImage);
        blurCalculator.blurAndTranspose(&tempImage, image);
    }
}

QImage SailfishSilicaBackground::backgroundTexture()
{
    QImageReader reader(":/images/graphic-shader-texture.png");
    QImage result = reader.read();
    
    if (result.isNull()) {
        qWarning() << "declarativetheme.cpp::generateWallpapersFrom(imageUrl) - Texture is not available" << "default";
    }
    
    return result;
}

QImage SailfishSilicaBackground::getAppBackground(const QString& path, const QRectF& rect)
{
    QImageReader reader(path);
    const QSize sourceSize = reader.size();
    
    // Calculate aspect ratios
    const double targetAspectRatio = rect.width() / rect.height();
    const double sourceAspectRatio = sourceSize.width() / static_cast<double>(sourceSize.height());
    
    // Calculate clip rect to maintain aspect ratio
    QRect clipRect;
    if (sourceAspectRatio > targetAspectRatio) {
        // Image is wider than needed - clip width
        const int clipWidth = sourceSize.height() * targetAspectRatio;
        const int xOffset = (sourceSize.width() - clipWidth) / 2;
        clipRect.setRect(xOffset, 0, clipWidth, sourceSize.height());
    } else {
        // Image is taller than needed - clip height 
        const int clipHeight = sourceSize.height() / targetAspectRatio;
        const int yOffset = (sourceSize.height() - clipHeight) / 2;
        clipRect.setRect(0, yOffset, sourceSize.width(), clipHeight);
    }
    
    // Set reader parameters and read image
    reader.setClipRect(clipRect);
    reader.setScaledSize(rect.size().toSize());
    return reader.read();
}

void SailfishSilicaBackground::buildBackgroundImageBase(const QImage& inputImage,
        QImage& outputImage, const QImage& texture, const QRectF& appRect)
{
    const double targetWidth = appRect.width() / appScaleFactor();

    if (texture.isNull()) {
        // No texture is supplied, just process and scale the image
        setWhiteLevel(1.0);
        outputImage = inputImage.scaledToWidth(targetWidth);
        processAppWallpaper(&outputImage);
        return;
    }

    // Scale the image to the target width and overlay the texture
    setWhiteLevel(0.4);

    // Process and scale the image
    outputImage = inputImage.copy(appRect.toRect())
                           .scaledToWidth(targetWidth);
    processAppWallpaper(&outputImage);

    // Final touches: scale to target size and apply effects
    outputImage = outputImage.scaledToWidth(appRect.width(), Qt::SmoothTransformation);
    addNoise(&outputImage);

    // Apply semi-transparent texture overlay
    QPainter painter(&outputImage);
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.setOpacity(0.1);
    painter.fillRect(0, 0, outputImage.width(), outputImage.height(), QBrush(texture));
}

void SailfishSilicaBackground::processAppWallpaper(QImage* image)
{
    blur(image);
    curves(image);
    saturate(image);
}

void SailfishSilicaBackground::setWhiteLevel(qreal whiteLevel) 
{
    if (m_whiteLevel == whiteLevel) {
        return;
    }
    
    m_whiteLevel = whiteLevel;
    
    // Calculate multiplier
    double multiplier = whiteLevel / (sin(1.5)/1.5);
    
    // Fill lookup table
    for (int i = 0; i < 256; i++) {
        // Calculate: 255 * multiplier * sin(1.5*i/255) / 1.5
        double sinArg = 1.5 * i / 255.0;
        double sinVal = sin(sinArg);
        double value = 255 * multiplier * sinVal / 1.5;
        
        // Clamp between 0-255 and store in lookup table
        m_curveLookup[i] = std::clamp(static_cast<int>(value), 0, 255);
    }
}

void SailfishSilicaBackground::setPixelRatio(double ratio)
{
    m_pixelRatio = ratio;
}

void SailfishSilicaBackground::setBlurRounds(int rounds)
{
    m_blurRounds = rounds;
}

void SailfishSilicaBackground::setBlurRadius(int radius)
{
    m_blurRadius = radius;
}

void SailfishSilicaBackground::setBlurSigma(double sigma)
{
    m_blurSigma = sigma;
}

QString SailfishSilicaBackground::outputPath() const
{
    return m_outputPath;
}

QString SailfishSilicaBackground::appImagePath() const
{
    return m_appImagePath;
}

double SailfishSilicaBackground::appScaleFactor() const
{
    return std::round(m_pixelRatio * 4.0);
}

int SailfishSilicaBackground::extractMeanValue(const QImage& image)
{
    int totalValue = 0;
    int height = image.height();
    int width = image.width();
    for (int y = 0; y < height; ++y) {
        const QRgb* line = reinterpret_cast<const QRgb*>(image.scanLine(y));
        for (int x = 0; x < width; ++x) {
            QColor color = QColor::fromRgb(line[x]);
            totalValue += color.value();
        }
    }
    return totalValue / (height * width);
}

void SailfishSilicaBackground::buildBackgroundImageForPortrait(
        SailfishSilicaBackground* filter, const QImage& inputImage,
        QString inputImagePath, const QImage& texture, const QRectF& appRect)
{
    if (inputImage.isNull()) {
        return;
    }

    // Generate output path with hash
    QString outputPath = filter->outputPath() + 
                        QString("/%1ap.jpg").arg(qHash(inputImagePath), 0, 16);
    filter->m_appImagePath = outputPath;

    // Create output image
    QImage outputImage;
    
    // Process the image
    filter->buildBackgroundImageBase(inputImage, outputImage, texture, appRect);

    // Save the result with high quality
    QImageWriter writer(outputPath);
    writer.setQuality(95);
    writer.write(outputImage);
}
