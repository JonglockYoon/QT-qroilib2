// imgprocengine.cpp
// Copyright 2018 jerry1455@gmail.com
//
// Qroilib를 이용할때 이용가능한 opencv함수들을 모아놓은 module이다.
// application개발시 필요한 함수들만 다시 구성해서 새로운 engine module을 만드는것이
// 유지보수에 이롭다.
//
#include <tesseract/baseapi.h>  //  Includes Tesseract and Leptonica libraries
#include <leptonica/allheaders.h>

#include <stdio.h>
#include <QDir>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <algorithm>

#include "imgprocengine.h"
#include "objectgroup.h"
#include "config.h"
#include "recipedata.h"
#include "mainwindow.h"
#include "mattoqimage.h"
#include "opencv2/ximgproc.hpp"
#include "opencv2/objdetect.hpp"

#include "QZXing.h"
#include <zxing/NotFoundException.h>
#include <zxing/ReaderException.h>

using namespace zxing;
using namespace tesseract;
//using namespace cv;

CImgProcEngine::CImgProcEngine()
{
    qDebug() << "CImgProcEngine";

    memset(&m_DetectResult, 0, sizeof(m_DetectResult));
    //m_sDebugPath = ".";
    m_bSaveEngineImg = true;

    QString str;

    //CConfig *pCfg = &gCfg;

    qDebug() << gCfg.RootPath;
    if (gCfg.m_sSaveImageDir.isEmpty())
        str = QString("%1/Engine").arg(gCfg.RootPath);
    else
        str = QString("%1/Engine").arg(gCfg.m_sSaveImageDir);
    QDir dir;
    dir.mkdir(str);
    m_sDebugPath = str;
    curImg = cv::Mat();

}

CImgProcEngine::~CImgProcEngine(void)
{
}

int CImgProcEngine::InspectOneItem(cv::Mat img, RoiObject *pData)
{
    if (pData->mParent != nullptr) {
        if (pData->mParent->mInspectType < _INSPACT_ROI_START)
            return -1;
    }

    QString str;
    str.sprintf("InspectOneItem type=%d", pData->mInspectType);
    theMainWindow->DevLogSave(str.toLatin1().data());
	m_DetectResult.dRadius = 0;
    //m_DetectResult.img = nullptr;

    if (pData == nullptr)
		return -1;

    curImg = img;

    if (m_bSaveEngineImg)
    {
        str.sprintf("0_%d_srcImage.jpg", 100);
        SaveOutImage(img, pData, str);
    }

	int size = pData->m_vecDetectResult.size();
	for (int i = 0; i < size; i++) {
		DetectResult *prst = &pData->m_vecDetectResult[i];
        //if (prst->img)
        //    cvReleaseImage(&prst->img);
	}
	pData->m_vecDetectResult.clear();

    cv::Size isize = cv::Size(img.cols, img.rows);
    cv::Mat colorImg = cv::Mat(isize, CV_8UC3);

    colorImg = cv::Mat(isize, CV_8UC3);
    if (img.channels() == 1)
        cv::cvtColor(img, colorImg, cv::COLOR_GRAY2BGR);
    else if (img.channels() == 3)
        img.copyTo(colorImg);
    else if (img.channels() == 4)
        cv::cvtColor(img, colorImg, cv::COLOR_BGRA2BGR);

//    if (m_bSaveEngineImg)
//	{
//        str.sprintf(("0_%d_Image.jpg"), 110);
//        SaveOutImage(searchImg, pData, str, false);
//	}

    QRectF rect = pData->bounds();	// Area로 등록된 ROI
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= colorImg.cols) rect.setRight(colorImg.cols);
    if (rect.bottom() >= colorImg.rows) rect.setBottom(colorImg.rows);
    pData->setBounds(rect);
    if (rect.width() < 0 || rect.height() < 0) {
        return -1;
    }

    Point2f left_top = Point2f(rect.left(), rect.top());
    cv::Rect roi = cv::Rect((int)left_top.x, (int)left_top.y, rect.width(), rect.height());
    cv::Mat croppedImage;
    croppedImage = cv::Mat(cv::Size(rect.width(), rect.height()), colorImg.type());
    colorImg(roi).copyTo(croppedImage);

    //int nDirection;
	switch (pData->mInspectType)
	{
    case _Inspect_Patt_Identify:
        //strLog.Format(("[%s] InspectType : _Inspect_Patt_Identify"), pData->m_sName);
        //MSystem.DevLogSave(("%s"), strLog);
        SinglePattIdentify(croppedImage, pData, rect);
        break;
    case _Inspect_Patt_MatchShapes:
        //strLog.sprintf(("[%s] InspectType : _Inspect_Patt_MatchShapes"), pData->name().toStdString().c_str());
        //theMainWindow->DevLogSave(strLog.toLatin1().data());
        SinglePattMatchShapes(croppedImage, pData, rect);
        break;
    case _Inspect_Patt_FeatureMatch:
        SinglePattFeatureMatch(croppedImage, pData, rect);
        break;

    case _Inspect_Roi_CenterOfPlusMark:
        SingleROICenterOfPlusMark(croppedImage, pData, rect);
        break;
	case _Inspect_Roi_SubpixelEdgeWithThreshold:
        //strLog.sprintf(("[%s] InspectType : _Inspect_Roi_SubpixelEdgeWithThreshold"), pData->name().toStdString().c_str());
        SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);
		break;
    case _Inspect_Roi_Corner:
        SingleROICorner(croppedImage, pData, rect);
        break;
    case _Inspect_Teseract:
        SingleROIOCR(croppedImage, pData, rect);
        break;
    case _Inspect_BarCode:
        SingleROIBarCode(croppedImage, pData, rect);
        break;
    case _Inspect_Line_Measurement:
        SingleROILineMeasurement(croppedImage, pData, rect);
        break;
    case _Inspect_Color_Matching:
        SingleROIColorMatching(croppedImage, pData, rect);
        break;
    case _Inspect_Label_Detect:
        SingleROILabelDetect(croppedImage, pData, rect);
        break;
    }

	return 0;
}



int CImgProcEngine::GetAlignPtWithMask(RoiObject* pData, cv::Mat graySearchImg)
{
    QString str;

    // mask image
    char strTemp[256];
    sprintf(strTemp, ("%s\\TeachingData\\%s\\%s.jpg"), gCfg.RootPath.toLatin1().data(), gCfg.m_sLastRecipeName.toLatin1().data(), ("maskimage"));
    cv::Mat maskImg = imread(strTemp, IMREAD_GRAYSCALE);

    DocumentView* v = theMainWindow->currentView();
    // mask image
    if (!maskImg.empty())
    {
        for (const Layer *layer : v->mRoi->objectGroups()) {
            const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
            for (const Qroilib::RoiObject *roiObject : objectGroup) {
                Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;

                    cv::Mat croppedImage;
                    QRectF rect = pData->bounds();	// Area로 등록된 ROI
                    QRectF rect1 = rect;
                    rect.normalized();

                    if (rect.left() < 0)	rect.setLeft(0);
                    if (rect.top() < 0)	rect.setTop(0);
                    if (rect.right() >= maskImg.cols)
                        rect.setRight(maskImg.cols);
                    if (rect.bottom() >= maskImg.rows)
                        rect.setBottom(maskImg.rows);
                    mObject->setBounds(rect);

                    Point2f left_top = Point2f(rect.left(), rect.top());
                    cv::Rect roi = cv::Rect((int)left_top.x, (int)left_top.y, rect.width(), rect.height());
                    croppedImage = cv::Mat(cv::Size(rect.width(), rect.height()), maskImg.type());
                    maskImg(roi).copyTo(croppedImage);

                    mObject->m_vecDetectResult.clear();
                    SingleROICorner(croppedImage, mObject, rect);
                    mObject->setBounds(rect1);

                    int size = mObject->m_vecDetectResult.size();
                    if (size > 0)
                    {
                        DetectResult *prst = &mObject->m_vecDetectResult[0];
                        prst->pt.x += rect1.left();
                        prst->pt.y += rect1.top();
                        alignpt.push_back(prst->pt);
                    }
                }
        }
    }

    // inspsect image
    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;

                cv::Mat croppedImage;
                QRectF rect = pData->bounds();	// Area로 등록된 ROI
                QRectF rect1 = rect;
                rect.normalized();
                if (rect.left() < 0)	rect.setLeft(0);
                if (rect.top() < 0)	rect.setTop(0);
                if (rect.right() >= maskImg.cols)
                    rect.setRight(maskImg.cols);
                if (rect.bottom() >= maskImg.rows)
                    rect.setBottom(maskImg.rows);
                mObject->setBounds(rect);

                Point2f left_top = Point2f(rect.left(), rect.top());
                cv::Rect roi = cv::Rect((int)left_top.x, (int)left_top.y, rect.width(), rect.height());
                croppedImage = cv::Mat(cv::Size(rect.width(), rect.height()), graySearchImg.type());
                graySearchImg(roi).copyTo(croppedImage);

                mObject->m_vecDetectResult.clear();
                SingleROICorner(croppedImage, mObject, rect);
                mObject->setBounds(rect1);

                int size = mObject->m_vecDetectResult.size();
                if (size > 0)
                {
                    DetectResult *prst = &mObject->m_vecDetectResult[0];
                    prst->pt.x += rect1.left();
                    prst->pt.y += rect1.top();
                    insppt.push_back(prst->pt);
                }

        }
    }

    return 0;
}

int CImgProcEngine::TowPointAlignImage(cv::Mat gray)
{
    QString str;

    vector<cv::Point2d>	alignpt;
    vector<cv::Point2d>	insppt;

    alignpt.clear();
    insppt.clear();

    DocumentView* v = theMainWindow->currentView();
    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;
            if (mObject->mInspectType == _Inspect_Roi_Corner) {
                GetAlignPtWithMask(mObject, gray);
                break;
            }
        }
    }

    //두점 사이의 각도 구하기.
#define CalculDegree(from, to)  -((double)atan2(to.y - from.y, to.x - from.x) * 180.0f / PI)
    //#ifndef PI
    //#define PI 3.141592653589793f
    //#endif
#define RADIAN(angle) angle *  PI /180


    std::stable_sort(alignpt.begin(), alignpt.end(), [](const cv::Point2d lhs, const cv::Point2d rhs)->bool {
        if (lhs.y < rhs.y) // assending
            return true;
        return false;
    });
    std::stable_sort(insppt.begin(), insppt.end(), [](const cv::Point2d lhs, const cv::Point2d rhs)->bool {
        if (lhs.y < rhs.y) // assending
            return true;
        return false;
    });

    // alignpt & insppt....
    const double dResX = gCfg.m_pCamInfo[0].dResX;
    const double dResY = gCfg.m_pCamInfo[0].dResY;

    for (int i = 0; i<alignpt.size(); i++) {
        //TRACE(_T("align %.3f %.3f\n"), alignpt[i].x, alignpt[i].y);

        alignpt[i].x = alignpt[i].x * dResX;
        alignpt[i].y = alignpt[i].y * dResY;
    }
    for (int i = 0; i<insppt.size(); i++) {
        //TRACE(_T("insp %.3f %.3f\n"), insppt[i].x, insppt[i].y);

        insppt[i].x = insppt[i].x * dResX;
        insppt[i].y = insppt[i].y * dResY;
    }

    if (alignpt.size() == 2 && insppt.size() == 2)
    {

        double dTheta1 = CalculDegree(alignpt[1], alignpt[0]); // 티칭각도 -> mask
        double dTheta2 = CalculDegree(insppt[1], insppt[0]); // 안착 각도 -> insp
        double dTheta = (dTheta1 - dTheta2);
        //TRACE(_T("Compensation theta value %.3f\n"), dTheta);

        // center of image
        cv::Point2d dPointCenter;
        dPointCenter.x = gray.cols / 2.0  * dResX;
        dPointCenter.y = gray.rows / 2.0 * dResY;


        //align(mask) image rotate.
        char strTemp[256];
        sprintf(strTemp, ("%s\\TeachingData\\%s\\%s.jpg"), gCfg.RootPath.toLatin1().data(), gCfg.m_sLastRecipeName.toLatin1().data(), ("maskimage"));
        cv::Mat maskImg = imread(strTemp, IMREAD_GRAYSCALE);

        //dTheta -= 1.0;
        RotateImage(maskImg, dTheta*-1.0);

        double a = RADIAN(dTheta);
        cv::Point2d tmp;
        cv::Point2d dTranslateP;

        tmp.x = alignpt[0].x;
        tmp.y = alignpt[0].y;
        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.x)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        alignpt[0].x = dTranslateP.x;
        alignpt[0].y = dTranslateP.y;


        tmp.x = alignpt[1].x;
        tmp.y = alignpt[1].y;
        dTranslateP.x = (cos(a) * (tmp.x - dPointCenter.x)) - (sin(a) * (tmp.y - dPointCenter.y)) + dPointCenter.x;
        dTranslateP.y = (sin(a) * (tmp.x - dPointCenter.x)) + (cos(a) * (tmp.y - dPointCenter.y)) + dPointCenter.y;
        alignpt[1].x = dTranslateP.x;
        alignpt[1].y = dTranslateP.y;

        // 회전시킨후 두개지점의 X,Y이동거리를 산출한다.
        double tx1 = (alignpt[0].x - insppt[0].x);
        double ty1 = (alignpt[0].y - insppt[0].y);
        double tx2 = (alignpt[1].x - insppt[1].x);
        double ty2 = (alignpt[1].y - insppt[1].y);

        double dsx = (double)(tx1 + tx2) / 2.0; // 티칭위치를 회전이동 시킨후 안착위치와 X,Y가 움직인 거리
        double dsy = (double)(ty1 + ty2) / 2.0;
        int sx = (int)(dsx / dResX) * -1;
        int sy = (int)(dsy / dResY) * -1;
        sy = sy + 4;
        //TRACE(_T("Compensation xy value %d %d\n"), sx, sy);

#if 1
        Mat mat;

        int tx = 0;
        int ty = 0;
        int cx = maskImg.cols;
        int cy = maskImg.rows;

        if (sx < 0) { // shift left
            cx = cx + sx;
            tx = 0;
            sx = abs(sx);
        }
        else if (sx > 0) { // shift right
            cx = cx - sx;
            tx = sx;
            sx = 0;
        }
        if (sy < 0) { // shift up
            cy = cy + sy;
            ty = 0;
            sy = abs(sy);
        }
        else if (sy > 0) { // shift down
            cy = cy - sy;
            ty = sy;
            sy = 0;
        }
        mat = cv::Mat::zeros(maskImg.size(), maskImg.type());
        maskImg(cv::Rect(sx, sy, cx, cy)).copyTo(mat(cv::Rect(tx, ty, cx, cy)));
#endif
        cv::Mat trans = maskImg.clone();
        cv::inRange(trans, cv::Scalar(250), cv::Scalar(255), trans);

        if (m_bSaveEngineImg)
        {
            str.sprintf(("205_mask.jpg"));
            SaveOutImage(trans, nullptr, str);
        }


        cv::erode(trans, trans, cv::Mat());
        cv::bitwise_and(gray, trans, gray);

        if (m_bSaveEngineImg)
        {
            str.sprintf(("206_grapimg.jpg"));
            SaveOutImage(gray, nullptr, str);
        }
    }

    return 0;
}

