//
//QROILIB : QT Vision ROI Library
//Copyright 2018, Created by Yoon Jong-lock <jerry1455@gmail.com,jlyoon@yj-hitech.com>
//

#include "imgprocbase.h"
using namespace cv;


namespace Qroilib
{


CImgProcBase::CImgProcBase()
{
}

CImgProcBase::~CImgProcBase()
{
}

cv::Mat CImgProcBase::shiftFrame(cv::Mat frame, int pixels, eShiftDirection direction)
{
    //create a same sized temporary Mat with all the pixels flagged as invalid (-1)
    cv::Mat temp = cv::Mat::zeros(frame.size(), frame.type());

    switch (direction)
    {
    case(ShiftUp) :
        frame(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, 0, temp.cols, temp.rows - pixels)));
        break;
    case(ShiftRight) :
        frame(cv::Rect(0, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)));
        break;
    case(ShiftDown) :
        frame(cv::Rect(0, 0, frame.cols, frame.rows - pixels)).copyTo(temp(cv::Rect(0, pixels, frame.cols, frame.rows - pixels)));
        break;
    case(ShiftLeft) :
        frame(cv::Rect(pixels, 0, frame.cols - pixels, frame.rows)).copyTo(temp(cv::Rect(0, 0, frame.cols - pixels, frame.rows)));
        break;
    default:
        std::cout << "Shift direction is not set properly" << std::endl;
    }

    return temp;
}

void CImgProcBase::CopyImageROI(cv::Mat& imgIn, cv::Mat& imgOut, cv::Rect rect)
{
    imgOut = imgIn(rect);
}


int CImgProcBase::FilterBlobLength(cv::Mat grayImg, int nMinLength, int nMaxLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);

    //double_stl_vector elong = blobs.GetSTLResult(CBlobGetLength());
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetLength(), B_LESS, nMinLength); // 작은 Length제거
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetLength(), B_GREATER, nMaxLength); // 큰 Length 제거
    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
        }
    }
    return 0;
}

int CImgProcBase::FilterBlobBoundingBoxLength(cv::Mat grayImg, int nMinLength, int nMaxLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            cv::Rect rect = currentBlob->GetBoundingBox();

            if (rect.width > nMaxLength || rect.height > nMaxLength) {
                currentBlob->ClearContours();
            }
            else if (rect.width < nMinLength || rect.height < nMinLength) {
                currentBlob->ClearContours();
            }
        }
    }
    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
        }
    }

    return 0;
}

//
// blob의 폭이 nMinLength nMaxLength 사이인것만 남긴다.
//
int CImgProcBase::FilterBlobBoundingBoxXLength(cv::Mat grayImg, int nMinLength, int nMaxLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            cv::Rect rect = currentBlob->GetBoundingBox();

            if (rect.width > nMaxLength) {
                currentBlob->ClearContours();
            }
            else if (rect.width < nMinLength) {
                currentBlob->ClearContours();
            }
        }
    }

    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
        }
    }

    return 0;
}
//
// blob의 높이가 nMinLength nMaxLength 사이인것만 남긴다.
//
int CImgProcBase::FilterBlobBoundingBoxYLength(cv::Mat grayImg, int nMinLength, int nMaxLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            cv::Rect rect = currentBlob->GetBoundingBox();

            if (rect.height > nMaxLength) {
                currentBlob->ClearContours();
            }
            else if (rect.height < nMinLength) {
                currentBlob->ClearContours();
            }
        }
    }

    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
        }
    }

    return 0;
}

int CImgProcBase::FilterIncludeLargeBlob(cv::Mat grayImg)
{
    //QString str;
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);
    double dLargeArea = 0;

    int n = blobs.GetNumBlobs();
    if (n > 1)
    {
        double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
        //std::sort(area.begin(), area.end(), sort_using_greater_than);
        std::stable_sort(area.begin(), area.end(), [](const double lhs, const double rhs)->bool {
            return lhs > rhs;
        });
        dLargeArea = area[0];
        blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);
    }

    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CV_RGB(255, 255, 255), 0, 0, true);
    }
    return 0;
}

int CImgProcBase::EraseLargeBlob(cv::Mat grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);
    double dLargeArea = 0;

    int n = blobs.GetNumBlobs();
    if (n > 1)
    {
        double_stl_vector area = blobs.GetSTLResult(CBlobGetArea());
        //std::sort(area.begin(), area.end(), sort_using_greater_than);
        std::stable_sort(area.begin(), area.end(), [](const double lhs, const double rhs)->bool {
            return lhs > rhs;
        });
        dLargeArea = area[0];
        blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_GREATER_OR_EQUAL, dLargeArea);
    }

    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    for (int i = 0; i < blobs.GetNumBlobs(); i++) {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CV_RGB(255, 255, 255), 0, 0, true);
    }
    return 0;
}


