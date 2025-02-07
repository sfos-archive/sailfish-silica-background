#ifndef GAUSSIANBLURCALCULATOR_H
#define GAUSSIANBLURCALCULATOR_H

#include <QImage>
#include <QRunnable>
#include <QMutex>

class GaussianBlurCalculator {
private:
    int m_radius;      // Kernel radius
    int m_kernelSize;  // Total kernel size (2*radius - 1)
    int* m_weights;    // Gaussian kernel weights
    int* m_runningSums;       // Running sums for normalization

    // Task class for parallel processing
    class BlurTask : public QRunnable {
    private:
        GaussianBlurCalculator* m_calc;
        const QImage* m_source;
        QImage* m_dest;
        int m_startLine;
        int m_lineCount;

        virtual void run() override;

    public:
        friend class GaussianBlurCalculator;
        BlurTask(GaussianBlurCalculator* calc, const QImage* src,
                 QImage* dst, int start, int count);
    };

public:
    // Constructor - initializes Gaussian kernel
    GaussianBlurCalculator(int radius, double sigma);
    
    // Destructor - cleans up arrays
    ~GaussianBlurCalculator() {
        delete[] m_weights;
        delete[] m_runningSums;
    }

    // Blur methods
    void blurAndDownsample(const QImage* src, QImage* dst, int line);
    void blurAndTranspose(const QImage* src, QImage* dst);
};

#endif // GAUSSIANBLURCALCULATOR_H