int CImgProcEngine::MeasureAlignImage(cv::Mat src)
{
    int nErrorType = 0;

    vector<Qroilib::RoiObject *> vecAlign;
    int nMesAlingNum = 0;

    DocumentView* v = theMainWindow->currentView();
    for (const Layer *layer : v->mRoi->objectGroups()) {
        const ObjectGroup &objectGroup = *static_cast<const ObjectGroup*>(layer);
        for (const Qroilib::RoiObject *roiObject : objectGroup) {
            Qroilib::RoiObject *mObject = (Qroilib::RoiObject *)roiObject;

            if (!mObject->objectGroup()->isVisible() || !mObject->isVisible())
                continue;

            if (mObject->mInspectType == _Inspect_Roi_MeasureAlign){
                vecAlign.push_back(mObject);
                nMesAlingNum++;
                if (nMesAlingNum >= 3)
                    break;
            }
        }
    }
    if (nMesAlingNum == 0)
        return -1;


    if (m_bSaveEngineImg)
    {
        QString str;
        str.sprintf(("%d_MeasureAlignSrc.jpg"), 120);
        SaveOutImage(src, nullptr, str);
    }

    if (nMesAlingNum == 3) //  Measure 얼라인
    {
        struct {
            RoiObject* pData;
            double dOrgDiff; // 기준 X,Y위치
            double dEdge;
        } pos[2][3]; // [v=0,h=1] [3]

        int nVert = 0, nHoriz = 0;
        int nDirection;	//("Left2Right,Right2Left,Top2Bottom,Bottom2Top")
        for (int i = 0; i < 3; i++) {
            RoiObject* pData = vecAlign[i];

            cv::Mat croppedImage;
            QRectF rect = pData->bounds();	// Area로 등록된 ROI
            Point2f left_top = Point2f(rect.left(), rect.top());
            cv::Rect roi = cv::Rect((int)left_top.x, (int)left_top.y, rect.width(), rect.height());
            croppedImage = cv::Mat(cv::Size(rect.width(), rect.height()), CV_8UC1);
            if (src.channels() == 3)
                cv::cvtColor(src(roi), croppedImage, cv::COLOR_BGR2GRAY);
            else
                src(roi).copyTo(croppedImage);
            double v = SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);

            if (pData->getIntParam(("Direction"), nDirection) == 0) {
                if (nDirection <= 1) { // Vertical edge
                    pos[0][nVert].pData = pData;
                    pos[0][nVert].dEdge = v;
                    pos[0][nVert].dOrgDiff = pData->bounds().x() - v;
                    nVert++;
                }
                else {
                    pos[1][nHoriz].pData = pData;
                    pos[1][nHoriz].dEdge = v;
                    pos[1][nHoriz].dOrgDiff = pData->bounds().y() - v;
                    nHoriz++;
                }
            }
        }

        double dRetAngle = 0;
        cv::Point2d center;
        center.x = src.cols / 2;
        center.y = src.rows / 2;
        if (nVert == 2 && nHoriz == 1) {
            // 수직으로 두개의 ROI가 있는데 Y 순서대로 배열하기 위해
            RoiObject* pData = pos[0][0].pData;
            RoiObject* pData1 = pos[0][1].pData;
            if (pData->bounds().top() > pData1->bounds().top()) {
                RoiObject* pTmpData = pos[0][0].pData;
                pos[0][0].pData = pos[0][1].pData;
                pos[0][1].pData = pTmpData;
                double dTmpEdge = pos[0][0].dEdge;
                pos[0][0].dEdge = pos[0][1].dEdge;
                pos[0][1].dEdge = dTmpEdge;
                double dtmpDiff = pos[0][0].dOrgDiff;
                pos[0][0].dOrgDiff = pos[0][1].dOrgDiff;
                pos[0][1].dOrgDiff = dtmpDiff;
            }
            pData = pos[0][0].pData;
            pData1 = pos[0][1].pData;
            RoiObject* pData2 = pos[1][0].pData; // 가로 Measure ROI

            double dDistX = fabs(pData1->bounds().top() - pData->bounds().top());
            double dDistY = (pos[0][0].dEdge - pos[0][1].dEdge);
            dRetAngle = (atan2(dDistY, dDistX) * 180.0 / PI); // degree
            //dRetAngle = -dRetAngle;

            // 가로 Measure ROI의 중심점이 dRetAngle 많큼 회전했을때 이동 위치를 산출해서 Rotate 회전 중심을 바꾸어준다.
            // 회전후 Shift된 위치를 계산해서 이동시키기위함.
            double a = (dRetAngle * PI) / 180; // radian
            double x1 = pData2->bounds().center().x();
            double y1 = pData2->bounds().center().y();
            double x0 = src.cols / 2; // 회전의 중심
            double y0 = src.rows / 2;
            double x2 = cos(a) * (x1 - x0) - (-sin(a) * (y1 - y0)) + x0;
            double y2 = (-sin(a) * (x1 - x0)) + cos(a) * (y1 - y0) + y0;

            center.x -= (pos[0][0].dOrgDiff + (x1 - x2) / 2);
            center.y -= (pos[1][0].dOrgDiff + (y1 - y2) / 2);
        }
        else if (nVert == 1 && nHoriz == 2)
        {
            // 수평으로 두개의 ROI가 있는데 X 순서대로 배열하기 위해
            RoiObject* pData = pos[1][0].pData;
            RoiObject* pData1 = pos[1][1].pData;
            if (pData->bounds().left() > pData1->bounds().left()) {
                RoiObject* pTmpData = pos[1][0].pData;
                pos[1][0].pData = pos[1][1].pData;
                pos[1][1].pData = pTmpData;
                double dTmpEdge = pos[1][0].dEdge;
                pos[1][0].dEdge = pos[1][1].dEdge;
                pos[1][1].dEdge = dTmpEdge;
                double dtmpDiff = pos[1][0].dOrgDiff;
                pos[1][0].dOrgDiff = pos[1][1].dOrgDiff;
                pos[1][1].dOrgDiff = dtmpDiff;
            }
            pData = pos[1][0].pData;
            pData1 = pos[1][1].pData;
            RoiObject* pData2 = pos[0][0].pData; // 세로 Measure ROI

            double dDistX = fabs((pData1->bounds().left() - pData->bounds().left()));
            double dDistY = (pos[1][0].dEdge - pos[1][1].dEdge);
            dRetAngle = (atan2(dDistY, dDistX) * 180.0 / PI); // degree
            dRetAngle = -dRetAngle;

            // 세로 Measure ROI의 중심점이 dRetAngle 많큼 회전했을때 이동 위치를 산출해서 Rotate 회전 중심을 바꾸어준다.
            // 회전후 Shift된 위치를 계산해서 이동시키기위함.
            double a = (dRetAngle * PI) / 180; // radian
            double x1 = pData2->bounds().center().x();
            double y1 = pData2->bounds().center().y();
            double x0 = src.cols / 2; // 회전의 중심
            double y0 = src.rows / 2;
            double x2 = cos(a) * (x1 - x0) - (-sin(a) * (y1 - y0)) + x0;
            double y2 = (-sin(a) * (x1 - x0)) + cos(a) * (y1 - y0) + y0;

            center.x -= (pos[0][0].dOrgDiff + (x1 - x2) / 2);
            center.y -= (pos[1][0].dOrgDiff + (y1 - y2) / 2);
        }
        else {
            nErrorType = 2;
            return nErrorType;
        }
        RotateImage(src, dRetAngle, center); // degree 입력

        if (m_bSaveEngineImg)
        {
            QString str;
            str.sprintf(("%d_MeasureAlignDst.jpg"), 121);
            SaveOutImage(src, nullptr, str);
        }

    }
    else if (nMesAlingNum) // nMesAlingNum == 1 이면 가로 또는 세로위치만 이동한다. (한개만 사용)
    {
        int nDirection;
        RoiObject* pData = vecAlign[0];
        if (pData->getIntParam(("Direction"), nDirection) == 0) {

            if (nDirection == 4)  { //  (nCh == 4) 상부 Align은 원의 무게 줌심을 가지고 처리한다.
                //AlignImageCH4(pData, src);
                //return 0;
            }

            cv::Mat croppedImage;
            QRectF rect = pData->bounds();	// Area로 등록된 ROI
            Point2f left_top = Point2f(rect.left(), rect.top());
            cv::Rect roi = cv::Rect((int)left_top.x, (int)left_top.y, rect.width(), rect.height());
            croppedImage = cv::Mat(cv::Size(rect.width(), rect.height()), CV_8UC1);
            if (src.channels() == 3)
                cv::cvtColor(src(roi), croppedImage, cv::COLOR_BGR2GRAY);
            else
                src(roi).copyTo(croppedImage);
            double dEdge = SingleROISubPixEdgeWithThreshold(croppedImage, pData, rect);

            if (nDirection <= 1) { // Vertical edge
                double v = pData->bounds().x() - dEdge; // 좌우 shift만 적용

                int sx = (int)round(v);
                if (sx < 0) { // shift left
                    sx = abs(sx);
                    cv::Mat m = shiftFrame(src, abs(sx), ShiftLeft);
                    m.copyTo(src);
                }
                else if (sx > 0) { // shift right
                    sx = abs(sx);
                    cv::Mat m = shiftFrame(src, abs(sx), ShiftRight);
                    m.copyTo(src);
                }
            }

            if (m_bSaveEngineImg)
            {
                QString str;
                str.sprintf(("%d_MeasureAlignDst.jpg"), 122);
                SaveOutImage(src, nullptr, str);
            }
        }
    }

    return nErrorType;
}


//
//
int CImgProcEngine::SingleROICenterOfPlusMark(cv::Mat croppedImage, RoiObject *pData, QRectF rectIn)
{
	QString str;
    cv::Mat drawImage = cv::Mat();
    cv::Size sz = cv::Size(croppedImage.cols, croppedImage.rows);
    cv::Mat grayImage = cv::Mat(sz, CV_8UC1);
    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, grayImage, cv::COLOR_BGR2GRAY);
    else
        croppedImage.copyTo(grayImage);

    int MinMomentSize = 6, MaxMomentSize = 600;
	int nMinCircleRadius = 300;
	int nMaxCircleRadius = 1000;

	m_DetectResult.pt.x = 0;
	m_DetectResult.pt.y = 0;

    if (pData != nullptr)
	{
		CParam *pParam = pData->getParam(("Minimum circle radius"));
        if (pParam)	nMinCircleRadius = (int)pParam->Value.toDouble();
		pParam = pData->getParam(("Maximum circle radius"));
        if (pParam)	nMaxCircleRadius = (int)pParam->Value.toDouble();
		int nThresholdValue = 0;
		pParam = pData->getParam(("High Threshold"));
		if (pParam)
            nThresholdValue = pParam->Value.toDouble();

		if (nThresholdValue == 0) {
            ThresholdOTSU(pData, grayImage, 130);
		}
		else
            ThresholdRange(pData, grayImage, 130);

        NoiseOut(pData, grayImage, _ProcessValue1, 131);
	}
	else {
		int filterSize = 3;
        cv::inRange(grayImage, cv::Scalar(200), cv::Scalar(255), grayImage);

        Mat element = getStructuringElement(MORPH_RECT, Size(filterSize,filterSize), Point(-1,-1));
        morphologyEx(grayImage, grayImage, MORPH_OPEN, element, Point(-1,-1), 2);
        morphologyEx(grayImage, grayImage, MORPH_CLOSE, element, Point(-1,-1), 2);

        if (m_bSaveEngineImg)
		{
			str.sprintf(("%d_Preprocess.jpg"), 132);
            SaveOutImage(grayImage, nullptr, str);
		}
	}

    if (m_bSaveEngineImg) {
        drawImage = cv::Mat::zeros(grayImage.size(), grayImage.type());
    }
    FilterLargeBlob(grayImage, nMaxCircleRadius*2);
    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("0_%d_ExcludeLargeBlob.jpg"), 133);
        SaveOutImage(grayImage, pData, str);
    }

    FilterLargeArea(grayImage);
    if (m_bSaveEngineImg)
	{
        str.sprintf(("134_cvApproxInImage.jpg"));
        SaveOutImage(grayImage, pData, str);
	}

    Expansion(pData, grayImage, _ProcessValue1, 134);



	// 외곽선 추적 및 근사화 변수 초기화
    vector<vector<Point> > contours;
    findContours(grayImage, contours,  cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	// (2) 외곽선 근사화
	////////////////////////////////////////////////////////////////////////////////////////////////////
	int testcount = -1;
	int iContoursSize = 0;

    int size = contours.size();
    vector<vector<Point> > approx(size);
    for (int k=0; k<size; k++)
    {
        testcount = 0;
        approxPolyDP(Mat(contours[k]), approx[k], arcLength(Mat(contours[k]), true)*0.01, true);
        if (approx[k].size() == 0)
                continue;

        if (m_bSaveEngineImg)
		{

            cv::drawContours(drawImage, approx, k, CVX_WHITE, 1, 8);
            str.sprintf(("135_cvApproxPoly.jpg"));
            SaveOutImage(drawImage, pData, str);
		}

        float radius;
        cv::Point2f center;
        cv::minEnclosingCircle(approx[k], center, radius);

        // 반지름이 설정치보다 적거나 크면 제외
        QString str;
        str.sprintf("%d %d %f", nMinCircleRadius, nMaxCircleRadius, radius);
        theMainWindow->DevLogSave(str.toLatin1().data());
        if (nMinCircleRadius > radius || nMaxCircleRadius < radius)
            continue;

        int sz1 = approx[k].size();
        if (sz1 >= MinMomentSize && sz1 < MaxMomentSize)            // 외곽선을 이루는 점의 갯수가 이것보다 미만이면 잡음이라고 판단
        {
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            // (4) 중심점 계산
            ////////////////////////////////////////////////////////////////////////////////////////////////////
            Point2f centerPoint = CenterOfMoment(approx[k]);

            if (m_bSaveEngineImg)
            {
                str.sprintf(("138_moments.jpg"));
                cv::circle(drawImage, cv::Point(cvRound((double)centerPoint.x), cvRound((double)centerPoint.y)), 3, CVX_WHITE, cv::FILLED);
                SaveOutImage(drawImage, pData, str);
            }

            testcount++;

            //Calcalate to radius from circumference
            m_DetectResult.dRadius = radius;//  m_IIEngine.CalculRadiusFromCircumference(dCircumference);

            //Move Point
            m_DetectResult.pt.x = centerPoint.x;
            m_DetectResult.pt.y = centerPoint.y;

            iContoursSize++;
        }
	}

	if (m_DetectResult.pt.x == 0 || m_DetectResult.pt.y == 0) return -1;
	if (iContoursSize <= 0) return -1;

    m_DetectResult.tl = cv::Point2d(rectIn.topLeft().x(), rectIn.topLeft().y());
    m_DetectResult.br = cv::Point2d(rectIn.bottomRight().x(), rectIn.bottomRight().y());
    pData->m_vecDetectResult.push_back(m_DetectResult);


	return iContoursSize;
}