//세 점이 주어질 때 사이 각도 구하기
//http://stackoverflow.com/a/3487062
int CImgProcBase::GetAngleABC(Point a, Point b, Point c)
{
    Point ab = { b.x - a.x, b.y - a.y };
    Point cb = { b.x - c.x, b.y - c.y };

    float dot = (ab.x * cb.x + ab.y * cb.y); // dot product
    float cross = (ab.x * cb.y - ab.y * cb.x); // cross product

    float alpha = atan2(cross, dot);

    return (int)floor(alpha * 180.0 / CV_PI + 0.5);
}

//
// 이미지의 중심에 Edge가 위치하고 있어야한다. 즉, Threshold등을 통하여 찾고자하는 Edge를
// 중심부에 위치를 해 놓아야 정확한 subpixel edge(경사가 가파른 구간)위치를구한다.
//
double CImgProcBase::SubPixelRampEdgeImage(cv::Mat edgeImage, int nDir)
{
    vector<cv::Point2f> vecEdges;

    int widthStep = edgeImage.step;
    int cx = edgeImage.cols;
    int cy = edgeImage.rows;

    if ((nDir % 2) == 1) {
        flip(edgeImage, edgeImage, -1); // 상하, 좌우반전
    }


    uchar *data = (uchar*)edgeImage.data;
    unsigned char *fxData = NULL;
    //int start = 0;
    //int end = 0;
    //int count = 0;
    double dEdge = 0;
    //double dEdgeOtherAxis = 0;
    switch (nDir) // ("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    {
    case 0: //Left2Right,Right2Left
    case 1:
        fxData = (unsigned char *)malloc(cx);
        for (int y = 0; y < cy; y++) //vecEdges에서 상하 30%를 버린 중간값으로 또는 Peak edge값의 이미지로 Ramp edge를 구한다.
        {
            int cnt = 0;
            for (int x = 0; x < cx; x++)
            {
                int index = x + y*widthStep;
                fxData[cnt++] = data[index];
            }
            dEdge = SubPixelRampEdge(fxData, cnt);
            vecEdges.push_back(cv::Point2f(dEdge, y));
        }

        break;
    case 2: //Top2Bottom,Bottom2Top
    case 3:
        fxData = (unsigned char *)malloc(cy);
        for (int x = 0; x < cx; x++)
        {
            int cnt = 0;
            for (int y = 0; y < cy; y++)
            {
                int index = x + y*widthStep;
                fxData[cnt++] = data[index];
            }
            dEdge = SubPixelRampEdge(fxData, cnt);
            vecEdges.push_back(cv::Point2f(x, dEdge));
        }
        break;
    }
    if (fxData)
        free(fxData);

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

    dEdge = dVal;
    switch (nDir)
    {
    case 0: // Left2Right 세로선
    case 1: // Right2Left
        if ((nDir % 2) == 1)
            dEdge = cx - dEdge;
        break;
    case 2: // 가로선
    case 3:
        if ((nDir % 2) == 1)
            dEdge = cy - dEdge;
        break;
    }

    return dEdge;
}

cv::Point2f CImgProcBase::getCorner(std::vector<cv::Point2f>& corners, cv::Point2f center, int CornerType)
{
    cv::Point2f edge[4] = { { 0, 0 }, };
    std::vector<cv::Point2f> top, bot;

    for (int i = 0; i < (int)corners.size(); i++)
    {
        if (corners[i].y < center.y)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }

    if (top.size() >= 2)
    {
        edge[0] = top[0].x > top[1].x ? top[1] : top[0]; // top left
        edge[1] = top[0].x > top[1].x ? top[0] : top[1]; // top right
    }
    if (bot.size() >= 2)
    {
        edge[2] = bot[0].x > bot[1].x ? bot[1] : bot[0]; // bottom left
        edge[3] = bot[0].x > bot[1].x ? bot[0] : bot[1]; // bottom right
    }
    return edge[CornerType];
}


