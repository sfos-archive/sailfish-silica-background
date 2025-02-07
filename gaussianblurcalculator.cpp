#include "gaussianblurcalculator.h"

#include <cmath>
#include <cstdint>
#include <QImage>
#include <QThreadPool>

GaussianBlurCalculator::GaussianBlurCalculator(int radius, double sigma) :
    m_radius(radius),
    m_kernelSize(2 * radius - 1)
{
    m_weights = new int[m_kernelSize];
    m_runningSums = new int[m_kernelSize + 1];
    double* gaussianValues = new double[radius];

    double twoSigmaSq = 2.0 * sigma * sigma;
    double norm = 1.0 / (std::sqrt(2.0 * M_PI) * sigma);

    // Compute Gaussian values
    for (int i = 0; i < radius; i++) {
        double x = static_cast<double>(i - (radius - 1));
        gaussianValues[i] = exp(-(x * x) / twoSigmaSq) * norm;
    }

    // Scale to [1-257] range
    const double scalar = 257.0 / gaussianValues[radius - 1];

    // Compute Gaussian kernel weights
    for (int i = 0; i < m_kernelSize; i++) {
        double value = (i < radius) ? gaussianValues[i] : gaussianValues[m_kernelSize - i - 1];
        m_weights[i] = static_cast<int>(std::round(value * scalar));
    }

    // Compute running sums in reverse order
    m_runningSums[m_kernelSize] = 0;
    for (int i = m_kernelSize - 1; i >= 0; i--) {
        m_runningSums[i] = m_runningSums[i + 1] + m_weights[i];
    }

    delete[] gaussianValues;
}

void GaussianBlurCalculator::BlurTask::run()
{
    // Process lines in range
    for (int y = m_startLine; y < m_startLine + m_lineCount; y++) {
        m_calc->blurAndDownsample(m_source, m_dest, y);
    }
}

GaussianBlurCalculator::BlurTask::BlurTask(GaussianBlurCalculator* calc, const QImage* src,
                                           QImage* dst, int start, int count)
    : QRunnable(), 
      m_calc(calc), m_source(src), m_dest(dst), m_startLine(start), m_lineCount(count)
{
    setAutoDelete(false);
}

void GaussianBlurCalculator::blurAndDownsample(const QImage* source, QImage* dest, int row) {
    // Get source row data
    int sourceWidth = source->width();
    const auto* srcLine = reinterpret_cast<const QRgb*>(source->scanLine(row));
    
    // Get destination base pointer for column-wise writing
    auto* destBits = reinterpret_cast<QRgb*>(dest->bits());
    int destWidth = dest->width();
    int destHeight = dest->height();
    
    int kernelOffset = m_radius - 1;

    // Process each pixel in the row and write transposed
    for (int x = 0; x < std::min(sourceWidth, destHeight); ++x) {
        // Calculate kernel bounds
        int kernelStart = std::max(0, (kernelOffset - x));
        int kernelEnd = std::min(sourceWidth - x, m_kernelSize);

        // Calculate weighted sum of pixels
        uint64_t redSum = 0, greenSum = 0, blueSum = 0;
        uint32_t totalWeight = m_runningSums[kernelStart] - m_runningSums[kernelEnd];

        for (int k = kernelStart; k < kernelEnd; ++k) {
            QRgb pixel = srcLine[std::max(0, x - kernelOffset + k)];
            int weight = m_weights[k];
            
            redSum += qRed(pixel) * weight;
            greenSum += qGreen(pixel) * weight;
            blueSum += qBlue(pixel) * weight;
        }

        // Write to transposed position: (x, y) becomes (y, x)
        QRgb outputPixel = qRgb(
            redSum / totalWeight,
            greenSum / totalWeight,
            blueSum / totalWeight
        );
        destBits[row + x * destWidth] = outputPixel;
    }
}

void GaussianBlurCalculator::blurAndTranspose(const QImage* src, QImage* dst)
{
    // Quick validation
    if (!src || src->isNull()) {
        return;
    }

    // Setup thread pool for parallel processing
    QThreadPool threadPool;
    int threadCount = QThread::idealThreadCount();
    int linesPerThread = src->height() / threadCount;

    // Create list to track blur tasks
    QList<BlurTask*> tasks;
    int startLine = 0;

    // Create and queue blur tasks for all but last section
    for (int i = 0; i < threadCount - 1; i++) {
        BlurTask* task = new BlurTask(this, src, dst, startLine, linesPerThread);
        tasks.append(task);
        threadPool.start(task, 0);

        startLine += linesPerThread;
    }
    
    // Process remaining lines directly
    for (int y = startLine; y < src->height(); y++) {
        blurAndDownsample(src, dst, y);
    }

    // Wait for tasks to complete
    threadPool.waitForDone();

    // Cleanup tasks
    qDeleteAll(tasks);
}