//
// 에지의 Gray변화가 많은 부분의 위치를 추출
// Threshold를 이용하여 Edge의 경계면을 추출하고 경계면으로부터 Subpixel edge를 추출한다.
// * Hessian Matrix 를 이용한것보다 좀더 경사진면에서의 edge를 구할수 있다.
//
double CImgProcEngine::SingleROISubPixEdgeWithThreshold(cv::Mat croppedImage, RoiObject *pData, QRectF rect)
{
	QString str;
    cv::Size sz1 = cv::Size(croppedImage.cols, croppedImage.rows);
    cv::Mat grayImg = cv::Mat(sz1, CV_8UC1);
    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, grayImg, cv::COLOR_BGR2GRAY);
    else
        croppedImage.copyTo(grayImg);

	vector<cv::Point2f> vecEdges;

	int nThresholdValue = 0;
	CParam *pParam = pData->getParam(("High Threshold"));
	if (pParam)
        nThresholdValue = pParam->Value.toDouble();

	if (nThresholdValue == 0) {
		ThresholdOTSU(pData, grayImg, 211);
	}
	else
		ThresholdRange(pData, grayImg, 211);

    NoiseOut(pData, grayImg, _ProcessValue1, 212);
    Expansion(pData, grayImg, _ProcessValue1, 213);

	//cvNot(grayImg, grayImg);


    int imgStep = grayImg.step;
    uchar* imageData = (uchar*)grayImg.data;
	uint sum = 0;
    for (int col = 0; col < grayImg.cols; col++) {
        for (int row = 0; row < grayImg.rows; row++) {
			sum += imageData[row * imgStep + col];
		}
	}
	if (sum == 0) // 이미지가 모두 0이면 fail 처리
		return -1;

	int nDir = 0;
	pParam = pData->getParam(("Direction")); // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
	if (pParam)
        nDir = (int)pParam->Value.toDouble();

	int nColor;
	int nPolarity = 0;
	int nDetectMethod = 0;
	pParam = pData->getParam(("Polarity")); // ("White2Black, Black2White")
	if (pParam)
        nPolarity = (int)pParam->Value.toDouble();
	if (nPolarity == 0)
		nColor = 255;
	else
		nColor = 0;
	pParam = pData->getParam(("Detect method")); // ("Average,First")
	if (pParam)
        nDetectMethod = (int)pParam->Value.toDouble();

    uchar* data = (uchar*)grayImg.data;
    int widthStep = grayImg.step;
    int cx = grayImg.cols;
    int cy = grayImg.rows;

	// Threshold한 결과 이미지의 경계면을 구한다. Left2Right,Right2Left,Top2Bottom,Bottom2Top에 따라서 
	bool bChange;
	switch (nDir)
	{
	case 0: //Left2Right
		for (int y = 0; y < cy; y++)
		{
			bChange = false;
			for (int x = 0; x < cx; x++)
			{
				int index = x + y*widthStep;
				if (data[index] == nColor)
					bChange = true;
				if (bChange && data[index] != nColor) {
					vecEdges.push_back(cv::Point2f(x, y));
					break;
				}
			}
		}
		break;
	case 1: //Right2Left
		for (int y = 0; y < cy; y++)
		{
			bChange = false;
			for (int x = cx - 1; x >= 0; x--)
			{
				int index = x + y*widthStep;
				if (data[index] == nColor)
					bChange = true;
				if (bChange && data[index] != nColor) {
					vecEdges.push_back(cv::Point2f(x, y));
					break;
				}
			}
		}
		break;
	case 2: //Top2Bottom
		for (int x = 0; x < cx; x++)
		{
			bChange = false;
			for (int y = 0; y < cy; y++)
			{
				int index = x + y*widthStep;
				if (data[index] == nColor)
					bChange = true;
				if (bChange && data[index] != nColor) {
					vecEdges.push_back(cv::Point2f(x, y));
					break;
				}
			}
		}
		break;
	case 3: //Bottom2Top
		for (int x = 0; x < cx; x++)
		{
			bChange = false;
			for (int y = cy - 1; y >= 0; y--)
			{
				int index = x + y*widthStep;
				if (data[index] == nColor)
					bChange = true;
				if (bChange && data[index] != nColor) {
					vecEdges.push_back(cv::Point2f(x, y));
					break;
				}
			}
		}
		break;
	}

	switch (nDir) {
	case 0: // 세로선
	case 1:
		std::stable_sort(vecEdges.begin(), vecEdges.end(), [](const cv::Point2f lhs, const cv::Point2f rhs)->bool {
			if (lhs.x < rhs.x) // assending
				return true;
			return false;
		});
		break;
	case 2: // 가로선
	case 3:
		std::stable_sort(vecEdges.begin(), vecEdges.end(), [](const cv::Point2f lhs, const cv::Point2f rhs)->bool {
			if (lhs.y < rhs.y) // assending
				return true;
			return false;
		});
		break;
	}

	double dVal = 0;
	float sz = vecEdges.size();
	int first = sz * 0.4;
	int last = sz - (sz * 0.4);
	if (first < 0)
		first = 0;
	if (last < 0)
		last = 0;

	if (nDetectMethod == 0)
	{
		// 소팅을 한 결과 테이블에서 상하 40%를 버리고 중간 20%의 중간값을 구한다.
		for (int i = first; i < last; i++)
		{
			if (nDir == 0 || nDir == 1)
				dVal += vecEdges[i].x;
			else
				dVal += vecEdges[i].y;
		}

		sz = last - first;
		if (sz > 0) {
			dVal /= sz;
		}
	}
	else {
		//제일 처음 만나는 edge를 구한다
		if (sz > 0) {
			if (nDir == 0)  // Left2Right 세로선
				dVal += vecEdges[0].x;
			else if (nDir == 1)  // Right2Left 세로선
				dVal += vecEdges[sz - 1].x;
			else if (nDir == 2)  // Top2Bottom 가로선
				dVal += vecEdges[0].y;
			else				//Bottom2Top
				dVal += vecEdges[sz - 1].y;
		}
	}
	//
    // 여기까지 Threshold를 이용한 edge를 구하였다.

	//
	// dVal이 Threshold를 이용한 edge값인데, 이값을 기준으로 Ramp edge를 구한다.
	// 
	int width = 6;
	pParam = pData->getParam(("Ramp width")); // default : 6
	if (pParam)
        width = (int)pParam->Value.toDouble();



	int start = 0;
	int end = 0;
    cv::Mat edgeImage = cv::Mat();
	switch (nDir) // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
	{
	case 0: //Left2Right,Right2Left
	case 1:
    {
		start = dVal - width;
		end = dVal + width;
		if (start < 0) start = 0;
		if (end > cx) end = cx;

        cv::Rect roi = cv::Rect((int)start, (int)0, end - start, croppedImage.rows);
        edgeImage = cv::Mat(cv::Size(end - start, croppedImage.rows), croppedImage.type());
        croppedImage(roi).copyTo(edgeImage);

        if (m_bSaveEngineImg){
            SaveOutImage(edgeImage, pData, ("251_RampImg.jpg"));
		}
    }
		break;
	case 2: //Top2Bottom,Bottom2Top
	case 3:
    {
		start = dVal - width;
		end = dVal + width;
		if (start < 0) start = 0;
		if (end > cy) end = cy;

        cv::Rect roi = cv::Rect((int)0, (int)start, croppedImage.cols, end - start);
        edgeImage = cv::Mat(cv::Size(croppedImage.cols, end - start), croppedImage.type());
        croppedImage(roi).copyTo(edgeImage);

        if (m_bSaveEngineImg){
            SaveOutImage(edgeImage, pData, ("252_RampImg.jpg"));
		}
    }
		break;
	}

    double dEdge = SubPixelRampEdgeImage(edgeImage, nDir);
    if (m_bSaveEngineImg){
        SaveOutImage(edgeImage, pData, ("260_SubPixelRampEdgeImageIn.jpg"));
    }

	//DetectResult result;
	switch (nDir)
	{
	case 0: // Left2Right 세로선
	case 1: // Right2Left
        dEdge += rect.left();
		dEdge += start;
		m_DetectResult.pt.x = dEdge;
        m_DetectResult.pt.y = rect.top() + rect.height() / 2;
		break;
	case 2: // 가로선
	case 3:
        dEdge += rect.top();
		dEdge += start;
		m_DetectResult.pt.y = dEdge;
        m_DetectResult.pt.x = rect.left() + rect.width() / 2;
		break;
	}

    //m_DetectResult.nCh = nCh;
	pData->m_vecDetectResult.push_back(m_DetectResult);


    str.sprintf(("%s Edge Result : %.3f"), pData->name().toStdString().c_str(), dEdge);
    theMainWindow->DevLogSave(str.toLatin1().data());

	return dEdge;
}

int CImgProcEngine::SinglePattIdentify(cv::Mat grayImage, RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    if (pData == nullptr || pData->templateMat.empty())
        return -1;
    QString str;

    cv::Size searchSize = cv::Size(grayImage.cols, grayImage.rows);
    cv::Mat graySearchImg = cv::Mat(searchSize, CV_8UC1);
    if (grayImage.channels() == 3)
        cv::cvtColor(grayImage, graySearchImg, cv::COLOR_BGR2GRAY);
    else
        grayImage.copyTo(graySearchImg);

    //static cv::Point2d cog = { 0, 0 };

    cv::Size templateSize = cv::Size(pData->templateMat.cols, pData->templateMat.rows);
    cv::Mat grayTemplateImg = cv::Mat(templateSize, CV_8UC1);
    if (pData->templateMat.channels() == 3)
        cv::cvtColor(pData->templateMat, grayTemplateImg, cv::COLOR_BGR2GRAY);
    else
        pData->templateMat.copyTo(grayTemplateImg);


    double dMatchShapes = 0;
    double MatchRate = 0, LimitMatchRate = 40;

    CParam *pParam = pData->getParam(("Pattern matching rate"));
    if (pParam) LimitMatchRate = pParam->Value.toDouble();

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_TemplateImage0.jpg"), 140);
        SaveOutImage(pData->templateMat, pData, str);
    }

    double dAngle = 0.0;
    double dAngleStep = 0.0;
    cv::Point left_top = { 0, 0 };
    pParam = pData->getParam(("Rotate angle"));
    if (pParam) dAngle = pParam->Value.toDouble();
    pParam = pData->getParam(("Angle step"));
    if (pParam) dAngleStep = pParam->Value.toDouble();

    if (dAngle > 0.0) // computing power가 낮은 시스템은 사용하지말자.
    {
        cv::Size size = cv::Size(pData->bounds().width() - grayTemplateImg.cols + 1, pData->bounds().height() - grayTemplateImg.rows + 1);
        cv::Mat C = cv::Mat(size, CV_32FC1); // 상관계수를 구할 이미지(C)
        double min, max;
        double rate = LimitMatchRate / 100.0;
        std::vector<std::pair<double, double> > pairs;
        cv::Mat cloneImg = grayTemplateImg.clone();
        int cnt = 0;
        for (double a = -dAngle; a < dAngle; a=a+dAngleStep) // 패턴을 -30 에서 30도까지 돌려가면서 매칭율이 가장좋은 이미지를 찾는다.
        {
            grayTemplateImg.copyTo(cloneImg);
            RotateImage(cloneImg, a);

            if (m_bSaveEngineImg)
            {
                cnt++;
                QString str; str.sprintf(("%d_Template%d.jpg"), 149, cnt);
                SaveOutImage(cloneImg, pData, str);
            }

            cv::matchTemplate(graySearchImg, cloneImg, C, cv::TM_CCOEFF_NORMED); // 제곱차 매칭
            cv::minMaxLoc(C, &min, &max, nullptr, &left_top); // 상관계수가 최대값을 값는 위치 찾기
            if (max > rate) {
                pairs.push_back(std::pair<double, double>(a, max));
            }
        }
        C.release();

        std::stable_sort(pairs.begin(), pairs.end(), [=](const std::pair<double, double>& a, const std::pair<double, double>& b)
        {
            return a.second > b.second; // descending
        });

        if (pairs.size() > 0) {
            std::pair<double, double> a = pairs[0];
            grayTemplateImg.copyTo(cloneImg);
            RotateImage(cloneImg, a.first);
            cloneImg.copyTo(grayTemplateImg);
            QString str;
            str.sprintf("big match : %f %f", a.first, a.second);
            //qDebug() << "big match : " << a.first << a.second;
            theMainWindow->DevLogSave(str.toLatin1().data());
        }
        cloneImg.release();
    }

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_TemplateImage1.jpg"), 150);
        SaveOutImage(grayTemplateImg, pData, str);
    }

    // Opencv Template Matching을 이용해서 Template 을 찾는다.
    // 이미지 Template Matching이기때문에 부정확한것은 cvMatchShapes()로 보완한다.
    MatchRate = TemplateMatch(pData, graySearchImg, grayTemplateImg, left_top, dMatchShapes);
    if (LimitMatchRate <= MatchRate)
    {
        //strMsg.Format(("TemplateMatch Result Success ===> : %.2f%%"), MatchRate);
        //MSystem.DevLogSave(("%s"), strMsg);

        if (m_DetectResult.result == true)
        {
            m_DetectResult.resultType = RESULTTYPE_RECT;
            m_DetectResult.pt = cv::Point2d(left_top.x + pData->templateMat.cols/2, left_top.y + pData->templateMat.rows/2);
            m_DetectResult.tl = cv::Point2d(left_top.x, left_top.y);
            m_DetectResult.br = cv::Point2d(left_top.x + pData->templateMat.cols, left_top.y + pData->templateMat.rows);
            pData->m_vecDetectResult.push_back(m_DetectResult);
        }
    }
    else {
        left_top = { 0, 0 };
    }

    str.sprintf(("SinglePattIdentify MatchRate:%.1f MatchShapes:%.1f (%s)"),
                MatchRate, dMatchShapes, m_DetectResult.result == true ? ("OK") : ("NG"));
    qDebug() << str;
    //MSystem.m_pFormBottom->SetBottomMessage(str);
    theMainWindow->DevLogSave(str.toLatin1().data());

    return 0;
}

int CImgProcEngine::SinglePattMatchShapes(cv::Mat croppedImage, RoiObject *pData, QRectF rectIn)
{
    Q_UNUSED(rectIn);
    if (pData->templateMat.empty())
        return -1;
    cv::Size sz = cv::Size(croppedImage.cols, croppedImage.rows);
    cv::Mat grayImg = cv::Mat(sz, CV_8UC1);
    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, grayImg, cv::COLOR_BGR2GRAY);
    else
        croppedImage.copyTo(grayImg);

    clock_t start_time1 = clock();

    QString str;
    int retry = 0;
    CParam *pParam;
    int nThresholdHighValue;

    nThresholdHighValue = 255;
    pParam = pData->getParam("High Threshold", _ProcessValue1+retry);
    if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

    pData->m_vecDetectResult.clear();

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Src.jpg"), 200);
        SaveOutImage(grayImg, pData, str);
    }

    if (nThresholdHighValue == 0)
        ThresholdOTSU(pData, grayImg, 211);
    else
        ThresholdRange(pData, grayImg, 211);

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_Threshold.jpg"), 203);
        SaveOutImage(grayImg, pData, str);
    }

    NoiseOut(pData, grayImg, _ProcessValue1, 212);
    Expansion(pData, grayImg, _ProcessValue1, 213);

    ///////////////////////////////////////////////////////////

    cv::Size templateSize = cv::Size(pData->templateMat.cols, pData->templateMat.rows);
    cv::Mat grayTemplateImg = cv::Mat(templateSize, CV_8UC1);
    if (pData->templateMat.channels() == 3)
        cv::cvtColor(pData->templateMat, grayTemplateImg, cv::COLOR_BGR2GRAY);
    else
        pData->templateMat.copyTo(grayTemplateImg);

     if (m_bSaveEngineImg)
    {
        str.sprintf(("%d_grayTemplateImg.jpg"), 250);
        SaveOutImage(grayTemplateImg, pData, str);
    }


    cv::Mat g2 = cv::Mat(cv::Size(grayTemplateImg.cols, grayTemplateImg.rows), CV_8UC1);
    grayTemplateImg.copyTo(g2);

    if (nThresholdHighValue == 0)
        ThresholdOTSU(pData, g2, 256);
    else
        ThresholdRange(pData, g2, 256);
    NoiseOut(pData, g2, _ProcessValue1, 260);
    Expansion(pData, g2, _ProcessValue1, 261);
    FilterLargeArea(g2);

    int nGaussian = 3;
    try {
        cv::GaussianBlur(g2, g2, cv::Size(nGaussian,nGaussian), 0);
    } catch (...) {
        qDebug() << "Error g2 cvSmooth()";
    }

    cv::Canny(g2, g2, 100, 300);
    if (m_bSaveEngineImg){
        SaveOutImage(g2, pData, ("265_TemplateImageCany.jpg"));
    }

    vector<vector<cv::Point> > c2;
    cv::findContours( g2, c2, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE, Point(0, 0) );
    if (c2.size() == 0) {
        return 0;
    }

    cv::Rect boundbox2 = cv::boundingRect( Mat(c2[0]) );
    for (int i = 0; i < c2[0].size(); ++i)
    {
        cv::Point* p = &c2[0][i];
        p->x = p->x - boundbox2.x;
        p->y = p->y - boundbox2.y;
    }

    if (m_bSaveEngineImg)
    {
        cv::Mat m = cv::Mat(g2.cols, g2.rows, CV_8SC1);
        cv::drawContours(m, c2, 0, CVX_WHITE, 1, 8);
        str.sprintf(("266_Template_Contour.jpg"));
        SaveOutImage(m, pData, str);
    }

    //int nGaussian = 3;
    try {
        cv::GaussianBlur(g2, g2, cv::Size(nGaussian,nGaussian), 0);
    } catch (...) {
        qDebug() << "Error grayImg cvSmooth()";
    }

    ///////////////////////////////////////////////////////////

    vector<vector<Point> > contours;
    cv::findContours(grayImg, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for( size_t i = 0; i< contours.size(); i++ )
    {
        // QThead Lambda를 이용하면 병렬 처리가 가능함.
        int iRst = OneMatchShapes(contours, c2, pData, i);
    }

    int size = pData->m_vecDetectResult.size();
    if (size >= 1) {
        std::stable_sort(pData->m_vecDetectResult.begin(), pData->m_vecDetectResult.end(), [](const DetectResult lhs, const DetectResult rhs)->bool {
            if (lhs.dMatchRate > rhs.dMatchRate) // descending
                return true;
            return false;
        });

        clock_t finish_time1 = clock();
        double total_time = (double)(finish_time1-start_time1)/CLOCKS_PER_SEC;
        str.sprintf("Searching Time=%dms", (int)(total_time*1000));
        theMainWindow->DevLogSave(str.toLatin1().data());

        m_DetectResult = pData->m_vecDetectResult[0];
        pData->m_vecDetectResult.clear();
        pData->m_vecDetectResult.push_back(m_DetectResult);
        str.sprintf(("Selected template Shape Match ===> : %.2f%%"), m_DetectResult.dMatchRate);
        theMainWindow->DevLogSave(str.toLatin1().data());
    }

    return 0;
}