//
// Bresenham 알고리즘 .. 라인을 따라 점으로 처리한다.
//
void CImgProcBase::bhm_line(int x1, int y1, int x2, int y2, std::vector<cv::Point>* points)
{
    cv::Point pt1;
    int x, y, dx, dy, dx1, dy1, px, py, xe, ye, i;
    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = abs(dx);
    dy1 = abs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;

    if (dy1 <= dx1)
    {
        if (dx >= 0)
        {
            x = x1;
            y = y1;
            xe = x2;
        }
        else
        {
            x = x2;
            y = y2;
            xe = x1;
        }
        pt1.x = x;
        pt1.y = y;
        points->push_back(pt1);
        for (i = 0; x<xe; i++)
        {
            x = x + 1;
            if (px<0)
            {
                px = px + 2 * dy1;
            }
            else
            {
                if ((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    y = y + 1;
                }
                else
                {
                    y = y - 1;
                }
                px = px + 2 * (dy1 - dx1);
            }
            pt1.x = x;
            pt1.y = y;
            points->push_back(pt1);
        }
    }
    else
    {
        if (dy >= 0)
        {
            x = x1;
            y = y1;
            ye = y2;
        }
        else
        {
            x = x2;
            y = y2;
            ye = y1;
        }
        pt1.x = x;
        pt1.y = y;
        points->push_back(pt1);
        for (i = 0; y<ye; i++)
        {
            y = y + 1;
            if (py <= 0)
            {
                py = py + 2 * dx1;
            }
            else
            {
                if ((dx<0 && dy<0) || (dx>0 && dy>0))
                {
                    x = x + 1;
                }
                else
                {
                    x = x - 1;
                }
                py = py + 2 * (dx1 - dy1);
            }
            pt1.x = x;
            pt1.y = y;
            points->push_back(pt1);
        }
    }
}

double CImgProcBase::getObjectAngle(cv::Mat src)
{
    cv::Moments cm;
    cm = moments(src);
    double th = 0.5 * atan((2 * cm.m11) / (cm.m20 - cm.m02));
    return th * (180 / 3.14);
}
double CImgProcBase::GetDistance2D(cv::Point2f p1, cv::Point2f p2)
{
    return sqrt(pow((float)p1.x - p2.x, 2) + pow((float)p1.y - p2.y, 2));
}

// nAxisLength : pixel count
int CImgProcBase::FilterLargeBlob(cv::Mat grayImg, int nAxisLength)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);
    blobs.Filter(blobs, B_EXCLUDE, CBlobGetMajorAxisLength(), B_GREATER, nAxisLength); // Erase Large blob.
    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    int blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);
            currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
        }
    }
    return 0;
}

int CImgProcBase::IncludeRadiusBlob(cv::Mat grayImg, int nMinCircleRadius, int nMaxCircleRadius)
{

    CBlobResult blobs = CBlobResult(grayImg);

    // filter blobs.
    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    int   blobCount = blobs.GetNumBlobs();
    if (blobCount > 0) {
        for (int i = 0; i < blobCount; i++)
        {
            CBlob *currentBlob = blobs.GetBlob(i);

            cv::Size2d f = currentBlob->GetEllipse().size;
            int _max = MAX(f.width, f.height);
            int _min = MIN(f.width, f.height);
            if (_max > nMaxCircleRadius)
                continue;
            if (_min < nMinCircleRadius)
                continue;

            currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
        }
    }

    return 0;
}

void CImgProcBase::FilterLargeArea(cv::Mat grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);

    double dLargeArea = 0;
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        double dArea = p->Area();
        if (dLargeArea < dArea) {
            dLargeArea = dArea;
        }
    }
    if (nBlobs > 0)
      blobs.Filter(blobs, B_EXCLUDE, CBlobGetArea(), B_LESS, dLargeArea);

    // filter blobs.
    int blobCount = blobs.GetNumBlobs();
    if (blobCount == 0) {
        return;
    }
    //cvZero(grayImg);
    grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
    for (int i = 0; i < blobCount; i++)
    {
        CBlob *currentBlob = blobs.GetBlob(i);
        currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
    }
}


void CImgProcBase::FilterLargeDiameter(cv::Mat grayImg)
{
    CBlobResult blobs;
    blobs = CBlobResult(grayImg);

    cv::Rect LargeRect = cv::Rect(0, 0, 0, 0);
    int index = -1;
    int nBlobs = blobs.GetNumBlobs();
    for (int i = 0; i < nBlobs; i++) {
        CBlob *p = blobs.GetBlob(i);
        cv::Rect r = p->GetBoundingBox();
        if (max(r.height,r.width) > max(LargeRect.height,LargeRect.width))
        {
            LargeRect = r;
            index = i;
        }
    }
    if (index >= 0)
    {
#if 1
        grayImg =  cv::Mat::zeros(cv::Size(grayImg.cols,grayImg.rows), grayImg.type());
        //cvZero(grayImg);
        CBlob *currentBlob = blobs.GetBlob(index);
        currentBlob->FillBlob(grayImg, CVX_WHITE, 0, 0, true);
#else
        cvSet(grayImg, cv::Scalar(255));
        for (int i = 0; i < nBlobs; i++) {
            if (i != index) {
                CBlob *p = blobs.GetBlob(i);
                p->FillBlob(grayImg, CVX_BLICK);
            }
        }
#endif
    }
}


