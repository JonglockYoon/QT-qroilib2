

#ifndef MATTOQIMAGE_H
#define MATTOQIMAGE_H

// Qt header files
#include <QtGui>
#include <QDebug>
// OpenCV header files
#include <opencv2/opencv.hpp>

QImage MatToQImage(const cv::Mat&);

#endif // MATTOQIMAGE_H