int CImgProcEngine::OneMatchShapes(vector<vector<Point> >& contours, vector<vector<Point> >& templateseq, RoiObject *pData, int seq)
{
    int rst = -1;

    float dMatchShapesingRate = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Shape matching rate"));
        if (pParam)
            dMatchShapesingRate = (float)pParam->Value.toDouble() / 100.0f;
    }

    Moments mu = moments( contours[seq], false );
    Point2f mc = Point2f( mu.m10/mu.m00 , mu.m01/mu.m00 );

    QString str;
    cv::Rect sbb = cv::boundingRect( Mat(contours[seq]) );
    if (m_bSaveEngineImg)
    {
        cv::Mat m = cv::Mat(sbb.width, sbb.height, CV_8SC1);
        cv::drawContours(m, contours, seq, CVX_WHITE, 1, 8, noArray(), 0, Point(-sbb.x, -sbb.y));
        str.sprintf(("270_SearchContour_%d.jpg"), seq);
        SaveOutImage(m, pData, str);
    }

    double matching = cv::matchShapes(contours[seq], templateseq[0], cv::CONTOURS_MATCH_I1, 0); // 작은 값 일수록 두 이미지의 형상이 가까운 (0은 같은 모양)라는 것이된다.
    //double matching = cvMatchShapes(contours, templateseq, cv::CONTOURS_MATCH_I1);
    //double matching2 = cvMatchShapes(searchSeq, templateSeq, cv::CONTOURS_MATCH_I2);
    //double matching3 = cvMatchShapes(searchSeq, templateSeq, cv::CONTOURS_MATCH_I3);
    //qDebug() << "matching" << matching << matching2 << matching3;
    double dMatchShapes = (1.0 - matching) * 100.0;
    if (matching >= 0 && matching <= (1.0 - dMatchShapesingRate))
    {
        m_DetectResult.resultType = RESULTTYPE_RECT;
        m_DetectResult.dMatchRate = dMatchShapes;
        m_DetectResult.pt = cv::Point2d(mc.x, mc.y);
        m_DetectResult.tl = cv::Point2d(m_DetectResult.pt.x-sbb.width/2,m_DetectResult.pt.y-sbb.height/2);
        m_DetectResult.br = cv::Point2d(m_DetectResult.pt.x+sbb.width/2,m_DetectResult.pt.y+sbb.height/2);
        pData->m_vecDetectResult.push_back(m_DetectResult);
        rst = 0;
    }

    return rst;
}