//Calculate Center of Moment
Point2f CImgProcBase::CenterOfMoment(vector<cv::Point> seq)
{
    double M;
    int x_order, y_order;
    double cX, cY, m00 = 1.0;
    Point2f point;

    cv::Moments moments;

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    // (4) Calculate Center of Moment using cvMoment
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    moments = cv::moments(seq);

    for (y_order = 0; y_order <= 3; y_order++)
    {
        for (x_order = 0; x_order <= 3; x_order++)
        {
            if (x_order + y_order > 3)
                continue;
            //M = cvGetSpatialMoment(&moments, x_order, y_order);
            int order = x_order + y_order;
            M = (&(m00))[order + (order >> 1) + (order > 2) * 2 + y_order];
            if (x_order == 0 && y_order == 0)
                m00 = M;
            else if (x_order == 1 && y_order == 0)
                cX = M;
            else if (x_order == 0 && y_order == 1)
                cY = M;
        }
    }

    cX /= m00;
    cY /= m00;

    point.x = (float)cX;
    point.y = (float)cY;
    return point;
}

//
// 라인이 직접 만나야지 true를 리턴(확장라인이 만날때는 false)
//  ref : http://www.gisdeveloper.co.kr/?p=89
//
bool CImgProcBase::checkCross(const cv::Point& AP1, const cv::Point& AP2, const cv::Point& BP1, const cv::Point& BP2, cv::Point* IP)
{
    double t;
    double s;
    double under = (BP2.y - BP1.y)*(AP2.x - AP1.x) - (BP2.x - BP1.x)*(AP2.y - AP1.y);
    if (under == 0) return false;

    double _t = (BP2.x - BP1.x)*(AP1.y - BP1.y) - (BP2.y - BP1.y)*(AP1.x - BP1.x);
    double _s = (AP2.x - AP1.x)*(AP1.y - BP1.y) - (AP2.y - AP1.y)*(AP1.x - BP1.x);

    t = _t / under;
    s = _s / under;

    if (t<0.0 || t>1.0 || s<0.0 || s>1.0) return false;
    if (_t == 0 && _s == 0) return false;

    IP->x = (long)(AP1.x + t * (double)(AP2.x - AP1.x));
    IP->y = (long)(AP1.y + t * (double)(AP2.y - AP1.y));
    return true;

}


//
// 확장 라인까지 계산해서 만나는 지점이 있는지 계산해줌
//
// intersection() 와 getIntersectionPoint()는 동일한 결과를 가짐.
//

// Finds the intersection of two lines, or returns false.
// The lines are defined by (o1, p1) and (o2, p2).
//bool CImgProcBase::intersection(Point2f o1, Point2f p1, Point2f o2, Point2f p2, Point2f &r)
//{
//	Point2f x = o2 - o1;
//	Point2f d1 = p1 - o1;
//	Point2f d2 = p2 - o2;
//
//	float cross = d1.x*d2.y - d1.y*d2.x;
//	if (abs(cross) < /*EPS*/1e-8)
//		return false;
//
//	double t1 = (x.x * d2.y - x.y * d2.x) / cross;
//	r = o1 + d1 * t1;
//	return true;
//}

double CImgProcBase::isCross(Point v1, Point v2)
{
    return v1.x*v2.y - v1.y*v2.x;
}
bool CImgProcBase::getIntersectionPoint(Point a1, Point a2, Point b1, Point b2, Point & intPnt)
{
    Point p = a1;
    Point q = b1;
    Point r(a2 - a1);
    Point s(b2 - b1);

    if (isCross(r, s) == 0) { return false; }

    double t = isCross(q - p, s) / isCross(r, s);

    intPnt = p + t*r;
    return true;
}

