#pragma once

//#include <roilib_export.h>

#include <QObject>
#include <QPointF>
#include <QRectF>
#include <QVector>

#include "imgprocbase.h"
#include "recipedata.h"
#include <opencv2/xfeatures2d/nonfree.hpp>

namespace tesseract {
class TessBaseAPI;
}

typedef struct {
    cv::Point2d center;
    double radius;
    double cPerc;
} RANSACIRCLE;


struct SURFDetector
{
    cv::Ptr<cv::Feature2D> surf;
    SURFDetector(double hessian = 800.0)
    {
        surf = cv::xfeatures2d::SURF::create(hessian);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        surf->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};
struct SIFTDetector
{
    cv::Ptr<cv::Feature2D> sift;
    SIFTDetector(int nFeatures = 0)
    {
        sift = cv::xfeatures2d::SIFT::create(nFeatures);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        sift->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};
struct ORBDetector
{
    cv::Ptr<cv::Feature2D> orb;
    ORBDetector(int nFeatures = 500)
    {
        orb = cv::ORB::create(nFeatures);
    }
    template<class T>
    void operator()(const T& in, const T& mask, std::vector<cv::KeyPoint>& pts, T& descriptors, bool useProvided = false)
    {
        orb->detectAndCompute(in, mask, pts, descriptors, useProvided);
    }
};

using namespace std;
using namespace cv;
using namespace Qroilib;

class CImgProcEngine : public CImgProcBase
{
public:
    CImgProcEngine();
    ~CImgProcEngine();


public:
    int GetAlignPtWithMask(Qroilib::RoiObject* pData, cv::Mat graySearchImg);
    int TowPointAlignImage(cv::Mat gray);
    int MeasureAlignImage(cv::Mat src);
    int InspectOneItem(cv::Mat img, Qroilib::RoiObject *pData);

    int OneMatchShapes(vector<vector<Point> >& contours, vector<vector<Point> >& templateseq, Qroilib::RoiObject *pData, int seq);
    typedef struct {
        cv::Point2f first;
        cv::Point2f center;
        cv::Point2f second;

        cv::Point2f p1;
        cv::Point2f p2;
        int len;
    } ElemLineIt;
    void MakeOneElemLine(Point cen, double dAngle, ElemLineIt &elem);
    int AppendOneLine(cv::Mat& mat, vector<ElemLineIt> &vecLineIt, ElemLineIt e, int interval, double dAngle);
    int OneLineMeasurement(cv::Mat& mat, vector<Point>& cone, Qroilib::RoiObject *pData, vector<ElemLineIt> &vecLineIt);

    int SingleROICenterOfPlusMark(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rectIn);
    double SingleROISubPixEdgeWithThreshold(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattIdentify(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattMatchShapes(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SinglePattFeatureMatch(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROISubPixEdge(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICorner(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIOCR(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIBarCode(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROILineMeasurement(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIColorMatching(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROILabelDetect(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROIAreaSegment(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect);
    int SingleROICircleHole(cv::Mat croppedImage, Qroilib::RoiObject* pData, QRectF rect);

    int Find_RANSAC_Circle(Mat grayImg, Qroilib::RoiObject *pData, vector<RANSACIRCLE> &vecRansicCircle);
    float verifyCircle(cv::Mat dt, cv::Point2f center, float radius, std::vector<cv::Point2f> & inlierSet);
    inline void getCircle(cv::Point2f& p1, cv::Point2f& p2, cv::Point2f& p3, cv::Point2f& center, float& radius);
    std::vector<cv::Point2f> getPointPositions(cv::Mat binaryImage);

    double HistEMD(cv::Mat& hist, cv::Mat& target, int dims);

    cv::Mat drawGoodMatches(
        const cv::Mat& img1,
        const cv::Mat& img2,
        const std::vector<cv::KeyPoint>& keypoints1,
        const std::vector<cv::KeyPoint>& keypoints2,
        std::vector<cv::DMatch>& matches,
        std::vector<cv::Point2f>& scene_corners_
    );

    int EdgeCorner(Qroilib::RoiObject *pData, cv::Mat graySearchImgIn, int CornerType, cv::Point2d &outCorner);
    int EdgeCornerByLine(Qroilib::RoiObject *pData, cv::Mat graySearchImgIn, int CornerType, cv::Point2d &corner);

    void SaveOutImage(cv::Mat imgOut, Qroilib::RoiObject *pData, QString strMsg);

    void Smooth(Qroilib::RoiObject *pData, cv::Mat ImgIn, int iImgID);
    double TemplateMatch(Qroilib::RoiObject *pData, cv::Mat SearchImg, cv::Mat TemplateImg, cv::Point &left_top, double &dMatchShapes);

    int Threshold(Qroilib::RoiObject *pData, cv::Mat grayImg, int nDbg = 100);
    int ThresholdRange(Qroilib::RoiObject *pData, cv::Mat grayImg, int nDbg = 100);
    double ThresholdOTSU(Qroilib::RoiObject *pData, cv::Mat grayImg, int nDbg = 100);

    int AdaptiveThreshold(Qroilib::RoiObject *pData, cv::Mat grayImg, int nDbg);
    int NoiseOut(Qroilib::RoiObject *pData, cv::Mat grayImg, int t = -1, int nDbg = 100);
    int Expansion(Qroilib::RoiObject *pData, cv::Mat grayImg, int t = -1, int nDbg = 150);

    void DrawResultCrossMark(cv::Mat img, Qroilib::RoiObject *pData);

private:
    cv::Mat curImg;
    bool m_bSaveEngineImg;

public:
    QString m_sDebugPath;
    Qroilib::DetectResult m_DetectResult;
    QVector<cv::Point2d>	alignpt;
    QVector<cv::Point2d>	insppt;

public:
    int init_tess_failed = 0;
    tesseract::TessBaseAPI *tessApi;
};