int CImgProcEngine::SinglePattFeatureMatch(cv::Mat croppedImage, RoiObject *pData, QRectF rectIn)
{
    Q_UNUSED(rectIn);

    if (m_bSaveEngineImg)
    {
        QString str; str.sprintf(("%d_TemplateImg.jpg"), 200);
        SaveOutImage(pData->templateMat, pData, str);
    }

    clock_t start_time1 = clock();

    CParam *pParam;
    int nMethod = 0;
    pParam = pData->getParam("Method");
    if (pParam)
        nMethod = (int)pParam->Value.toDouble();
    double dParam1 = 0;
    pParam = pData->getParam("Param1");
    if (pParam)
        dParam1 = pParam->Value.toDouble();

    double kDistanceCoef = 0;
    pParam = pData->getParam("kDistanceCoef");
    if (pParam)
        kDistanceCoef = pParam->Value.toDouble();
    double kMaxMatchingSize = 0;
    pParam = pData->getParam("kMaxMatchingSize");
    if (pParam)
        kMaxMatchingSize = pParam->Value.toDouble();


    //std::string detectorName;
    //detectorName = ui->comboBoxDetector->currentText().toLatin1().data(); // "SURF";
    std::vector<cv::KeyPoint> keypoints_object, keypoints_scene;
    cv::Mat descriptors_object, descriptors_scene;
    vector<vector<cv::DMatch>> m_knnMatches;
    std::vector< cv::DMatch > good_matches;

    cv::Mat img_object = pData->templateMat;
    cv::Mat img_scene =  croppedImage;

    if (nMethod == 1) { // SIFT distance:10, MaxSize:200
        SIFTDetector sift(dParam1); //  nParam1=0
        sift(img_object, cv::Mat(), keypoints_object, descriptors_object);
        sift(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher;
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else if (nMethod == 0) { // SURF distance:5, MaxSize:200
        SURFDetector surf(dParam1); //  nParam1=800
        surf(img_object, cv::Mat(), keypoints_object, descriptors_object);
        surf(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher;
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else if (nMethod == 2) { // ORB distance:5, MaxSize:200
        ORBDetector orb(dParam1); // nParam1=2000
        orb(img_object, cv::Mat(), keypoints_object, descriptors_object);
        orb(img_scene, cv::Mat(), keypoints_scene, descriptors_scene);

        cv::BFMatcher matcher(cv::NORM_HAMMING); // use cv::NORM_HAMMING2 for ORB descriptor with WTA_K == 3 or 4 (see ORB constructor)
        matcher.knnMatch(descriptors_object, descriptors_scene, m_knnMatches, 2);
    }
    else return -1;

    for(int i = 0; i < min(img_scene.rows-1,(int) m_knnMatches.size()); i++) //THIS LOOP IS SENSITIVE TO SEGFAULTS
    {
        const cv::DMatch& bestMatch = m_knnMatches[i][0];
        if((int)m_knnMatches[i].size()<=2 && (int)m_knnMatches[i].size()>0)
        {
            good_matches.push_back(bestMatch);
        }
    }

    if (good_matches.size() == 0)
        return -1;

    std::sort(good_matches.begin(), good_matches.end());
    while (good_matches.front().distance * kDistanceCoef < good_matches.back().distance) {
        good_matches.pop_back();
    }
    while (good_matches.size() > kMaxMatchingSize) {
        good_matches.pop_back();
    }

    std::vector<cv::Point2f> corner;
    cv::Mat img_matches = drawGoodMatches(img_object, img_scene,
                                keypoints_object, keypoints_scene, good_matches, corner);

    //-- Show detected matches
    if (img_matches.rows > 0)
    {
        cv::namedWindow("knnMatch", 0);
        cv::imshow("knnMatch", img_matches);
    }

    if (corner.size() > 0)
    {
        qDebug() << "there are " << good_matches.size() << " good matches";

        cv::Point pt1 = corner[0];
        cv::Point pt2 = corner[1];
        cv::Point pt3 = corner[2];
        cv::Point pt4 = corner[3];

        m_DetectResult.resultType = RESULTTYPE_RECT4P;
        m_DetectResult.tl = cv::Point2d(pt1);
        m_DetectResult.tr = cv::Point2d(pt2);
        m_DetectResult.br = cv::Point2d(pt3);
        m_DetectResult.bl = cv::Point2d(pt4);
        pData->m_vecDetectResult.push_back(m_DetectResult);

//        cvLine(outImg, pt1, pt2, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        cvLine(outImg, pt2, pt3, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        cvLine(outImg, pt3, pt4, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        cvLine(outImg, pt4, pt1, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//        m_DetectResult.rect = QRectF(QPointF(0,0),QPointF(0,0));
//        pData->m_vecDetectResult.push_back(m_DetectResult);

//        cv::Rectangle(matImg2, pt1, pt3, cv::Scalar(255, 255, 255), cv::FILLED); // test
//        theMainWindow->outWidget("test1", matImg2);


        clock_t finish_time1 = clock();
        double total_time = (double)(finish_time1-start_time1)/CLOCKS_PER_SEC;

        QString str;
        str.sprintf("Searching Time=%dms", (int)(total_time*1000));
        theMainWindow->DevLogSave(str.toLatin1().data());

        RotatedRect myRotatedRect = minAreaRect( Mat(corner) );
        double myContourAngle = myRotatedRect.angle;
        if (myRotatedRect.size.width < myRotatedRect.size.height) {
          //myContourAngle = myContourAngle - 90;
        }
        qDebug() << "myContourAngle: " << myContourAngle;
        for (int i=0; i<4; i++) {
            qDebug() << corner[i].x << corner[i].y;
        }
    }

    return 0;
}

cv::Mat CImgProcEngine::drawGoodMatches(
    const cv::Mat& img1,
    const cv::Mat& img2,
    const std::vector<cv::KeyPoint>& keypoints1,
    const std::vector<cv::KeyPoint>& keypoints2,
    std::vector<cv::DMatch>& good_matches,
    std::vector<cv::Point2f>& scene_corners_
)
{
    cv::Mat img_matches;
    if (good_matches.size() == 0)
        return img_matches;

    cv::drawMatches(img1, keypoints1, img2, keypoints2,
        good_matches, img_matches, cv::Scalar::all(-1), cv::Scalar::all(-1),
        std::vector<char>(), cv::DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

    //-- Localize the object
    std::vector<cv::Point2f> obj;
    std::vector<cv::Point2f> scene;

    for (size_t i = 0; i < good_matches.size(); i++)
    {
        //-- Get the keypoints from the good matches
        obj.push_back(keypoints1[good_matches[i].queryIdx].pt);
        scene.push_back(keypoints2[good_matches[i].trainIdx].pt);
    }
    //-- Get the corners from the image_1 ( the object to be "detected" )
    std::vector<cv::Point2f> obj_corners(4);
    obj_corners[0] = cv::Point(0, 0);
    obj_corners[1] = cv::Point(img1.cols, 0);
    obj_corners[2] = cv::Point(img1.cols, img1.rows);
    obj_corners[3] = cv::Point(0, img1.rows);
    std::vector<cv::Point2f> scene_corners(4);

    cv::Mat H;
    try {
        H = cv::findHomography(obj, scene, cv::RANSAC);
        perspectiveTransform(obj_corners, scene_corners, H);
    } catch (...) {
        qDebug() << ("Error <unknown> findHomography()");
        return img_matches;
    }

    scene_corners_ = scene_corners;

    //-- Draw lines between the corners (the mapped object in the scene - image_2 )
//    cv::Point pt1 = scene_corners[0] + cv::Point2f((float)img1.cols, 0);
//    cv::Point pt2 = scene_corners[1] + cv::Point2f((float)img1.cols, 0);
//    cv::Point pt3 = scene_corners[2] + cv::Point2f((float)img1.cols, 0);
//    cv::Point pt4 = scene_corners[3] + cv::Point2f((float)img1.cols, 0);
//    line(img_matches, pt1, pt2, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//    line(img_matches, pt2, pt3, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//    line(img_matches, pt3, pt4, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
//    line(img_matches, pt4, pt1, cv::Scalar(128, 128, 128), 2, cv::LINE_AA);

    return img_matches;
}

int CImgProcEngine::SingleROICorner(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    Q_UNUSED(rect);
    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, croppedImage, cv::COLOR_BGR2GRAY);

    int nCornerType = 1;
    int nFindMethod = 1; // 0 - 코너 찾기, 1-가로,세로 Line으로 코너 찾기
    nCornerType = 0;	// 좌상단 코너
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Corner"));
        if (pParam)
            nCornerType = pParam->Value.toDouble();
        pParam = pData->getParam(("Method"));
        if (pParam)
            nFindMethod = pParam->Value.toDouble();
    }


    cv::Point2d outCorner;

    int nRet2 = 0;
    if (nFindMethod == 0)
        nRet2 = EdgeCorner(pData, croppedImage, nCornerType, outCorner);
    else
        nRet2 = EdgeCornerByLine(pData, croppedImage, nCornerType, outCorner);

    if (nRet2 == 0)	// Blob처리를 한 Cornet 찾기
    {
        cv::Point2d pt;

        pt = outCorner;
        //pt.x += rect.left();
        //pt.y += rect.top();

        //DetectResult result;
        m_DetectResult.pt = pt;
        pData->m_vecDetectResult.push_back(m_DetectResult);

        QString str;
        str.sprintf(("SingleROICorner point : %.2f,%.2f"), m_DetectResult.pt.x, m_DetectResult.pt.y);
        theMainWindow->DevLogSave(str.toLatin1().data());
    }
    return 0;
}

//
// CornerType : 0 - Upper Left, 1 - Upper Right, 2 - Bottom Left, 3 - Bottom Right
//
int CImgProcEngine::EdgeCorner(Qroilib::RoiObject *pData, cv::Mat graySearchImgIn, int CornerType, cv::Point2d &outCorner)
{
    outCorner = cv::Point2d(0, 0);
    cv::Mat graySearchImg = graySearchImgIn.clone();

    if (m_bSaveEngineImg){
        SaveOutImage(graySearchImg, pData, ("310_EdgeCornerSrc.jpg"));
    }

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    if (nThresholdValue == 0) {
        ThresholdOTSU(pData, graySearchImg, 311);
    }
    else
        ThresholdRange(pData, graySearchImg, 311);

    NoiseOut(pData, graySearchImg, _ProcessValue1, 312);
    Expansion(pData, graySearchImg, _ProcessValue1, 313);

    CBlobResult blobs;
    blobs = CBlobResult(graySearchImg);
    int n = blobs.GetNumBlobs();

#if 1
    // 가장큰 Blob 2개만 남김
    n = blobs.GetNumBlobs();
    if (n > 1)
    {
        double dLargeArea = 0;
        double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
        if (area.size() >= 2)
            dLargeArea = area[1];
        else if (area.size() >= 1)
            dLargeArea = area[0];
        blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);
    }

    // 정사각형에 가까운 큰 Blob 2개만 남김
    n = blobs.GetNumBlobs();
    if (n > 1)
    {
        int nPos = 0;
        double_stl_vector AxisRatio = blobs.GetSTLResult(CBlobGetAxisRatio());
        std::stable_sort(AxisRatio.begin(), AxisRatio.end(), [](const double lhs, const double rhs)->bool {
            return lhs < rhs;
        });
        if (AxisRatio.size() >= 2)
            nPos = 1;
        blobs.Filter(blobs, B_INCLUDE, CBlobGetAxisRatio(), B_LESS_OR_EQUAL, AxisRatio[nPos]); // 정사각형에 가까운  blob 2개만 남김
    }
#endif

    graySearchImg = cv::Mat::zeros(graySearchImg.size(), graySearchImg.type());
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(graySearchImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    }

    if (m_bSaveEngineImg){
        SaveOutImage(graySearchImg, pData, ("316_imageLargeBlobs.jpg"));
    }

#if 1
    // 코너위치를 보고 가까운 Blob만 남긴다.
    typedef struct _tagBlobInfo {
        cv::Point2f pt;
        double area;
    } BLOBSEL;
    vector<BLOBSEL*> vecBlobs;
    BLOBSEL *pBlobSel;
    graySearchImg = cv::Mat::zeros(graySearchImg.size(), graySearchImg.type());
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(graySearchImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.

        cv::Rect rect = currentBlob->GetBoundingBox();
        pBlobSel = new BLOBSEL;
        pBlobSel->area = currentBlob->Area();
        pBlobSel->pt.x = (rect.x + rect.width / 2) / 4; // Blob의 중앙점
        pBlobSel->pt.y = (rect.y + rect.height / 2) / 4;
        vecBlobs.push_back(pBlobSel);
    }

    int size = vecBlobs.size();
    if (size > 0)
    {
        switch (CornerType)
        {
        case 0: // UpperLeft - 위쪽 Blob, 왼쪽 Blob만 남긴다
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x > rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        case 2: // BottomLeft
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x > rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        case 1: // UpperRight
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x < rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        case 3: // BottomRight - 오른쪽 Blob, 아래쪽 blob만 남긴다.
            std::stable_sort(vecBlobs.begin(), vecBlobs.end(), [](const BLOBSEL* lhs, const BLOBSEL* rhs)->bool {
                if (lhs->pt.x < rhs->pt.x)
                    return true;
                return false;
            });
            pBlobSel = vecBlobs[0];
            blobs.Filter(blobs, B_INCLUDE, CBlobGetArea(), B_EQUAL, pBlobSel->area);
            break;
        }
    }
    size = vecBlobs.size();
    for (int i = 0; i < size; i++)
        delete vecBlobs[i];
#endif

    // Filter된  blob을 이미지로 변환
    graySearchImg = cv::Mat::zeros(graySearchImg.size(), graySearchImg.type());
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(graySearchImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    }

    if (m_bSaveEngineImg){
        SaveOutImage(graySearchImg, pData, ("317_imageLargeBlobs.jpg"));
    }


    // 후처리
    Expansion(pData, graySearchImg, _PostProcessValue1, 319);

    // 2. 윤곽선 표시 및 윤곽선 영상 생성
    vector<vector<Point> > contours;
    cv::findContours(graySearchImg, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    if (m_bSaveEngineImg){
        graySearchImg = cv::Mat::zeros(graySearchImg.size(), graySearchImg.type());
        cv::drawContours(graySearchImg, contours, -1, CV_RGB(255, 255, 255), 1, 8);
        SaveOutImage(graySearchImg, pData, ("323_cvDrawContours.jpg"));
    }

    // 3. 꼭지점 추출

    std::vector<cv::Point2f> corners;
    int ret = -1;

    vector<vector<Point> > approx(contours.size());
    double d = 0;
    for (int i=0; i<contours.size(); i++)
    {
        if (contours[i].size() < 4)
            continue;
        while (1) {
            approxPolyDP(Mat(contours[i]), approx[i], d, true);
            d++;
            if (approx[i].size() > 4)
                continue;
            break;
        }

        if (m_bSaveEngineImg)
        {
            cv::Mat tempImage = cv::Mat(cv::Size(graySearchImg.cols, graySearchImg.rows), graySearchImg.type());
            cv::drawContours(tempImage, approx, i, CVX_RED, 1, 8);
            SaveOutImage(tempImage, pData, ("324_cvApproxPoly.jpg"));
        }

        for (int j=0; j<approx[i].size(); j++) {
            cv::Point* pt = (cv::Point *)&approx[i][j];
            corners.push_back(*pt);
        }
    }

    cv::Point2f center(0, 0);
    // Get mass center
    for (int i = 0; i < (int)corners.size(); i++)
        center += corners[i];
    center *= (1. / corners.size());

    Point2f pt;
    if (corners.size() > 0)
    {
        pt = getCorner(corners, center, CornerType);
        outCorner = pt;
        ret = 0;
    }
    else
        ret = -1;

    if (m_bSaveEngineImg){
        Point2f pt1 = Point2f(pt.x - 30, pt.y - 30);
        Point2f pt2 = Point2f(pt.x + 30, pt.y + 30);
        cv::rectangle(graySearchImg, pt1, pt2, CV_RGB(0, 84, 255), 3, 8, 0);
        //cvSetImageROI(graySearchImg, cv::Rect((int)pt1.x, (int)pt1.y, 200, 200));
        //cvAddS(graySearchImg, cv::Scalar(150), graySearchImg);
        //cvResetImageROI(graySearchImg);

        cv::circle(graySearchImg, pt, 3, CV_RGB(255, 255, 255), 2);

        SaveOutImage(graySearchImg, pData, ("335_Corners.jpg"));
    }

    return ret;
}

//
// CornerType : 0 - Upper Left, 1 - Upper Right, 2 - Bottom Left, 3 - Bottom Right
//
int CImgProcEngine::EdgeCornerByLine(Qroilib::RoiObject *pData, cv::Mat grayCroppedImgIn, int CornerType, cv::Point2d &outCorner)
{
    outCorner = cv::Point2d();
    QString str;
    cv::Mat grayCroppedImg = grayCroppedImgIn.clone();

    if (m_bSaveEngineImg)
    {
        str.sprintf(("209_GrayImage.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    if (nThresholdValue == 0)
        ThresholdOTSU(pData, grayCroppedImg, 211);
    else
        ThresholdRange(pData, grayCroppedImg, 211);

    NoiseOut(pData, grayCroppedImg, _ProcessValue1, 212);
    Expansion(pData, grayCroppedImg, _ProcessValue1, 213);

    /////////////////////////
    // 가장큰 Blob만 남김
    ////////////////////////
    CBlobResult blobs;
    blobs = CBlobResult(grayCroppedImg);
    double dLargeArea = 0;
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        double dArea = p->Area();
        if (dLargeArea < dArea) {
            dLargeArea = dArea;
        }
    }
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);
    int NumOfBlob = blobs.GetNumBlobs();
    if (NumOfBlob < 1)
    {
        return -1;
    }
    grayCroppedImg = cv::Mat::zeros(grayCroppedImg.size(), grayCroppedImg.type());
    // large blobs.
    CBlob *currentBlob = blobs.GetBlob(0);
    currentBlob->FillBlob(grayCroppedImg, CV_RGB(255, 255, 255));	// Draw the large blobs as white.
    cv::Rect rect = currentBlob->GetBoundingBox();

    if (m_bSaveEngineImg)
    {
        str.sprintf(("217_imageLargeBlobs.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }

    rect.x -= 5;
    rect.y -= 5;
    rect.width += 5;
    rect.height += 5;
    if (rect.x < 0)
        rect.x = 0;
    if (rect.y < 0)
        rect.y = 0;
    if (rect.width+rect.x >= grayCroppedImg.cols)
        rect.width = grayCroppedImg.cols - rect.x;
    if (rect.height+rect.y >= grayCroppedImg.rows)
        rect.height = grayCroppedImg.rows - rect.y;
    cv::Rect roi = cv::Rect((int)rect.x, (int)rect.y, rect.width, rect.height);
    cv::Mat workImg = cv::Mat(cv::Size(rect.width, rect.height), CV_8UC1);
    grayCroppedImg(roi).copyTo(workImg);

    if (m_bSaveEngineImg)
    {
        str.sprintf(("218_workImg.jpg"));
        SaveOutImage(workImg, pData, str);
    }

    double dRejectLow = 0.45; // 양쪽 45%를 버리고 중앙 10%만 사용한다.
    double dRejectHigh = 0.45;

    int w = (int)((float)workImg.cols * 0.3); // ROI영역 30%만 사용한다.
    int h = workImg.rows;

    Point2f pt = Point2f(0,0);
    //Corner Position 에 따라 Line Edge 추출 방향이 다르다.
    switch (CornerType) {
    case 0 : // Upper Left // White to Black edge, 중앙에서 좌, 중앙에서 위
        pt.x = ROIPixEdge(workImg, 1, dRejectLow, dRejectHigh); // Right2Left
        pt.y = ROIPixEdge(workImg, 3, dRejectLow, dRejectHigh); // Bottom2Top
        break;
    case 1: // Upper Right
        pt.x = ROIPixEdge(workImg, 0, dRejectLow, dRejectHigh); // Left2Right
        pt.y = ROIPixEdge(workImg, 3, dRejectLow, dRejectHigh); // Bottom2Top
        break;
    case 2: // Bottom Left
        pt.x = ROIPixEdge(workImg, 1, dRejectLow, dRejectHigh); // Right2Left
        pt.y = ROIPixEdge(workImg, 2, dRejectLow, dRejectHigh); // Top2Bottom
        break;
    case 3: // Bottom Right
        pt.x = ROIPixEdge(workImg, 0, dRejectLow, dRejectHigh); // Left2Right
        pt.y = ROIPixEdge(workImg, 2, dRejectLow, dRejectHigh); // Top2Bottom
        break;
    }
    int ret = 0;
    w = workImg.cols - 1;
    h = workImg.rows - 1;
    if (pt.x <= 1 || pt.x >= w || pt.y <= 1 || pt.y >= h) {
        str.sprintf(("Edge position is out range of ROI"));
        qDebug() << str;
        ret = -1;
    }

    pt.x += rect.x;
    pt.y += rect.y;

    outCorner = pt;
    m_DetectResult.pt = pt;

    if (m_bSaveEngineImg)
    {
        Point2f pt1 = Point2f(pt.x - 20, pt.y - 20);
        Point2f pt2 = Point2f(pt.x + 20, pt.y + 20);
        cv::rectangle(grayCroppedImg, pt1, pt2, CV_RGB(0, 84, 255), 3, 8, 0);

        cv::Rect roi = cv::Rect((int)pt1.x, (int)pt1.y, 40, 40);
        cv::add(grayCroppedImg(roi), cv::Scalar(150), grayCroppedImg(roi));

        cv::circle(grayCroppedImg, pt, 3, CVX_RED, 2);
        str.sprintf(("235_Corners.jpg"));
        SaveOutImage(grayCroppedImg, pData, str);
    }
    return ret;
}

void CImgProcEngine::Smooth(RoiObject *pData, cv::Mat ImgIn, int iImgID)
{
    int param1 = 3;
    int param2 = 0;
    double param3 = 2, param4 = 2;
    int smoothMethod = 1;
    int smoothSize = 3;
    //cv::Mat image = nullptr;
    bool bUse = true;

    if (pData != nullptr)
    {
        CParam *pParam = pData->getParam(("Smooth method"));
        if (pParam) smoothMethod = pParam->Value.toDouble();
        pParam = pData->getParam(("Smooth size"));
        if (pParam) smoothSize = pParam->Value.toDouble();
        // 5x5 행렬
        if (smoothSize == 1) smoothSize = 5;
        // 7x7 행렬
        else if (smoothSize == 2) smoothSize = 7;
        // 9x9 행렬
        else if (smoothSize == 3) smoothSize = 9;
        // 11x11 행렬
        else if (smoothSize == 4) smoothSize = 11;
        // 13x13 행렬
        else if (smoothSize == 5) smoothSize = 13;
        // 15x15 행렬
        else if (smoothSize == 6) smoothSize = 15;
        // 3x3 행렬
        else smoothSize = 3;

        pParam = pData->getParam(("Smooth Use"));
        if (pParam) bUse = (bool)pParam->Value.toDouble();
    }

    if (!bUse) return;

    // This is done so as to prevent a lot of false circles from being detected
    //
    // 1. const CvArr* src
    // 2. CvArr* dst
    // 3. int smooththype=CV_GAUSSIAN
    // 4. int param1=3
    // 5. int param2=0
    // 6. double param3=0
    // 7. double param4=0
    param1 = smoothSize;
    switch (smoothMethod)
    {
    case 0: // blur
        cv::blur(ImgIn,ImgIn,cv::Size(smoothSize,smoothSize));
        break;
    case 1: // Gaussian
        cv::GaussianBlur(ImgIn, ImgIn, cv::Size(smoothSize,smoothSize), 0);
        break;
    case 2: //medianBlur
        cv::medianBlur(ImgIn,ImgIn,smoothSize);
        break;
    case 3: //bilateralFilter
        cv::bilateralFilter(ImgIn,ImgIn,param1,param2,param3);
        break;
    }

    if (m_bSaveEngineImg)
    {
        QString str;
        str.sprintf(("%d_Smooth.jpg"), iImgID);
        SaveOutImage(ImgIn, pData, str);
    }
}

double CImgProcEngine::TemplateMatch(RoiObject *pData, cv::Mat graySearchImgIn, cv::Mat grayTemplateImg, cv::Point &left_top, double &dMatchShapes)
{
    QString str;
    QRectF srect = pData->bounds();
    cv::Mat graySearchImg = graySearchImgIn.clone();

    if (m_bSaveEngineImg)
    {
        //str.sprintf(("%d_graySearchImg.jpg"), 200);
        //SaveOutImage(graySearchImg, pData, str);

        str.sprintf(("%d_grayTemplateImg.jpg"), 201);
        SaveOutImage(grayTemplateImg, pData, str);
    }

    clock_t start_time1 = clock();

    cv::Size size = cv::Size(srect.width() - grayTemplateImg.cols + 1, srect.height() - grayTemplateImg.rows + 1);
    cv::Mat C = cv::Mat(size, CV_32FC1); // 상관계수를 구할 이미지(C)
    double min, max;

    cv::Mat g2 = cv::Mat(cv::Size(grayTemplateImg.cols, grayTemplateImg.rows), CV_8UC1);
    grayTemplateImg.copyTo(g2);

    int nThresholdValue = 0;
    CParam *pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdValue = pParam->Value.toDouble();

    if (nThresholdValue == 0)
        ThresholdOTSU(pData, g2, 211);
    else
        ThresholdRange(pData, g2, 211);

    NoiseOut(pData, g2, _ProcessValue2, 212);
    Expansion(pData, g2, _ProcessValue2, 213);

    int nFilterBlob = 0;
    pParam = pData->getParam(("Filter blob"));
    if (pParam)
        nFilterBlob = (int)pParam->Value.toDouble();
    // 가장큼 or 긴 blob만 남긴다.
    if (nFilterBlob == 1)
        FilterLargeArea(g2);
    else if (nFilterBlob == 2)
        FilterLargeDiameter(g2);
    if (m_bSaveEngineImg){
        SaveOutImage(g2, pData, ("226_FilterBlob.jpg"));
    }

    NoiseOut(pData, g2, _ProcessValue2, 231);
    Expansion(pData, g2, _ProcessValue2, 232);
    cv::Canny(g2, g2, 100, 300);
    if (m_bSaveEngineImg){
        SaveOutImage(g2, pData, ("227_TemplateImageCany.jpg"));
    }

    float dMatchingRate = 0.5f;
    float dMatchShapesingRate = 0.7f;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam("Pattern matching rate");
        if (pParam)
            dMatchingRate = (float)pParam->Value.toDouble() / 100.0f;
    }
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Shape matching rate"));
        if (pParam)
            dMatchShapesingRate = (float)pParam->Value.toDouble() / 100.0f;
    }

    dMatchShapes = 0.0;
    int nLoop = 0;
    double maxRate = 0;
    while (1)
    {
        cv::matchTemplate(graySearchImg, grayTemplateImg, C, cv::TM_CCOEFF_NORMED); // 제곱차 매칭
        cv::minMaxLoc(C, &min, &max, nullptr, &left_top); // 상관계수가 최대값을 값는 위치 찾기
        //str.sprintf(("MatchTemplate : %.3f"), max);
        if (maxRate < max)
            maxRate = max;

        if (max >= dMatchingRate)	// OpenCV의 Template Matching 함수를 이용해서 유사한 패턴을 찾은다음, Shape 기능을 이용하여 한번더 검사한다.
        {
            str.sprintf(("TemplateMatch step%d : %.2f%%"), nLoop, max*100);

            cv::Rect roi = cv::Rect(left_top.x, left_top.y, grayTemplateImg.cols, grayTemplateImg.rows);
            if (m_bSaveEngineImg)
            {
                str.sprintf(("242_MatchImage%d.jpg"), nLoop);
                SaveOutImage(graySearchImg, pData, str);
            }

            if (dMatchShapesingRate > 0)
            {
                cv::Mat g1 = cv::Mat(cv::Size(grayTemplateImg.cols, grayTemplateImg.rows), CV_8UC1);

                graySearchImg(roi).copyTo(g1); // cvSetImageROI() 만큼 Copy
                if (nThresholdValue == 0)
                    ThresholdOTSU(pData, g1, 243);
                else
                    ThresholdRange(pData, g1, 243);

                NoiseOut(pData, g1, _ProcessValue2, 244); // 노이즈 제거
                Expansion(pData, g1, _ProcessValue2, 245);

                // 가장큼 or 긴 blob만 남긴다.
                if (nFilterBlob == 1)
                    FilterLargeArea(g1);
                else if (nFilterBlob == 2)
                    FilterLargeDiameter(g1);
                if (m_bSaveEngineImg){
                    SaveOutImage(g1, pData, ("248_FilterBlob.jpg"));
                }

                NoiseOut(pData, g1, _ProcessValue2, 251);
                Expansion(pData, g1, _ProcessValue2, 252);
                cv::Canny(g1, g1, 100, 300);
                if (m_bSaveEngineImg){
                    str.sprintf(("256_SearchImageCany%d.jpg"), nLoop);
                    SaveOutImage(g1, pData, str);
                }

                // g1이 전부 0로 채워져있으면 cvMatchShapes()에서 0로 리턴되어서 zero image filtering
                //cv::Mat c1 = cvCloneImage(g1);
                //CvSeq* contours = 0;
                //cvFindContours(c1, storage, &contours, sizeof(CvContour), CV_RETR_EXTERNAL);
                //cvReleaseImage(&c1);

                double matching = 1.0;
                //if (contours && contours->total > 0)
                    matching = cv::matchShapes(g1, g2, cv::CONTOURS_MATCH_I1, 0); // 작은 값 일수록 두 이미지의 형상이 가까운 (0은 같은 모양)라는 것이된다.
                g1.release();

                str.sprintf(("MatchTemplate cvMatchShapes : %.3f"), matching);
                theMainWindow->DevLogSave(str.toLatin1().data());

                Point2f pt2 = Point2f((float)grayTemplateImg.cols, (float)grayTemplateImg.rows);
                cv::rectangle(graySearchImg(roi), cv::Point(0, 0), pt2, CV_RGB(128, 128, 128), cv::FILLED); // filled rectangle.

                if (matching > 1.0)
                    matching = 1.0;
                dMatchShapes = (1.0-matching) * 100.0;
                if (matching <= (1.0 - dMatchShapesingRate)) // || max > 0.9)
                {
                    str.sprintf(("Template Shape Match(succ) ===> : %.2f%%"), dMatchShapes);
                    theMainWindow->DevLogSave(str.toLatin1().data());

//                    cvReleaseImage(&graySearchImg);
//                    cvReleaseImage(&C);
//                    cvReleaseImage(&g2);
//                    cvReleaseMemStorage(&storage);

                    m_DetectResult.result = true; // OK

                    //left_top.x += srect.left;
                    //left_top.y += srect.top;
                    //return (max*100);
                    break;
                }
                else {
                    str.sprintf(("Template Shape Match(Fail) ===> : %.2f%%"), dMatchShapes);
                    qDebug() << str;
                    theMainWindow->DevLogSave(str.toLatin1().data());
                    break;
                }
                nLoop++;
                //if (nLoop > 10) {
                //	str.sprintf(("MatchShape failed"));
                //	break;
                //}
            } else {
                str.sprintf(("TemplateMatch Match(succ) ===> : %.2f%%"), maxRate * 100);
                theMainWindow->DevLogSave(str.toLatin1().data());
                m_DetectResult.result = true; // OK
                break;
            }
        }
        else
        {
            nLoop++;
            str.sprintf(("TemplateMatch Result Fail ===> : %.2f%%"), maxRate * 100);
            theMainWindow->DevLogSave(str.toLatin1().data());
            max = maxRate;
            m_DetectResult.result = false; // NG
            break;
        }
    }

    clock_t finish_time1 = clock();
    double total_time = (double)(finish_time1-start_time1)/CLOCKS_PER_SEC;
    str.sprintf("Searching Time=%dms", (int)(total_time*1000));
    theMainWindow->DevLogSave(str.toLatin1().data());

    return max*100.0;// -1;
}


//
// 사용자가 설정한 값으로 Threshold를 실행
//
int CImgProcEngine::Threshold(RoiObject *pData, cv::Mat grayImg, int nDbg)
{
    QString str;

    int nThresholdValue = 70;
    int nThresholdMaxVal = 255;

    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Brightness Threshold"));
        if (pParam)
            nThresholdValue = pParam->Value.toDouble();
        pParam = pData->getParam(("Brightness Max"));
        if (pParam)
            nThresholdMaxVal = pParam->Value.toDouble();
    }

    int nInvert = cv::THRESH_BINARY;
    cv::threshold(grayImg, grayImg, nThresholdValue, nThresholdMaxVal, nInvert);
    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvThreshold.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str);
    }

    return 0;
}

//
// 사용자가 설정한 값으로 Threshold를 실행
//
int CImgProcEngine::ThresholdRange(RoiObject *pData, cv::Mat grayImg, int nDbg)
{
    QString str;

    int nThresholdLowValue = 70;
    int nThresholdHighValue = 255;

    //if (m_bSaveEngineImg){
    //	str.sprintf(("%03d_cvThresholdRange.jpg"), nDbg);
    //	SaveOutImage(grayImg, pData, str, false);
    //}

    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Low Threshold"), -1);
        if (pParam)
            nThresholdLowValue = pParam->Value.toDouble();
        pParam = pData->getParam(("High Threshold"), -1);
        if (pParam)
            nThresholdHighValue = pParam->Value.toDouble();
    }
    if (nThresholdHighValue == 0 && nThresholdLowValue == 0)
        return -1;

    cv::inRange(grayImg, Scalar(nThresholdLowValue), Scalar(nThresholdHighValue), grayImg);

    //if (m_bSaveEngineImg){
    //	str.sprintf(("%03d_cvThresholdRange1.jpg"), nDbg);
    //	SaveOutImage(grayImg, pData, str, false);
    //}

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvThresholdRange.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str);
    }

    return 0;
}