//
// 한점과 직선이 만나는 가장 가까운점
// 리턴값 : 길이
// nearX, nearY : 만나는점
// 수선의 조건이 충족되면 라인상의 가장 가까운점을 찾게되고, 아니면 시작점 또는 끝점이 된다.
//
double CImgProcBase::Dist2LineSegment(double px, double py, double X1, double Y1, double X2, double Y2, double &nearX, double &nearY)
{
    double dx = X2 - X1;
    double dy = Y2 - Y1;
    //	, t;
    if (dx == 0 && dy == 0)
    {
        dx = px - X1;
        dy = py - Y1;

        nearX = X1;
        nearY = Y1;

        return sqrt((double)(dx * dx + dy * dy));
    }

    double t = (double)((px - X1) * dx + (py - Y1) * dy) / (double)(dx * dx + dy * dy);

    if (t < 0)
    {
        dx = px - X1;
        dy = py - Y1;
        nearX = X1;
        nearY = Y1;
    }
    else if (t > 1)
    {
        dx = px - X2;
        dy = py - Y2;
        nearX = X2;
        nearY = Y2;
    }
    else
    {
        nearX = X1 + t * dx;
        nearY = Y1 + t * dy;
        dx = px - nearX;
        dy = py - nearY;
    }

    return sqrt((double)(dx * dx + dy * dy));
}


//
// Hessian matrix 알고리즘을 이용한 Subpixel Edge를 구함
// 여러 edge contour를 구한다.
//
double CImgProcBase::SubPixelHessianEdge(cv::Mat src, int nDir)
{
    vector<Contour> contours;

    double alpha = 1.0;
    int low = 10;
    int high = 10;
    int mode = RETR_LIST; //retrieves all of the contours without establishing any hierarchical relationships.

    //Mat msrc = src;// imread(imagePath, IMREAD_GRAYSCALE);
    //vector<Contour> contours;
    vector<Vec4i> hierarchy;
    //int64 t0 = getCPUTickCount();

    // alpha - GaussianBlur = sigma이며 흐려지는 정도를 조절할 수 있다.
    // hierarchy, mode  - have the same meanings as in cv::findContours
    EdgesSubPix(src, alpha, low, high, contours, hierarchy, mode);



    // 윤곽선이 1개 이상 형성될것이므로 ROI Direction에 따라 첫번째 contour를 선택한다
    typedef struct {
        cv::Point2f sum;
        cv::Point2f avg;
        int count;
        int seq;
    } EDGE;
    vector<EDGE> points;

    //points.resize(contours.size());
    for (size_t i = 0; i < contours.size(); ++i)
    {
        EDGE edge;
        edge.seq = i;
        for (size_t j = 0; j < contours[i].points.size(); ++j)
        {
            cv::Point2f pt = contours[i].points[j];
            edge.sum.x += pt.x;
            edge.sum.y += pt.y;
        }
        edge.count = contours[i].points.size();
        points.push_back(edge);
    }
    for (size_t i = 0; i < points.size(); ++i)
    {
        points[i].avg = points[i].sum / points[i].count;
    }

    //int nDir = 0;
    int nDetectMethod = 0; // Average
//    CParam *pParam = pData->getParam(_T("Detect method")); // _T("Average,First")
//    if (pParam)
//        nDetectMethod = (int)_ttoi(pParam->Value.c_str());
//    pParam = pData->getParam(_T("Direction"));
//    if (pParam)
//        nDir = (int)_ttoi(pParam->Value.c_str());

    switch (nDir) // _T("Left2Right,Right2Left,Top2Bottom,Bottom2Top")),
    {
    case 0:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.x < rhs.avg.x) // assending
                return true;
            return false;
        });
        break;
    case 1:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.x > rhs.avg.x) // descending
                return true;
            return false;
        });
        break;
    case 2:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.y < rhs.avg.y) // assending
                return true;
            return false;
        });
        break;
    case 3:
        std::stable_sort(points.begin(), points.end(), [](const EDGE lhs, const EDGE rhs)->bool {
            if (lhs.avg.y > rhs.avg.y) // descending
                return true;
            return false;
        });
        break;

    }

    int select = -1;
    for (size_t i = 0; i < points.size(); ++i)
    {
        int seq = points[i].seq;
        if (contours[seq].points.size() > (src.cols / 4)) // ROI영역의 폭의 1/4만큼의 edge라인이 형성된것중 x or y가 적은(소팅결과) edge
        {
            select = seq;
            break;
        }
    }
    //  first contour selected.

    if (select < 0)
        return -1;

    // 선택된 Contour의 각 point를 가져와서 튀는 값은 버리고 중간값들중 첫번째 또는 평균 위치를 구한다.
    // Sorting해서 상위 40%, 하위 40%를 버리고 중간지점의 point만 취한다.
    Contour &contour = contours[select];
    switch (nDir)
    {
    case 0: // vertical line
    case 1:
        std::stable_sort(contour.points.begin(), contour.points.end(), [](const Point2f lhs, const Point2f rhs)->bool {
            if (lhs.x < rhs.x) // assending
                return true;
            return false;
        });
        break;
    case 2: // horizontal line
    case 3:
        std::stable_sort(contour.points.begin(), contour.points.end(), [](const Point2f lhs, const Point2f rhs)->bool {
            if (lhs.y < rhs.y) // assending
                return true;
            return false;
        });
        break;
    }

    double dVal = 0;
    cv::Point2d pt = { 0, 0 };
    if (nDetectMethod == 0) // Average
    {
        // 소팅을 한 결과 테이블에서 상하 40%를 버리고 중간 20%의 중간값을 구한다.
        float sz = contour.points.size();
        int first = sz * 0.4;
        int last = sz - (sz * 0.4);
        for (int i = first; i < last; i++)
        {
            pt.x += contour.points[i].x;
            pt.y += contour.points[i].y;
        }

        sz = last - first;
        if (sz > 0) {
            pt.x /= sz;
            pt.y /= sz;
        }
    }
    else {	// First
        //제일처음 만나는 edge를 구한다
        float sz = contour.points.size();
        if (sz > 0) {
            if (nDir == 0)  { // Left2Right 세로선
                pt.x += contour.points[0].x;
                pt.y += contour.points[0].y;
            }
            else if (nDir == 1)  { // Right2Left 세로선
                pt.x += contour.points[sz - 1].x;
                pt.y += contour.points[sz - 1].y;
            }
            else if (nDir == 2)  { // Top2Bottom 가로선
                pt.x += contour.points[0].x;
                pt.y += contour.points[0].y;
            }
            else {				//Bottom2Top
                pt.x += contour.points[sz - 1].x;
                pt.y += contour.points[sz - 1].y;
            }
        }
    }

    if (nDir == 0 || nDir == 1) // x 위치
        dVal = pt.x;
    else
        dVal = pt.y; // y 위치

    return dVal;
}


