#ifndef SAILFISHSILICABACKGROUND_H
#define SAILFISHSILICABACKGROUND_H

#include <QObject>
#include <QImage>
#include <QString>
#include <QRectF>

class SailfishSilicaBackground {
public:
    // Constructors
    explicit SailfishSilicaBackground();
    explicit SailfishSilicaBackground(const QString& path);
    virtual ~SailfishSilicaBackground();

    // Image processing methods
    void curves(QImage* image);
    void darken(QImage* image);
    void lighten(QImage* image);
    void darkenMore(QImage* image);
    void saturate(QImage* image);
    void addNoise(QImage* image);
    void blur(QImage* image);
    
    // Background generation
    QImage backgroundTexture();
    QImage getAppBackground(const QString& path, const QRectF& rect);
    void buildBackgroundImageBase(const QImage& inputImage,
        QImage& outputImage, const QImage& texture, const QRectF& appRect);
    void processAppWallpaper(QImage* image);

    // Property setters
    void setWhiteLevel(double level);
    void setPixelRatio(double ratio);
    void setBlurRounds(int rounds);
    void setBlurRadius(int radius); 
    void setBlurSigma(double sigma);

    // Property getters
    QString outputPath() const;
    QString appImagePath() const;
    double appScaleFactor() const;

    // Static helper for portrait mode
    static void buildBackgroundImageForPortrait(
        SailfishSilicaBackground* filter, const QImage& inputImage,
        QString inputImagePath, const QImage& texture, const QRectF& appRect);

private:
    // Internal image processing
    int extractMeanValue(const QImage& image);

    // Member variables
    QString m_outputPath;
    QString m_appImagePath;
    double m_whiteLevel;
    double m_pixelRatio;
    int m_blurRounds;
    int m_blurRadius;
    double m_blurSigma;
    uint8_t m_curveLookup[256];

};

#endif // SAILFISHSILICABACKGROUND_H