//
// OTUS 알고리즘을 이용하여 히스토그램 Threshold를 실행
//
double CImgProcEngine::ThresholdOTSU(RoiObject *pData, cv::Mat grayImg, int nDbg)
{
    QString str;

    int nThresholdMaxVal = 255;
    if (pData != nullptr) {
        //CParam *pParam;// = pData->getParam(("Brightness Threshold"));
        //if (pParam)
        //	nThresholdValue = pParam->Value.toDouble();
    }

    int nInvert = cv::THRESH_BINARY | cv::THRESH_OTSU;
    double otsuThreshold = cv::threshold(grayImg, grayImg, 0, nThresholdMaxVal, nInvert);
    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvThreshold.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str);
    }

    return otsuThreshold;
}

int CImgProcEngine::AdaptiveThreshold(RoiObject *pData, cv::Mat grayImg, int nDbg)
{
    QString str;
    int nBlkSize   = 41;// hresholding을 적용할 영역 사이즈
    int C = 3;//  평균이나 가중평균에서 차감할 값

    CParam *pParam = pData->getParam(("BlockSize"));
    if (pParam)
        nBlkSize = pParam->Value.toDouble();
    pParam = pData->getParam(("C"));
    if (pParam)
        C = pParam->Value.toDouble();
    if (nBlkSize == 0 && C == 0)
        return -1;

    int cx = grayImg.cols;
    int cy = grayImg.rows;

    //세번째 255는 최대값.  이렇게 하면 0과 255로 이루어진 영상으로 됨
    //마지막에서 두번째값은 threshold 계산 때 주변 픽셀 사용하는 크기.  3,5,7 식으로 홀수로 넣어줌
    //마지막은 Constant subtracted from mean or weighted mean. It may be negative

    //if (C == 0)
    //	C = 11;
    if (nBlkSize > min(cx, cy) / 4)
        nBlkSize = min(cx, cy) / 4;
    if (nBlkSize % 2 == 0) // 강제로 홀수로 만든다.
        nBlkSize++;
    if (nBlkSize < 5)
        nBlkSize = 5;
    cv::adaptiveThreshold(grayImg, grayImg, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, nBlkSize, C);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvAdaptiveThreshold.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str);
    }

    return 0;
}

//
// 모폴리지를 이용하여 잡음제거
//
int CImgProcEngine::NoiseOut(RoiObject *pData, Mat grayImg, int t, int nDbg)
{
    QString str;

    if (t < 0)
        t = _ProcessValue1;

    // 1. Template이미지의 노이즈 제거
    int filterSize = 3;  // 필터의 크기를 3으로 설정 (Noise out area)
    Mat element;
    if (filterSize <= 0)
        filterSize = 1;
    if (filterSize % 2 == 0)
        filterSize++;
    element = getStructuringElement(MORPH_RECT, Size(filterSize,filterSize), Point(-1,-1));

    int nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 1"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            morphologyEx(grayImg, grayImg, MORPH_OPEN, element, Point(-1,-1), -nNoiseout);
        else
            morphologyEx(grayImg, grayImg, MORPH_CLOSE, element, Point(-1,-1), nNoiseout);
    }

    nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 2"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            morphologyEx(grayImg, grayImg, MORPH_OPEN, element, Point(-1,-1), -nNoiseout);
        else
            morphologyEx(grayImg, grayImg, MORPH_CLOSE, element, Point(-1,-1), nNoiseout);
    }

    nNoiseout = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Noise out 3"),t);
        if (pParam)
            nNoiseout = pParam->Value.toDouble();
    }
    if (nNoiseout != 0)
    {
        if (nNoiseout < 0)
            morphologyEx(grayImg, grayImg, MORPH_OPEN, element, Point(-1,-1), -nNoiseout);
        else
            morphologyEx(grayImg, grayImg, MORPH_CLOSE, element, Point(-1,-1), nNoiseout);
    }

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvClose.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str);
    }

    return 0;
}

//
// Dialate / Erode
//
int CImgProcEngine::Expansion(RoiObject *pData, cv::Mat grayImg, int t, int nDbg)
{
    QString str;

    if (t < 0)
        t = _ProcessValue1;
    int nExpansion1 = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Expansion 1"),t);
        if (pParam)
            nExpansion1 = pParam->Value.toDouble();
    }
    int nExpansion2 = 0;
    if (pData != nullptr) {
        CParam *pParam = pData->getParam(("Expansion 2"),t);
        if (pParam)
            nExpansion2 = pParam->Value.toDouble();
    }
    if (nExpansion1 == 0 && nExpansion2 == 0)
        return 0;

    if (nExpansion1 < 0)
        cv::erode(grayImg, grayImg, Mat(), Point(-1,-1), -nExpansion1);
    else
        cv::dilate(grayImg, grayImg, Mat(), Point(-1,-1), nExpansion1);

    if (nExpansion2 < 0)
        cv::erode(grayImg, grayImg, Mat(), Point(-1,-1), -nExpansion2);
    else
        cv::dilate(grayImg, grayImg, Mat(), Point(-1,-1), nExpansion2);

    if (m_bSaveEngineImg){
        str.sprintf(("%03d_cvExpansion.jpg"), nDbg);
        SaveOutImage(grayImg, pData, str);
    }
    return 0;
}