//
// 경사진면의 edge를 구한다
// nCnt - threshold경계면부터 검색할 범위(0 ~ 512)
//
// nDir = 0 : Left2Right, Top2Bottom
//        1 : Right2Left, Bottom2Top
double CImgProcBase::SubPixelRampEdge(unsigned char *pixelData, int pCnt)
{
    double dLoc = 0.0;
    int i;
    int ffx[512+1];
    int iSumfx;

    if (pCnt > 512)
        return 0;
    memset(ffx, 0, sizeof(ffx));
    iSumfx = 0;
    for (i = 0; i < pCnt - 1; i++) {
        ffx[i + 1] = abs(pixelData[i + 1] - pixelData[i]);
        iSumfx += ffx[i + 1];
    }

    for (i = 0; i < pCnt - 1; i++) {
        if (iSumfx > 0)
            dLoc += (double(i + 1.0) * double(ffx[i + 1]) / double(iSumfx));
    }

    return dLoc;
}

/**
* Helper function to find a cosine of angle between vectors
* from pt0->pt1 and pt0->pt2
*/
double CImgProcBase::calcAngle(cv::Point pt1, cv::Point pt2, cv::Point pt0)
{
    double dx1 = pt1.x - pt0.x;
    double dy1 = pt1.y - pt0.y;
    double dx2 = pt2.x - pt0.x;
    double dy2 = pt2.y - pt0.y;
    return (dx1*dx2 + dy1*dy2) / sqrt((dx1*dx1 + dy1*dy1)*(dx2*dx2 + dy2*dy2) + 1e-10);
}

//
// OTSU Threshold를 이용하여 히스토그램 Threshold Value를 구한다.
//
int CImgProcBase::find_thresholdOTSU(cv::Mat image)
{
    //compute histogram first
    cv::Mat imageh; //image edited to grayscale for histogram purpose
    //imageh=image; //to delete and uncomment below;
    //cv::cvtColor(imagecolor, imageh, cv::COLOR_BGR2GRAY);
    //imageh = cvarrToMat(image);

    int histSize[1] = {256}; // number of bins
    float hranges[2] = {0.0, 256.0}; // min andax pixel value
    const float* ranges[1] = {hranges};
    int channels[1] = {0}; // only 1 channel used

    cv::MatND hist;
    // Compute histogram
    calcHist(&image, 1, channels, cv::Mat(), hist, 1, histSize, ranges);

    int totalNumberOfPixels= imageh.total();
    //cout<<"total number of Pixels is " <<totalNumberOfPixels<< endl;


    float sum = 0;
    for (int t=0 ; t<256 ; t++)
    {
        sum += t * hist.at<float>(t);
    }
    //cout<<"sum is "<<sum<<endl;

    float sumB = 0; //sum of background
    int wB = 0; // weight of background
    int wF = 0; //weight of foreground

    float varMax = 0;
    int threshold = 0;

    //run an iteration to find the maximum value of the between class variance(as between class variance shld be maximise)
    for (int t=0 ; t<256 ; t++)
    {
        wB += hist.at<float>(t);               // Weight Background
        if (wB == 0) continue;

        wF = totalNumberOfPixels - wB;                 // Weight Foreground
        if (wF == 0) break;

        sumB += (float) (t * hist.at<float>(t));

        float mB = sumB / wB;            // Mean Background
        float mF = (sum - sumB) / wF;    // Mean Foreground

        // Calculate Between Class Variance
        float varBetween = (float)wB * (float)wF * (mB - mF) * (mB - mF);

        // Check if new maximum found
        if (varBetween > varMax) {
            varMax = varBetween;
            threshold = t;
        }
    }

    return threshold; //threshold value is
}


