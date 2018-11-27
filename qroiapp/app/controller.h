

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "capturethread.h"
#include "processingthread.h"

// Qt header files
#include <QtGui>
// OpenCV header files

class ImageBuffer;

class Controller : public QObject
{
    Q_OBJECT

public:
    Controller(int windowNr);
    ~Controller();
    int windowNumber;
    int deviceNumber;
    ImageBuffer *imageBuffer;
    ProcessingThread *processingThread;
    CaptureThread *captureThread;
    bool connectToCamera(int,int,bool,int,int);
    void disconnectCamera();
    void stopCaptureThread();
    void stopProcessingThread();
    void deleteCaptureThread();
    void deleteProcessingThread();
    void clearImageBuffer();
    void deleteImageBuffer();
private:
    int imageBufferSize;
};

#endif // CONTROLLER_H