void CImgProcEngine::DrawResultCrossMark(cv::Mat img, RoiObject *pData)
{
    if (img.empty()) return;

    QRectF rect = pData->bounds();	// Area로 등록된 ROI
    rect.normalized();

    if (rect.left() < 0)	rect.setLeft(0);
    if (rect.top() < 0)	rect.setTop(0);
    if (rect.right() >= img.cols) rect.setRight(img.cols);
    if (rect.bottom() >= img.rows) rect.setBottom(img.rows);

    int size = pData->m_vecDetectResult.size();
    for (int i = 0; i < size; i++) {
        DetectResult *prst = &pData->m_vecDetectResult[i];

        double x = 0;//prst->pt.x + rect.x();// / gCfg.m_pCamInfo[0].dResX;
        double y = 0;//prst->pt.y + rect.y();// / gCfg.m_pCamInfo[0].dResY;
        qDebug() << "DrawResultCrossMark" << x << y;

        double w = fabs(prst->br.x - prst->tl.x);
        double h = fabs(prst->br.y - prst->tl.y);
        if (w + h > 0)
        {
            if (prst->resultType == RESULTTYPE_RECT4P)
            {
                x = rect.x() + prst->tl.x + w / 2;
                y = rect.y() + prst->tl.y + h / 2;
                cv::line(img, cv::Point(prst->tl.x,prst->tl.y), cv::Point(prst->tr.x,prst->tr.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
                cv::line(img, cv::Point(prst->tr.x,prst->tr.y), cv::Point(prst->br.x,prst->br.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
                cv::line(img, cv::Point(prst->br.x,prst->br.y), cv::Point(prst->bl.x,prst->bl.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
                cv::line(img, cv::Point(prst->bl.x,prst->bl.y), cv::Point(prst->tl.x,prst->tl.y), cv::Scalar(128, 128, 128), 2, cv::LINE_AA);
            }
            else if (prst->resultType == RESULTTYPE_RECT)
            {
                x = rect.x() + prst->tl.x + w / 2;
                y = rect.y() + prst->tl.y + h / 2;
                Point2f pt1;
                pt1.x = rect.x() + prst->tl.x;
                pt1.y = rect.y() + prst->tl.y;
                Point2f pt2 = Point2f((float)pt1.x+w, (float)pt1.y+h);
                cv::rectangle(img, pt1, pt2, CV_RGB(255, 0, 0), 2);
            }
            else
            {
                x = rect.x() + prst->pt.x;
                y = rect.y() + prst->pt.y;
            }
        }

        if (x > 0 && y > 0)
        {
            cv::Point pt1, pt2;
            pt1.x = x - 40;
            pt1.y = y;
            pt2.x = x + 40;
            pt2.y = y;
            cv::line(img, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
            pt1.x = x;
            pt1.y = y - 40;
            pt2.x = x;
            pt2.y = y + 40;
            cv::line(img, pt1, pt2, CV_RGB(192, 192, 192), 2, 8, 0);
        }
    }
}

void CImgProcEngine::SaveOutImage(cv::Mat imgOut, RoiObject *pData, QString strMsg)
{
    if (!gCfg.m_bSaveEngineImg)
        return;
    QString str = ("");
    if (pData != nullptr)
        str.sprintf("%s/[%s]%s_%s", m_sDebugPath.toStdString().c_str(), pData->groupName().toStdString().c_str(), pData->name().toStdString().c_str(), strMsg.toStdString().c_str());
    else
        str.sprintf("%s/%s", m_sDebugPath.toStdString().c_str(), strMsg.toStdString().c_str());
    cv::imwrite((const char *)str.toStdString().c_str(), imgOut);

    //qDebug() << "SaveOutImage:" << str;
}


int CImgProcEngine::SingleROIOCR(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    if (pData == nullptr)
        return -1;
    QString str;
    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, croppedImage, cv::COLOR_BGR2GRAY);


    tessApi = new tesseract::TessBaseAPI();
    init_tess_failed = tessApi->Init("./tessdata", "eng",  OEM_LSTM_ONLY);
    if (init_tess_failed) {
        qDebug() << "Could not initialize tesseract.";
        return -1;
    }

    //tessApi->SetVariable("tessedit_char_whitelist",
    //    "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ");
    //tessApi->SetVariable("tessedit_char_blacklist",
    //    "~!@#$%^&*()_+`\\[],.;':\"?><");

    // Set Page segmentation mode to PSM_AUTO (3)
    tessApi->SetPageSegMode(tesseract::PSM_AUTO);


    ThresholdRange(pData, croppedImage, 200);

    NoiseOut(pData, croppedImage, _ProcessValue1, 202);
    Expansion(pData, croppedImage, _ProcessValue1, 204);

    int iMinY = 0, iMaxY = 100000;
    if (pData != NULL) {
        CParam *pParam;// = pData->getParam(("Min Size Y"));
        //if (pParam)
        //    iMinY = (int)(pParam->Value.toDouble());
        pParam = pData->getParam(("Max Size Y"));
        if (pParam)
            iMaxY = (int)(pParam->Value.toDouble());
    }
    bitwise_not(croppedImage,croppedImage);
    FilterBlobBoundingBoxYLength(croppedImage, iMinY, iMaxY);
    bitwise_not(croppedImage,croppedImage);
    if (gCfg.m_bSaveEngineImg) {
        str.sprintf(("%03d_Filter.BMP"), 205);
        SaveOutImage(croppedImage, pData, str);
    }

    NoiseOut(pData, croppedImage, _ProcessValue2, 206);
    Expansion(pData, croppedImage, _ProcessValue2, 208);

    if (gCfg.m_bSaveEngineImg) {
        str.sprintf(("%03d_cvClose.BMP"), 210);
        SaveOutImage(croppedImage, pData, str);
    }

    CParam *pParam;
    double dSizeX = 1.0, dSizeY = 1.0;
    pParam = pData->getParam(("Size X(%)"));
    if (pParam)
        dSizeX = (pParam->Value.toDouble()) / 100.0;
    pParam = pData->getParam(("Size Y(%)"));
    if (pParam)
        dSizeY = (pParam->Value.toDouble()) / 100.0;

    cv::Size sz = cv::Size(croppedImage.cols * dSizeX, croppedImage.rows * dSizeY);
    cv::Mat MatIn = cv::Mat(sz, 8, 1);
    cv::resize(croppedImage, MatIn, MatIn.size(), dSizeX, dSizeY, cv::INTER_CUBIC);

    Smooth(pData, MatIn, 220);

    if (gCfg.m_bSaveEngineImg) {
        str.sprintf(("%03d_InputOCR.BMP"), 300);
        SaveOutImage(MatIn, pData, str);
    }

    // Open input image using OpenCV
    cv::Mat im = MatIn;

    // Set image data
    tessApi->SetImage(im.data, im.cols, im.rows, 1, im.step); // BW color

    // Run Tesseract OCR on image
    char* rst = tessApi->GetUTF8Text();
    //string outText = string();

    // print recognized text
    //cout << outText << endl; // Destroy used object and release memory ocr->End();

    std::string sstr = rst;
    size_t endpos = sstr.find_last_not_of(" \t\r\n"); // rtrim
      if(std::string::npos != endpos )
        sstr = sstr.substr(0, endpos+1);
    size_t startpos = sstr.find_first_not_of(" \t\r\n"); // ltrim
      if(std::string::npos != startpos )
        sstr = sstr.substr(startpos);

    qDebug() << "OCR Text:" << sstr.c_str();
    m_DetectResult.pt = cv::Point(0,0);
    strcpy(m_DetectResult.str, sstr.c_str());
    pData->m_vecDetectResult.push_back(m_DetectResult);

//    CvFont font;
//    cvInitFont(&font, CV_FONT_HERSHEY_SIMPLEX, 0.3, 0.3, 0, 1, CV_AA);

//    cv::Rectangle(outImg, cv::Point(0,0), cv::Point(outImg.cols,10), cv::Scalar(255, 255, 255), cv::FILLED);
//    cvPutText(outImg, text, cvPoint(10, 10), &font, cv::Scalar(128, 128, 128));

    //QString str;
    str.sprintf("OCR Text:%s", sstr.c_str());
    theMainWindow->DevLogSave(str.toLatin1().data());


    bitwise_not(im, im);
    CBlobResult blobs;
    blobs = CBlobResult(im);
    int nBlobs = blobs.GetNumBlobs();
    Mat result = cv::Mat::zeros(cv::Size(im.rows*nBlobs, im.rows), im.type());
    int wpos = 0;
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        cv::Rect rect = p->GetBoundingBox();
        wpos = wpos + 12;
        p->FillBlob(result, CVX_WHITE, p->MinX()*-1 + wpos, 0, true);
        wpos += rect.width;
    }
    bitwise_not(result, result);
    str.sprintf(("%03d_InputOCR.BMP"), 400);
    SaveOutImage(result, pData, str);

    //init_tess_failed = tessApi->Init("./tessdata", "eng",  OEM_LSTM_ONLY);
    //tessApi->SetPageSegMode(tesseract::PSM_AUTO);

    tessApi->SetImage(result.data, result.cols, result.rows, 1, result.step); // BW color
    char* rst1 = tessApi->GetUTF8Text();
    qDebug() << ":" << QString(rst1);



    if (tessApi)
        tessApi->End();

    return -1;
}

int CImgProcEngine::SingleROIBarCode(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    QString str;
    QZXing *qz = new QZXing(nullptr);

    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, croppedImage, cv::COLOR_BGR2GRAY);

    ThresholdRange(pData, croppedImage, 130);
    AdaptiveThreshold(pData, croppedImage, 131);

    NoiseOut(pData, croppedImage, _ProcessValue3, 131);


    double area = 0;
    CParam *pParam = pData->getParam(("Area"));
    if (pParam)
       area = pParam->Value.toDouble();
    if (area > 0)
    {
        bitwise_not(croppedImage, croppedImage);

        CBlobResult blobs;
        blobs = CBlobResult(croppedImage);
        int nBlobs = blobs.GetNumBlobs();
        for (int i = 0; i < nBlobs; i++) {
            CBlob *p = blobs.GetBlob(i);
            if (area > p->Area())
                p->ClearContours();
            cv::Rect r = p->GetBoundingBox();
            if (r.width > 20)
                p->ClearContours();
        }
        int type = croppedImage.type();
        croppedImage = cv::Mat::zeros(croppedImage.size(), type);

        for (int i = 0; i < nBlobs; i++)
        {
            CBlob *p = blobs.GetBlob(i);
            p->FillBlob(croppedImage, CVX_WHITE, 0, 0, true);
        }
        bitwise_not(croppedImage, croppedImage);

    }

    if (gCfg.m_bSaveEngineImg) {
        str.sprintf(("%03d_Input.BMP"), 210);
        SaveOutImage(croppedImage, pData, str);
    }

    //cv::Size sz = cv::Size(256, 256);
    //cv::Mat MatIn = cv::Mat(sz, 8, 1);
    //cv::resize(croppedImage, MatIn, MatIn.size());
    //cv::Mat m = MatIn;
    cv::Mat m = croppedImage;


    int nGaussian = 3;
    try {
        cv::GaussianBlur(m, m, cv::Size(nGaussian,nGaussian), 0);
    } catch (...) {
        qDebug() << "Error g2 cvSmooth()";
    }


    QImage img = MatToQImage(m);

    QString decode;
    try {
        decode = qz->decodeImage(img);
    }
    catch(zxing::NotFoundException  &e){}
    catch(zxing::ReaderException  &e){}

    qDebug() << "Barcode :" << decode;

    str = "Barcode : " + decode;
    theMainWindow->DevLogSave(str.toLatin1().data());
    m_DetectResult.resultType = RESULTTYPE_STRING;
    strcpy(m_DetectResult.str, str.toLatin1().data());
    pData->m_vecDetectResult.push_back(m_DetectResult);

    delete qz;

    cv::QRCodeDetector detector;
    vector<Point> points;
    if (detector.detect(m, points)) {
        polylines(m, points, true, Scalar(0, 255, 255), 2);
        String info = detector.decode(m, points);
        if (!info.empty()) {
            polylines(m, points, true, Scalar(0, 0, 255), 2);
            qDebug() << "OpenCV Barcode :" << info.c_str();
        }
        if (gCfg.m_bSaveEngineImg) {
            str.sprintf(("%03d_Rst.BMP"), 310);
            SaveOutImage(m, pData, str);
        }
    }

    return 0;
}

// ref https://docs.opencv.org/3.4/dd/d49/tutorial_py_contour_features.html
// https://stackoverflow.com/questions/47910428/contour-width-measurement-along-its-entire-lendth 이미지 사용.
int CImgProcEngine::SingleROILineMeasurement(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    if (pData == nullptr)
        return -1;
    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, croppedImage, cv::COLOR_BGR2GRAY);

    int size1 = 1;
    int morph_size = 3;
    int thinningType = 0;
    CParam *pParam = pData->getParam(("Size1"));
    if (pParam)
       size1 = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("MorphSize"));
    if (pParam)
       morph_size = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("thinningType"));
    if (pParam)
       thinningType = (int)pParam->Value.toDouble();

    NoiseOut(pData, croppedImage);

    QString str;
    cv::Mat mat = croppedImage;

    cv::Mat ZS;
    if (thinningType == 0)
        cv::ximgproc::thinning(mat, ZS, ximgproc::THINNING_ZHANGSUEN);
    else
        cv::ximgproc::thinning(mat, ZS, ximgproc::THINNING_GUOHALL);

    //cv::Mat element7(7, 7, CV_8U, cv::Scalar(1));
    //morphologyEx( ZS, ZS, cv::MORPH_DILATE, element7 );

    if (m_bSaveEngineImg)
    {
        str.sprintf(("201_mat.jpg"));
        SaveOutImage(mat, pData, str);
        str.sprintf(("201_ZH.jpg"));
        SaveOutImage(ZS, pData, str);
    }

    vector<vector<cv::Point> > contours;
    cv::findContours( mat, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    mat = cv::Mat::zeros(mat.size(), mat.type());

    vector<vector<cv::Point> > cvec;
    cv::findContours( ZS, cvec, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);
    for (int i=0; i<cvec.size(); i++) {
        vector<cv::Point> approx;
        approxPolyDP(Mat(cvec[i]), approx, 0.4, false); // eps는 0.4 or 0.5가 적합.

        vector<ElemLineIt> vecLineIt;
        vecLineIt.clear();
        OneLineMeasurement(mat, approx, pData, vecLineIt);

        drawContours(mat, contours, i, CVX_WHITE, 2);
        if (m_bSaveEngineImg)
        {
            str.sprintf(("205_matcont_%d.jpg"), i);
            SaveOutImage(mat, pData, str);
        }

        int linecnt = 0;
        double total = 0;
        double maxwidth = 0;
        double minwidth = 99999;

        Rect bounds(0, 0, mat.cols, mat.rows);
        for (int j=0; j<vecLineIt.size(); j++) {
            ElemLineIt *e = &vecLineIt[j];
            LineIterator it1(mat, e->center, e->first, 8);
            int cnt = 0;
            while(bounds.contains(it1.pos())) {
                uchar pixel = mat.at<uchar>(it1.pos());
                if (pixel == 255) {
                    e->p1 = it1.pos();
                    break;
                }
                 Point pt = it1.pos();
                 mat.at<uchar>(pt.y, pt.x) = 128;
                 it1++;
                 cnt++;
            }
            LineIterator it2(mat, e->center, e->second, 8);
            while(bounds.contains(it2.pos())) {
                uchar pixel = mat.at<uchar>(it2.pos());
                if (pixel == 255) {
                    e->p2 = it2.pos();
                    break;
                }
                 Point pt = it2.pos();
                 mat.at<uchar>(pt.y, pt.x) = 128;
                 it2++;
                 cnt++;
            }
            cnt--;
            e->len = cnt; // pixel count

            if (e->len > 0)
            {
                total += e->len;
                linecnt++;
                if (e->len > maxwidth)
                    maxwidth = e->len;
                if (e->len < minwidth)
                    minwidth = e->len;
            }
        }

        str.sprintf(("LineMeasure Avg=%.1f Min=%.1f Max=%.1f"),
                    total/linecnt, minwidth, maxwidth);
        qDebug() << str;
        theMainWindow->DevLogSave(str.toLatin1().data());

    }

    if (m_bSaveEngineImg)
    {
        str.sprintf(("210_matout.jpg"));
        SaveOutImage(mat, pData, str);
    }
    theMainWindow->outWidget("LineMeasurement", mat);

    return 0;
}

int CImgProcEngine::SingleROIColorMatching(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    QString str;
    if (pData == nullptr || pData->templateMat.empty())
        return -1;


    cv::Size templateSize = cv::Size(pData->templateMat.cols, pData->templateMat.rows);
    cv::Mat templateImg = cv::Mat(templateSize, CV_8UC3);
    if (pData->templateMat.channels() == 3)
        pData->templateMat.copyTo(templateImg);
    else
        cv::cvtColor(pData->templateMat, templateImg, cv::COLOR_GRAY2BGR);


    int method = 1; // Hue+Saturation
    int hbins = 30;
    int sbins = 32;
    CParam *pParam = pData->getParam(("Method"));
    if (pParam)
       method = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("hbins"));
    if (pParam)
       hbins = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("sbins"));
    if (pParam)
       sbins = (int)pParam->Value.toDouble();
    int nThresholdLowValue = 0;
    pParam = pData->getParam(("Low Threshold"));
    if (pParam)
        nThresholdLowValue = pParam->Value.toDouble();
    int nThresholdHighValue = 0;
    pParam = pData->getParam(("High Threshold"));
    if (pParam)
        nThresholdHighValue = pParam->Value.toDouble();

    Mat ref_hsv=templateImg;
    Mat tar_hsv =croppedImage;
    //cv::imshow("templateImg", ref_hsv);
    //cv::imshow("searchImg", tar_hsv);

    // Quantize the hue to 30 levels
    // and the saturation to 32 levels
    int histSize[] = {hbins, sbins};
    // hue varies from 0 to 179, see cvtColor
    float hranges[] = { 0, 180 };
    // saturation varies from 0 (black-gray-white) to
    // 255 (pure spectrum color)
    float sranges[] = { 0, 255 };
    const float* ranges[] = { hranges, sranges };
    // we compute the histogram from the 0-th and 1-st channels
    int channels[] = {0, 1};

    int dims = 2;
    if (method == 0) // Hue Only
    {
        dims = 1;
    } else {
        dims = 2;
    }

    cv::Mat hist;
    cvtColor(ref_hsv, ref_hsv, COLOR_BGR2HSV);
    //GaussianBlur(ref_hsv, ref_hsv, Size(3, 3), 0);

    calcHist(&ref_hsv,
          1, // Number of source images.
          channels, // List of the dims channels used to compute the histogram.
          cv::Mat(), // do not use mask
          hist,  // Output histogram
          dims, // dims
          histSize, // Array of histogram sizes in each dimension. BINS 값.
          ranges,  // Array of the dims arrays of the histogram bin boundaries in each dimension. Range값.
          true, // the histogram is uniform
          false
      );

    // Plot the histogram
    double maxVal=0;
    minMaxLoc(hist, 0, &maxVal, 0, 0);
    int scalew = 10;
    int scaleh = 1;
    int hist_h = sbins*scaleh;
    int hist_w = hbins*scalew;
    if (dims == 1)
        hist_h = hbins;
    Mat histImage( hist_h, hist_w, CV_8UC1, Scalar( 0,0,0) );

    if (dims == 2)
    {
        for( int h = 0; h < hbins; h++ )
            for( int s = 0; s < sbins; s++ )
            {
                float binVal = hist.at<float>(h, s);
                int intensity = cvRound(binVal*255/maxVal);
                rectangle( histImage, Point(h*scalew, s*scaleh),
                            Point( (h+1)*scalew - 1, (s+1)*scaleh - 1),
                            Scalar::all(intensity),
                            -1 );
            }
    }
    else
    {
        normalize(hist, hist, 0, histImage.rows, NORM_MINMAX, -1, Mat() );
        for (int k=0; k<dims; k++)
        {
            int bin_w = cvRound( (double) hist_w/histSize[k] );
            for( int i = 1; i < histSize[k]; i++ )
            {
                line( histImage, Point( bin_w*(i-1), hist_h - cvRound(hist.at<float>(i-1)) ) ,
                           Point( bin_w*(i), hist_h - cvRound(hist.at<float>(i)) ),
                           Scalar( 255, 0, 0), 2, 8, 0  );
            }
        }
    }
    if (m_bSaveEngineImg)
    {
        str.sprintf(("130_hist.jpg"));
        SaveOutImage(histImage, pData, str);
    }


    //cv::normalize(hist, hist, 1.0);
    cv::normalize(hist, hist, 0, 1, cv::NORM_MINMAX);

    cv::Mat result;
    cvtColor(tar_hsv, tar_hsv, COLOR_BGR2HSV);
    //GaussianBlur(tar_hsv, tar_hsv, Size(3, 3), 0);
    calcBackProject(&tar_hsv,
         1, // Number of source images.
         channels,// The list of channels used to compute the back projection.
         hist,    // Input histogram that can be dense or sparse.
         result,  // Destination back projection array that is a single-channel array of the same size and depth as images[0].
         ranges,  // Array of arrays of the histogram bin boundaries in each dimension.
         255.0    // Optional scale factor for the output back projection.
    );
    if (m_bSaveEngineImg)
    {
        str.sprintf(("131_bp.jpg"));
        SaveOutImage(result, pData, str);
    }

    cv::threshold(result, result, nThresholdLowValue, nThresholdHighValue, cv::THRESH_BINARY);

    NoiseOut(pData, result, _ProcessValue3, 131);

    double area = 0;
    pParam = pData->getParam(("Area"));
    if (pParam)
       area = pParam->Value.toDouble();
    if (area > 0)
    {
        CBlobResult blobs;
        blobs = CBlobResult(result);
        int nBlobs = blobs.GetNumBlobs();
        for (int i = 0; i < nBlobs; i++) {
            CBlob *p = blobs.GetBlob(i);
            if (area > p->Area()) {
                p->ClearContours();
            }
        }
        int type = result.type();
        result = cv::Mat::zeros(cv::Size(result.cols, result.rows), type);

        for (int i = 0; i < nBlobs; i++)
        {
            CBlob *p = blobs.GetBlob(i);
            p->FillBlob(result, CVX_WHITE, 0, 0, true);
        }
    }

    if (m_bSaveEngineImg)
    {
        str.sprintf(("134_Area.jpg"));
        SaveOutImage(result, pData, str);
    }

    //cv::namedWindow("Back Project Result");
    //cv::imshow("Back Project Result", result);
    CBlobResult blobs;
    blobs = CBlobResult(result);
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        cv::Point pt = p->getCenter();
        cv::Rect r1 = p->GetBoundingBox();
        m_DetectResult.resultType = RESULTTYPE_RECT;
        m_DetectResult.pt = cv::Point2d(pt.x, pt.y);
        m_DetectResult.tl = cv::Point2d(r1.x, r1.y);
        m_DetectResult.br = cv::Point2d(r1.x + r1.width, r1.y + r1.height);

        cv:Rect rect = r1;
        Mat tar = tar_hsv(rect);
        double per = HistEMD(ref_hsv, tar, dims);
        m_DetectResult.dMatchRate = per;
        pData->m_vecDetectResult.push_back(m_DetectResult);
    }

    int iMatchingResult = 0;
    pParam = pData->getParam(("Matching Result"));
    if (pParam)
       iMatchingResult = (int)pParam->Value.toDouble();
    if (iMatchingResult == 0) { // TopOne
        std::stable_sort(pData->m_vecDetectResult.begin(), pData->m_vecDetectResult.end(), [](const DetectResult lhs, const DetectResult rhs)->bool {
            if (lhs.dMatchRate > rhs.dMatchRate) // descending
                return true;
            return false;
        });
        int size = pData->m_vecDetectResult.size();
        for (int i=size-1; i>0; i--) {
            pData->m_vecDetectResult.erase(pData->m_vecDetectResult.begin()+i);
        }
    }
    int size = pData->m_vecDetectResult.size();
    for (int i=0; i<size; i++) {
        DetectResult *pRst = &pData->m_vecDetectResult[i];

        str.sprintf(("ColorMatching(%d) Rate=%.1f"), i, pRst->dMatchRate);
        qDebug() << str;
        theMainWindow->DevLogSave(str.toLatin1().data());
    }

    return 0;
}