//cv::Point2d srcTri[4];
//cv::Point2d dstTri[4];
//srcTri[0] = Point2f(0, 0);
//srcTri[1] = Point2f(100, 0);
//srcTri[2] = Point2f(100, 100);
//srcTri[3] = Point2f(0, 100);
//dstTri[0] = Point2f(0, 5);
//dstTri[1] = Point2f(100, 4);
//dstTri[2] = Point2f(100, 100);
//dstTri[3] = Point2f(0, 100);
// //http://lueseypid.tistory.com/111

void CImgProcBase::AffineTransform(std::vector<Point2f> &vec, cv::Point2f srcTri[], cv::Point2f dstTri[])
{
    Mat m(2, 3, CV_32F);
    m = getAffineTransform(srcTri, dstTri);
    transform(vec, vec, m);
}

void CImgProcBase::WarpPerspectiveImage(cv::Mat img, cv::Point2f srcTri[], cv::Point2f dstTri[])
{
    Mat warp_mat = getPerspectiveTransform(srcTri, dstTri);
    warpPerspective(img, img, warp_mat, img.size());
}

void CImgProcBase::WarpAffineImage(cv::Mat img, cv::Point2f srcTri[], cv::Point2f dstTri[])
{
    Mat warp_mat = getAffineTransform(srcTri, dstTri); // 트랜스폼 매트릭스 생성 from SrcTri,dstTri
    warpAffine(img, img, warp_mat, img.size());
}

void CImgProcBase::RotateImage(cv::Mat img, double angle, cv::Point2d center, double scale)
{
    Mat rot_mat = getRotationMatrix2D( center, angle, scale );
    warpAffine( img, img, rot_mat, img.size() );
}

// 중심점과 회전각도를 이용하여 이미지 변환 : 패턴매칭에서 Align에 이용
void CImgProcBase::RotateImage(cv::Mat img, double angle, cv::Point2d center)
{
    double scale = 1.0;
    Mat rot_mat = getRotationMatrix2D( center, angle, scale );
    warpAffine( img, img, rot_mat, img.size() );
}
void CImgProcBase::RotateImage(cv::Mat img, double angle)
{
    Point center = Point( img.cols/2, img.rows/2 );
    double scale = 1.0;
    Mat rot_mat = getRotationMatrix2D( center, angle, scale );
    warpAffine( img, img, rot_mat, img.size() );
}

#if 1
//
// AutoFocus를 위한 함수들
//

bool CImgProcBase::checkForBurryImage(cv::Mat matImage)
{
    int kBlurThreshhold = -6118750;
    cv::Mat finalImage;

    cv::Mat matImageGrey;
    cv::cvtColor(matImage, matImageGrey, cv::COLOR_BGRA2GRAY);
    matImage.release();

    cv::Mat newEX;
    const int MEDIAN_BLUR_FILTER_SIZE = 15; // odd number
    cv::medianBlur(matImageGrey, newEX, MEDIAN_BLUR_FILTER_SIZE);
    matImageGrey.release();

    cv::Mat laplacianImage;
    cv::Laplacian(newEX, laplacianImage, CV_8U); // CV_8U
    newEX.release();

    cv::Mat laplacianImage8bit;
    laplacianImage.convertTo(laplacianImage8bit, CV_8UC1);
    laplacianImage.release();
    cv::cvtColor(laplacianImage8bit, finalImage, cv::COLOR_GRAY2BGRA);
    laplacianImage8bit.release();

    int rows = finalImage.rows;
    int cols = finalImage.cols;
    char *pixels = reinterpret_cast<char *>(finalImage.data);
    int maxLap = -16777216;
    for (int i = 0; i < (rows*cols); i++) {
        if (pixels[i] > maxLap)
            maxLap = pixels[i];
    }

    //int soglia = -6118750;

    pixels = NULL;
    finalImage.release();

    bool isBlur = (maxLap < kBlurThreshhold) ? true : false;
    return isBlur;
}

double CImgProcBase::rateFrame(Mat frame)
{
    unsigned long int sum = 0;
    unsigned long int size = frame.cols * frame.rows;
    Mat edges;
    if (frame.channels() > 1)
        cvtColor(frame, edges, cv::COLOR_BGR2GRAY);
    else edges = frame;
    GaussianBlur(edges, edges, Size(7, 7), 1.5, 1.5);
    Canny(edges, edges, 0, 30, 3);

    MatIterator_<uchar> it, end;
    for (it = edges.begin<uchar>(), end = edges.end<uchar>(); it != end; ++it)
    {
        sum += *it != 0;
    }

    return (double)sum / (double)size;
}

double CImgProcBase::correctFocus(FocusState & state, double rate)
{
    const double epsylon = 0.001; // compression, noice, etc. // 0.0005

    //state.lastDirectionChange++;
    double rateDelta = rate - state.rate;

    if (rate >= state.rateMax + epsylon) // Update Max
    {
        state.rateMax = rate;
        //state.lastDirectionChange = 0; // My local minimum is now on the other direction, that's why:
    }

    if (rate < epsylon)
    { // It's hard to say anything
        state.step = 1;// state.step * 0.75;
    }
    else if (rateDelta < -epsylon)
    { // Wrong direction ?
        state.direction *= -1;
        state.step = state.step * 0.75;
        //state.lastDirectionChange = 0;
    }
    else if (rate + epsylon < state.rateMax)
    {
        state.step = state.step * 0.75;
        //state.lastDirectionChange = 0; // Like reset.
    }

    // Update state.
    state.rate = rate;
    return state.step;
}
#endif

//
// Threshold 이미지에서
// Edge의 경계면을 추출하고 경계면으로부터 Subpixel edge를 추출한다.
// * Hessian Matrix 를 이용한것보다 좀더 경사진면에서의 edge를 구할수 있다.
//
// nDir = Left2Right,Right2Left,Top2Bottom,Bottom2Top
double CImgProcBase::ROIPixEdge(cv::Mat croppedImage, int nDir, double dRejectLow, double dRejectHigh)
{
    //QString str;
    cv::Mat grayImg;

    cv::Rect r = cv::Rect(0,0,croppedImage.cols, croppedImage.rows);
    int x=0, y=0, width=r.width, height=r.height;

    grayImg = cv::Mat(cv::Size(width, height), croppedImage.type());
    CopyImageROI(croppedImage, grayImg, cv::Rect(x, y, width, height));

    double dEdge = 0;
    vector<cv::Point2f> vecEdges;

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

    int nColor;
    if (nDir == 0 || nDir == 2)
        nColor = imageData[0];
    else
        nColor = imageData[width-1];

    uchar* data = (uchar*)grayImg.data;
    int widthStep = grayImg.step;
    int cx = grayImg.cols;
    int cy = grayImg.rows;

    // Threshold한 결과 이미지의 경계면을 구한다. Left2Right,Right2Left,Top2Bottom,Bottom2Top에 따라서
    switch (nDir)
    {
    case 0: //Left2Right
        for (int y = 0; y < cy; y++)
        {
            for (int x = 0; x < cx; x++)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 1: //Right2Left
        for (int y = 0; y < cy; y++)
        {
            for (int x = cx - 1; x >= 0; x--)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 2: //Top2Bottom
        for (int x = 0; x < cx; x++)
        {
            for (int y = 0; y < cy; y++)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
                    vecEdges.push_back(cv::Point2f(x, y));
                    break;
                }
            }
        }
        break;
    case 3: //Bottom2Top
        for (int x = 0; x < cx; x++)
        {
            for (int y = cy - 1; y >= 0; y--)
            {
                int index = x + y*widthStep;
                if (data[index] != nColor) {
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
    int first = sz * dRejectLow;
    int last = sz - (sz * dRejectHigh);
    if (first < 0)
        first = 0;
    if (last < 0)
        last = 0;

    // 소팅을 한 결과 테이블에서 상하 45%를 버리고 중간 10%의 중간값을 구한다.
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

    // 여기까지 Threshold를 이용한 edge를 구하였다.

    switch (nDir) {
        case 0: // Left2Right
            dEdge = dVal + r.x;
            break;
        case 1: // Right2Left
            dEdge = dVal + r.x;
            break;
        case 2: // Top2Bottom
            dEdge = dVal + r.y;
            break;
        case 3: // Bottom2Top
            dEdge = dVal + r.y;
            break;
    }
    return dEdge;
}

}  // namespace