//Ref : http://study.marearts.com/2014/11/opencv-emdearth-mover-distance-example.html
double CImgProcEngine::HistEMD(cv::Mat& ref_hsv, cv::Mat& target, int dims)
{
    int hbins = 30, sbins = 32;
    int channels[] = {0,  1};
    int histSize[] = {hbins, sbins};
    float hranges[] = { 0, 180 };
    float sranges[] = { 0, 255 };
    const float* ranges[] = { hranges, sranges};

    Mat hist1;
    calcHist( &ref_hsv, 1, channels,  Mat(),// do not use mask
     hist1, dims, histSize, ranges,
     true, // the histogram is uniform
     false );
    normalize(hist1, hist1, 0, 1, cv::NORM_MINMAX);

    Mat hist2;
     calcHist( &target, 1, channels,  Mat(),// do not use mask
      hist2, dims, histSize, ranges,
      true, // the histogram is uniform
      false );
     normalize(hist2, hist2, 0, 1, cv::NORM_MINMAX);

     //compare histogram
     ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
     int numrows = hbins * sbins;

     //make signature
     Mat sig1(numrows, 3, CV_32FC1);
     Mat sig2(numrows, 3, CV_32FC1);

     //fill value into signature
     for(int h=0; h< hbins; h++)
     {
      for(int s=0; s< sbins; ++s)
      {
       float binval = 0;
       if (dims == 2)
            binval = hist1.at< float>(h,s);
       else
            binval = hist1.at< float>(h);
       sig1.at< float>( h*sbins + s, 0) = binval;
       sig1.at< float>( h*sbins + s, 1) = h;
       sig1.at< float>( h*sbins + s, 2) = s;

       if (dims == 2)
            binval = hist2.at< float>(h,s);
       else
            binval = hist2.at< float>(h);
       sig2.at< float>( h*sbins + s, 0) = binval;
       sig2.at< float>( h*sbins + s, 1) = h;
       sig2.at< float>( h*sbins + s, 2) = s;
      }
     }

     //compare similarity of 2images using emd.
     float emd = cv::EMD(sig1, sig2, cv::DIST_L2); //emd 0 is best matching.
     if (emd >= 1)
         emd = 0.95;
     else emd = (1-emd)*100;
     //qDebug() << "similarity " << perc<< "%%";
     return emd;

}


int CImgProcEngine::OneLineMeasurement(cv::Mat& mat, vector<Point>& cone, RoiObject *pData, vector<ElemLineIt> &vecLineIt)
{
    ElemLineIt e1;
    int rst = -1;

    const int llen = 512;//13;
    int interval = 7;
    CParam *pParam = pData->getParam(("Interval"));
    if (pParam)
       interval = (int)pParam->Value.toDouble();

//    int size1 = 0;
//    int size2 = 99999;
//    pParam = pData->getParam(("Start"));
//    if (pParam)
//       size1 = (int)pParam->Value.toDouble();
//    pParam = pData->getParam(("End"));
//    if (pParam)
//       size2 = (int)pParam->Value.toDouble();

    int size = cone.size();
    for (int i = 0; i < size-interval; i=i+interval)
    {
//        if (i >= size1 && i < size2) {
//            static int ii = 0;
//            ii++;
//        }
//        else continue;

        float x0 = cone[i+interval/2].x;
        float y0 = cone[i+interval/2].y;

        vector<double> vAngle;
        vAngle.clear();
        double dAngle = 0;
        int mid = interval / 2;
        for (int j=i; j<i+interval; j++) {
            if (cone.size() <= j+2)
                break;
            float x1 = cone[j].x;
            float y1 = cone[j].y;
            float x2 = cone[j+2].x;
            float y2 = cone[j+2].y;

            double d1 = sqrt(pow(x1 - x2, 2) + pow(y1 - y2, 2));
            double d2 = sqrt(pow(cone[j+1].x - x1, 2) + pow(cone[j+1].y - y1, 2));
            //되돌아 나오는 꼭지점 처리를 위함.
            if (d1 < 3.0 && d2 >= 3.0)
            {
                mid = vAngle.size();
                x1 = x0 = cone[j+1].x;
                y1 = y0 = cone[j+1].y;
            }
            dAngle = ((double)atan2(y1 - y2, x1 - x2) * 180.0f / PI);
            vAngle.push_back(dAngle);
            //line( m, Point(x1,y1), Point(x2,y2), CV_RGB(221,221,221), 1, 8 );
        }
        if (vAngle.size() == 0)
            continue;

        if (mid != (interval / 2)) {
            std::stable_sort(vAngle.begin(), vAngle.end(), [](const double lhs, const double rhs)->bool {
                return lhs < rhs;
            });
        }
        //dAngle = 0;
        dAngle = vAngle[mid];
        //qDebug() << "angle1: " << dAngle;
        MakeOneElemLine(Point(x0,y0), dAngle, e1);
        AppendOneLine(mat, vecLineIt, e1, interval, dAngle);

        // 최종 종단점 처리를 위해 필요.
        if (i+interval >= size-interval) {
            x0 = cone[size-1].x;
            y0 = cone[size-1].y;
            size = vAngle.size();
            dAngle = vAngle[size-1];
            MakeOneElemLine(Point(x0,y0), dAngle, e1);
            AppendOneLine(mat, vecLineIt, e1, interval, dAngle);
        }
    }

    return rst;
}

void CImgProcEngine::MakeOneElemLine(Point cen, double dAngle, ElemLineIt &elem)
{
    const int llen = 512;//13;
    double a2 = (dAngle + 90);
    double s = sin((a2)*CV_PI/180);
    double c = cos((a2)*CV_PI/180);
    Point p2(cen.x+c*llen, cen.y+s*llen);
    elem.center = Point(cen.x,cen.y);
    elem.first = p2;

    a2 = (dAngle - 90);
    s = sin((a2)*CV_PI/180);
    c = cos((a2)*CV_PI/180);
    p2 = Point(cen.x+c*llen, cen.y+s*llen);
    elem.second = p2;
}

int CImgProcEngine::AppendOneLine(cv::Mat& mat, vector<ElemLineIt> &vecLineIt, ElemLineIt ee, int interval, double dAngle)
{
    ElemLineIt e1;
    int size = vecLineIt.size();
    // interval 보다 거리가 멀면 중간에 같은 각도의 라인을 추가한다.
    ElemLineIt le = ee;
    if (size > 0)
        le.center = vecLineIt[size-1].center;
    double d = sqrt(pow(le.center.x - ee.center.x, 2) + pow(le.center.y - ee.center.y, 2));
    if (d > interval) {
        LineIterator it(mat, le.center, ee.center, 8);
        vector<Point> buf(it.count);
        for(int i = 0; i < it.count; i++, ++it) {
            buf[i] = it.pos();
        }
        for(int i=interval; i<it.count; i=i+interval) {
            MakeOneElemLine(buf[i], dAngle, e1);
            double d1 = sqrt(pow(e1.first.x - e1.second.x, 2) + pow(e1.first.y - e1.second.y, 2));
            if (d1 > 3)
                vecLineIt.push_back(e1);
        }
    }
    double d1 = sqrt(pow(ee.first.x - ee.second.x, 2) + pow(ee.first.y - ee.second.y, 2));
    if (d1 > 3)
        vecLineIt.push_back(ee);
    return 0;
}

int CImgProcEngine::SingleROILabelDetect(cv::Mat croppedImage, Qroilib::RoiObject *pData, QRectF rect)
{
    if (pData == nullptr)
        return -1;
    QString str;

    if (croppedImage.channels() == 3)
        cv::cvtColor(croppedImage, croppedImage, cv::COLOR_BGR2GRAY);

    Mat src = croppedImage.clone();

    ThresholdRange(pData, src, 110);
    NoiseOut(pData, src, -1, 125);

    int nThreshold1 = 100;
    int nThreshold2 = 300;
    int nEdgeClose = 1;
    CParam *pParam = pData->getParam(("EdgeThreshold1"));
    if (pParam)	nThreshold1 = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("EdgeThreshold2"));
    if (pParam)	nThreshold2 = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("EdgeClose"));
    if (pParam)	nEdgeClose = (int)pParam->Value.toDouble();

    int nGaussian = 3;
    try {
        cv::GaussianBlur(src, src, cv::Size(nGaussian,nGaussian), 0);
    } catch (...) {
        qDebug() << "Error g2 cvSmooth()";
    }
    Mat dst;
    cv::Canny(src, dst, nThreshold1, nThreshold2);
    cv::Mat element1(nEdgeClose, nEdgeClose, CV_8U, cv::Scalar(1));
    morphologyEx(src, src, MORPH_CLOSE, element1, Point(-1,-1), 1);
    if (m_bSaveEngineImg){
        SaveOutImage(dst, pData, ("201_Canny.jpg"));
    }

    double dLen1 = 200;
    double dLen2 = 500;
    double dArea = 10000;
    pParam = pData->getParam(("Min Len"));
    if (pParam)	dLen1 = pParam->Value.toDouble();
    pParam = pData->getParam(("Max Len"));
    if (pParam)	dLen2 = pParam->Value.toDouble();
    pParam = pData->getParam(("Area"));
    if (pParam)	dArea = pParam->Value.toDouble();

    CBlobResult blobs = CBlobResult(dst);
    int nBlobs = blobs.GetNumBlobs();
    for (int i=0; i<nBlobs; i++)
    {
        CBlob *p = blobs.GetBlob(i);
        cv::Rect rect = p->GetBoundingBox();
        int l1 = min(rect.width, rect.height);
        int l2 = max(rect.width, rect.height);
        if (l1 < dLen1 || l2 > dLen2)
            p->ClearContours();
    }
    Mat bdst = cv::Mat::zeros(dst.size(), dst.type());
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *p = blobs.GetBlob(i);
        double area = p->Area();
        if (area > dArea)
            p->ClearContours();
        p->FillBlob(bdst, CVX_WHITE, 0, 0, false);
    }
    if (m_bSaveEngineImg){
        SaveOutImage(bdst, pData, ("250_Blob.jpg"));
    }

    //cv::dilate(bdst, bdst, Mat(), Point(-1,-1), 5);
    blobs = CBlobResult(bdst);
    nBlobs = blobs.GetNumBlobs();
    bdst = cv::Mat::zeros(dst.size(), dst.type());
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *p = blobs.GetBlob(i);
        double area = p->Area();
        if (area < dArea)
            p->ClearContours();
        p->FillBlob(bdst, CVX_WHITE, 0, 0, false);
    }
    if (m_bSaveEngineImg){
        SaveOutImage(bdst, pData, ("260_Blob.jpg"));
    }

    int e1 = 15;
    int e2 = 25;
    pParam = pData->getParam(("Close"));
    if (pParam)	e1 = (int)pParam->Value.toDouble();
    pParam = pData->getParam(("Open"));
    if (pParam)	e2 = (int)pParam->Value.toDouble();

    cv::Mat element7(e1, e1, CV_8U, cv::Scalar(1));
    morphologyEx(bdst, bdst, MORPH_CLOSE, element7, Point(-1,-1), 1);
    cv::Mat element11(e2, e2, CV_8U, cv::Scalar(1));
    morphologyEx(bdst, bdst, MORPH_OPEN, element11, Point(-1,-1), 1);
    if (m_bSaveEngineImg){
        SaveOutImage(bdst, pData, ("270_Blob.jpg"));
    }
    Mat src2 = bdst.clone();


    cv::GaussianBlur(bdst, bdst, cv::Size(nGaussian,nGaussian), 0);
    vector<vector<Point>> contours;
    findContours(bdst, contours,  cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    Mat drawImage = cv::Mat::zeros(src.size(), src.type());
    int size = contours.size();
    vector<vector<Point>> approx(size);
    for (int k=0; k<size; k++)
    {
        Rect rect = boundingRect(contours[k]);
        int l1 = min(rect.width, rect.height);
        int l2 = max(rect.width, rect.height);
        if (l1 < dLen1 || l2 > dLen2)
            continue;

        approxPolyDP(Mat(contours[k]), approx[k], arcLength(Mat(contours[k]), true)*0.04, true);
        if (approx[k].size() == 0)
                continue;

        int size = approx[k].size();
        vector<int> angle;
        for (int a = 0; a < size; a++) {
            int ang = GetAngleABC(approx[k][a], approx[k][(a + 1) % size], approx[k][(a + 2)%size]);
            angle.push_back(ang);
        }

        std::sort(angle.begin(), angle.end());
        //int minAngle = angle.front();
        int maxAngle = angle.back();
        if (size == 4 && maxAngle < 120)
        {
            cv::drawContours(drawImage, approx, k, CVX_WHITE, 3, 8);
            str.sprintf(("301_approxPoly.jpg"));
            SaveOutImage(drawImage, pData, str);
        }
    }


    croppedImage.copyTo(src);

    ThresholdRange(pData, src, 110);
    NoiseOut(pData, src, -1, 125);

    cv::Mat element15(20, 20, CV_8U, cv::Scalar(1));
    morphologyEx(src, src, MORPH_OPEN, element15, Point(-1,-1), 1);
    if (m_bSaveEngineImg){
        SaveOutImage(src, pData, ("501_Blob.jpg"));
    }

    // Circl find

    double dArea2 = 10000;
    pParam = pData->getParam(("Area2"));
    if (pParam)	dArea2 = pParam->Value.toDouble();

    blobs = CBlobResult(src);
    nBlobs = blobs.GetNumBlobs();
    bdst = cv::Mat::zeros(dst.size(), dst.type());
    for (int i = 0; i < nBlobs; i++)
    {
        CBlob *p = blobs.GetBlob(i);
        double area = p->Area();
        if (area < 1000 || area > dArea2)
            p->ClearContours();
        p->FillBlob(bdst, CVX_WHITE, 0, 0, false);
    }
    bitwise_or(bdst, src2, bdst);
    if (m_bSaveEngineImg){
        SaveOutImage(bdst, pData, ("511_Blob.jpg"));
    }


    return 0;
}
