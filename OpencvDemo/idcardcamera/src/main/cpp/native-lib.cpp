#include "com_wildma_idcardcamera_cropper_CropOverlayView.h"
#include <android/bitmap.h>
#include <opencv2/opencv.hpp>


using namespace cv;
using namespace std;

int halfx, halfy;

float countwhitepoint(Mat image, Point2f corners, Point2f corners1);
Mat drawingline(Mat image, Point2f corners, Point2f corners1);
vector<Point2f> contourselect(Mat image, vector<Point2f>corners, vector<Point2f>corners2);
Point2f cornersolve(Mat image, Point2f corner1, Point2f corner2, vector<Point2f>corners);//计算corners1所在竖直线与corners2所在水平线之间的交点;
Point2f selonepoint(vector<Point2f> point, Point2f a, Mat image);
Point2f selonepoint2(vector<Point2f> point, Point2f a, Point2f b, Mat image);
vector<int> contourjudge(Mat image, Mat image1, vector<Point2f>corners);
Point2f selonepointk(vector<Point2f> point, Point2f a);
vector<int> getcorner(Mat image);

extern "C" JNIEXPORT  jobjectArray
JNICALL Java_com_wildma_idcardcamera_cropper_CropOverlayView_createArray2D
        (JNIEnv *env, jobject obj, jobject bitmap) {
    AndroidBitmapInfo info;
    void *pixels;
    jobjectArray jobjArr;
    jclass intArr = env->FindClass("[I");
    jobjArr = env->NewObjectArray(8 * 2, intArr, nullptr);
    jint pointarry[8];



    CV_Assert(AndroidBitmap_getInfo(env, bitmap, &info) >= 0);
    CV_Assert(info.format == ANDROID_BITMAP_FORMAT_RGBA_8888 ||
              info.format == ANDROID_BITMAP_FORMAT_RGB_565);
    CV_Assert(AndroidBitmap_lockPixels(env, bitmap, &pixels) >= 0);
    CV_Assert(pixels);
    if (info.format == ANDROID_BITMAP_FORMAT_RGBA_8888) {
        Mat temp(info.height, info.width, CV_8UC4, pixels);
        Mat runpic=temp.clone();
        vector<int> corners=getcorner(runpic);
        int n=0;
        for (jint i = 0; i <4; i++){
            jintArray colArr = env->NewIntArray(2);
            for (jint j = 0; j < 2; j++)
            {
                pointarry[j] = corners[n];
                n++;
            }
            env->SetIntArrayRegion(colArr, 0, 2, pointarry);
            env->SetObjectArrayElement(jobjArr, i, colArr);
            env->DeleteLocalRef(colArr);
        }

    } else {
        Mat temp(info.height, info.width, CV_8UC2, pixels);
        Mat runpic=temp.clone();
        vector<int> corners=getcorner(runpic);
        int n=0;
        for (jint i = 0; i < 4; i++){
            jintArray colArr = env->NewIntArray(2);
            for (jint j = 0; j < 2; j++)
            {
                pointarry[j] = corners[n];
                n++;
            }
            env->SetIntArrayRegion(colArr, 0, 2, pointarry);
            env->SetObjectArrayElement(jobjArr, i, colArr);
            env->DeleteLocalRef(colArr);
        }

    }

    AndroidBitmap_unlockPixels(env, bitmap);
    return jobjArr;
}



vector<int> getcorner(Mat image)
{
    Mat  Imggray, edge, mask;
    cvtColor(image, Imggray, COLOR_RGB2GRAY);
    Canny(Imggray, edge, 15, 100);



    Mat out;
    //获取自定义核
    Mat element = getStructuringElement(MORPH_RECT, Size(3, 3)); //第一个参数MORPH_RECT表示矩形的卷积核，当然还可以选择椭圆形的、交叉型的
    //膨胀操作
    dilate(edge, out, element);
    vector<vector<Point>>contours;
    findContours(out, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    vector<vector<Point>>poly(contours.size());

    for (int i = 0; i < contours.size(); i++)
    {
        approxPolyDP(Mat(contours[i]), poly[i], 5, true);//对轮廓进行多边形拟合
    }

    Mat drawing = Mat::zeros(edge.size(), CV_8UC1);
    for (int i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(255, 255, 255);
        drawContours(drawing, poly, i, color, 1, 8, vector<Vec4i>(), 0, Point());
        drawContours(image, poly, i, color, 1, 8, vector<Vec4i>(), 0, Point());
    }


    Mat mask1, edge3 = Mat::zeros(edge.size(), CV_8UC1);
    Mat element2 = getStructuringElement(MORPH_RECT, Size(10, 10));
    dilate(drawing, mask1, element2);
    Mat mask2 = 255 - mask1;//反色



    Mat gsfuzzy;
    Mat image1 = image;
    GaussianBlur(image, gsfuzzy, Size(25, 25), 0, 0);
    gsfuzzy.copyTo(image1, mask2);
    Mat Imggray1, edge1, expan, expan2;
    cvtColor(image1, Imggray1, COLOR_RGB2GRAY);
    Canny(Imggray1, edge1, 15, 100);
    Mat element3 = getStructuringElement(MORPH_RECT, Size(5, 5));
    dilate(edge1, expan, element3);








    vector<Point2f>corners;//Point2f类型的向量：存储每个角点的坐标
    //输入图，向量，最大角点数量，角点的最小特征值，角点间最小距离，掩码（Mat（）表示掩码为空），blocksize，是否使用Harris角点检测，权重系数
    goodFeaturesToTrack(drawing, corners, 100, 0.01, 15, Mat(), 3, false, 0.04);
    cout << "角点数量" << corners.size() << endl;


    //画圆标注角点
    for (int i = 0; i < corners.size(); i++)
    {
        circle(image, corners[i], 2, Scalar(0, 255, 0), 1, 2);
        char ch[100];
        sprintf(ch, "%d", i);
        string str;
        str = ch;
        putText(image, str, corners[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 23, 0), 1, 2);
    }
    Mat drawing2;

    Mat element4 = getStructuringElement(MORPH_RECT, Size(7, 7));
    dilate(expan, expan2, element3);
    dilate(drawing, drawing2, element3);
    vector<int> corners2 = contourjudge(expan, image, corners);

    return corners2;

}




Point2f selonepointk(vector<Point2f> point, Point2f a)//从point中选择一点，使得这一点和a构成的直线最接近水平直线
{
    float dk, dkmin = 100; Point2f b;
    for (int i = 0; i < point.size(); i++)
    {
        float k1 = float((point[i].y - a.y) / (point[i].x - a.x));
        dk = fabsf(k1);
        if (dk <dkmin)
        {
            b = point[i];
            dkmin = dk;
        }
    }
    return b;
}

vector<Point2f> selonepointk2(vector<Point2f> pointa, vector<Point2f> pointb)//从point中选择一点，使得这一点和a构成的直线最接近水平直线
{
    float dk, dkmin = 100; Point2f a,b;int c= pointb.size(), d = pointa.size();
    for (int i = 0; i < pointa.size(); i++)
    {
        for (int j = 0; j < pointb.size();j++)
        {
            float k1 = float((pointa[i].y - pointb[j].y) / (pointa[i].x - pointb[j].x+1e-6));
            dk = fabsf(k1);
            if (dk <dkmin)
            {
                a = pointa[i];
                b = pointb[j];
                dkmin = dk;
            }

        }

    }
    vector<Point2f> pointreturn;
    pointreturn.push_back(a);
    pointreturn.push_back(b);
    return pointreturn;
}

vector<Point2f> selpointwhitenum(vector<Point2f> point, Point2f a,Mat image)//point中的点和点a构成的线段中白色像素点比例最高的两个点
{
    float nummax=0,num;
    int maxn,maxn2;
    vector<Point2f> pointreturn;
    Point2f r;
    for (int i = 0; i <point.size(); i++)
    {
        num = countwhitepoint(image, point[i], a);
        if (num > nummax)
        {
            r = point[i];
            nummax = num;
            maxn = i;
        }

    }
    pointreturn.push_back(r);
    nummax = 0;
    for (int i = 0; i <point.size(); i++)
    {
        if (i != maxn)
        {
            num = countwhitepoint(image, point[i], a);
            if (num > nummax)
            {
                r = point[i];
                nummax = num;
                maxn2 = i;

            }
        }

    }
    if(point.size()>=3)
    {
        nummax = 0;
        for (int i = 0; i <point.size(); i++)
        {
            if (i != maxn&&i != maxn2)
            {
                num = countwhitepoint(image, point[i], a);
                if (num > nummax)
                {
                    r = point[i];
                    nummax = num;
                    maxn2 = i;

                }
            }

        }

    }
    pointreturn.push_back(r);
    return pointreturn;
}
Point2f selonepoint(vector<Point2f> point, Point2f a, Mat image)//判断point中的点和点a构成的线段中白色像素点所占比重，选出point中所占比重最高的点
{
    float whitepoint, whitepointmax=0;
    int n = point.size();
    for (int i = 0; i < n; i++)
    {

        whitepoint = countwhitepoint(image,point[i], a);
        if (whitepoint > whitepointmax)
        {
            whitepointmax = whitepoint;
            point[0] = point[i];
        }

    }
    return point[0];

}

Point2f selonepoint2(vector<Point2f> point, Point2f a,Point2f b, Mat image)////判断point中的点到点a，b中白色像素点所占比重，选出point中所占比重最高的点
{
    float whitepointx, whitepointy, whitepointsum,whitepointsummax = 0;
    int n = point.size();
    for (int i = 0; i < n; i++)
    {

        whitepointx = countwhitepoint(image, point[i], a);
        whitepointy= countwhitepoint(image, point[i], b);
        whitepointsum = whitepointx + whitepointy;
        if (whitepointsum> whitepointsummax)
        {
            whitepointsummax = whitepointsum;
            point[0] = point[i];
        }

    }
    return point[0];

}


vector<Point2f> contourselect(Mat image, vector<Point2f>corners, vector<Point2f>corners2) //根据角点在图像中的位置进一步删选角点，最后在图像的四个位置留下四个角点
{

    int y = image.cols;
    int x = image.rows;
    int flag = 0;
    float whitepointx, whitepointy, whitepoint;
    float whitepointmax = 0;

    float sumwhitepoint=0;

    vector<Point2f> upleft;
    vector<Point2f> upright;
    vector<Point2f> downleft;
    vector<Point2f> downright;
    vector<Point2f> cornersback;
    for (int i = 0; i < corners.size(); i++)
    {
        if (corners[i].x < 0.5*x && corners[i].y < 0.5*y)
        {
            upleft.push_back(corners[i]);
        }
        if (corners[i].x > 0.5*x && corners[i].y < 0.5*y)
        {
            upright.push_back(corners[i]);
        }
        if (corners[i].x < 0.5*x && corners[i].y >0.5*y)
        {
            downleft.push_back(corners[i]);
        }
        if (corners[i].x > 0.5*x && corners[i].y > 0.5*y)
        {
            downright.push_back(corners[i]);
        }

    }
    int ul = upleft.size();
    int ur = upright.size();
    int dl = downleft.size();
    int dr = downright.size();

    int onept = 0, twopt = 0, zeropt = 0;
    if (ul == 0) { zeropt++; }if (ul == 1) { onept++; }if (ul >=2) { twopt++; }
    if (ur == 0) { zeropt++; }if (ur == 1) { onept++; }if (ur >= 2) { twopt++; }
    if (dl == 0) { zeropt++; }if (dl == 1) { onept++; }if (dl >= 2) { twopt++; }
    if (dr == 0) { zeropt++; }if (dr == 1) { onept++; }if (dr >= 2) { twopt++; }

    if (onept==4)
    {

        flag = 1;

    }
    if (onept==3&&twopt==1)
    {
        flag = 1;
        if(ul>=2)
        {
            upleft[0]=selonepointk(upleft, upright[0]);
        }

        if (ur >= 2)
        {
            upright[0] = selonepointk(upright, upleft[0]);
        }

        if (dr >= 2)
        {
            downright[0] = selonepointk(downright, downleft[0]);
        }

        if (dl >= 2)
        {
            downleft[0] = selonepointk(downleft, downright[0]);
        }

    }
    if (onept == 2 && twopt == 2)
    {
        flag = 1;
        if(ul>=2&&ur>=2)
        {
            vector<Point2f> pointa, pointb, pointc;
            pointa=selpointwhitenum( upleft,downleft[0], image);
            pointb = selpointwhitenum(upright, downright[0], image);
            pointc=selonepointk2(pointa, pointb);
            upleft[0] = pointc[0];
            upright[0] = pointc[1];
        }

        if (ul >= 2 && dl >= 2)
        {
            upleft[0] = selonepoint(upleft, upright[0], image);

            downleft[0] = selonepoint(downleft, downright[0], image);
        }

        if (ur >= 2 && dr >= 2)
        {
            upright[0] = selonepoint(upright, upleft[0], image);

            downright[0] = selonepoint(downright, downleft[0], image);
        }

        if (dl >= 2 && dr >= 2)
        {
            downleft[0] = selonepoint(downleft, upleft[0], image);

            downright[0] = selonepoint(downright, upright[0], image);
        }
        if(ul>=2&&dr>=2)
        {
            upleft[0] = selonepoint2(upleft, upright[0], downleft[0], image);
            downright[0]= selonepoint2(downright, downleft[0], upright[0], image);

        }
        if (ur >= 2 && dl >= 2)
        {
            upright[0] = selonepoint2(upright, upleft[0], downright[0], image);
            downleft[0] = selonepoint2(downleft, upleft[0], downright[0], image);
        }



    }

    if (onept == 1 && twopt == 3)
    {
        flag = 1;
        if (ul == 1)
        {
            upright[0]= selonepoint(upright, upleft[0], image);

            downleft[0]= selonepoint(downleft, upleft[0], image);

            downright[0]= selonepoint2(downright, upright[0], downleft[0], image);
        }

        if (ur == 1)
        {

            upleft[0] = selonepoint(upleft, upright[0], image);
            downright[0] = selonepoint(downright, upright[0], image);
            downleft[0] = selonepoint2(downleft, upleft[0],downright[0], image);
        }
        if (dl == 1)
        {
            upleft[0] = selonepoint(upleft, downleft[0], image);

            downright[0] = selonepoint(downright, downleft[0], image);

            upright[0] = selonepoint2(upright, downright[0], upleft[0], image);
        }
        if (dr == 1)
        {
            downleft[0] = selonepoint(downleft, downright[0], image);

            upright[0] = selonepoint(upright, downright[0], image);

            upleft[0] = selonepoint2(upleft, downleft[0], upright[0], image);
        }


    }
    if(onept==2&&twopt==1&&zeropt==1)
    {
        flag = 1;
        if (ul == 1 && ur ==1&&dr==0)
        {
            downleft[0]= selonepoint(upleft, downleft[0], image);
            downright.push_back ( cornersolve(image, upright[0], downleft[0], corners2));
        }
        if (ul == 1 && ur == 1 && dl == 0)
        {
            downright[0] = selonepoint(downright, upleft[0], image);
            downleft.push_back ( cornersolve(image, upleft[0], downright[0], corners2));
        }
        if (ul == 1 && dl == 1 && ur == 0)
        {
            downright[0] = selonepoint(downright, downleft[0], image);
            upright.push_back( cornersolve(image, downright[0], upleft[0], corners2));
        }
        if (ul == 1 && dl == 1 && dr == 0)
        {
            upright[0] = selonepoint(upright, upleft[0], image);
            downright.push_back (cornersolve(image, upright[0],downleft[0], corners2));

        }

        if (ul == 1 && dr == 1 && ur == 0)
        {
            downleft[0] = selonepoint2(downleft, upleft[0],downright[0], image);
            upright.push_back( cornersolve(image, downright[0], upleft[0], corners2));

        }
        if (ul == 1 && dr == 1 && dl == 0)
        {
            upright[0] = selonepoint2(upright, upleft[0],downright[0], image);
            downleft.push_back(cornersolve(image, upleft[0], downright[0], corners2));

        }
        if (ur == 1 && dl == 1 && ul == 0)
        {
            downright[0] = selonepoint2(downright, upright[0], downleft[0], image);
            upleft.push_back(cornersolve(image, downleft[0], upright[0], corners2));

        }
        if (ur == 1 && dl == 1 && dr == 0)
        {
            upleft[0] = selonepoint2(upleft, upright[0], downleft[0], image);
            downright.push_back(cornersolve(image, upright[0], downleft[0], corners2));

        }
        if (ur == 1 && dr == 1 && dl == 0)
        {
            upleft[0] = selonepoint(upleft, upright[0], image);
            downleft.push_back(cornersolve(image, upleft[0], downright[0], corners2));
        }
        if (dr == 1 && ur == 1 && ul == 0)
        {
            downleft[0] = selonepoint(downleft, downright[0], image);
            upleft.push_back(cornersolve(image, downleft[0],upright[0], corners2));
        }
        if (dl == 1 && dr == 1 && ul == 0)
        {
            upright[0] = selonepoint(upright, downright[0], image);
            upleft.push_back(cornersolve(image, downleft[0], upright[0], corners2));
        }
        if (dr == 1 && dr == 1 && ur == 0)
        {
            upleft[0] = selonepoint(upleft, downleft[0], image);
            upright.push_back(cornersolve(image, downright[0], upleft[0], corners2));
        }

    }
    if (onept == 1 && twopt == 2 && zeropt == 1)
    {
        flag = 1;
        if(dl==0&&dr==1)
        {upright[0]= selonepoint(upright, downright[0], image);
            upleft[0] = selonepoint(upleft, downright[0], image);
            downleft.push_back(cornersolve(image, upleft[0], downright[0], corners2));
        }
        if (dl == 1 && dr == 0)
        {
            upleft[0] = selonepoint(upleft, downleft[0], image);
            upright[0] = selonepoint(upright, upleft[0], image);
            downright.push_back(cornersolve(image, upright[0], downleft[0], corners2));
        }
        if (ur == 0 && dr == 1)
        {
            downleft[0] = selonepoint(downleft, downright[0], image);
            upleft[0] = selonepoint(upleft, downleft[0], image);
            upright.push_back(cornersolve(image, downright[0], upleft[0], corners2));
        }
        if (dr == 0 && ur == 1)
        {
            upleft[0] = selonepoint(upleft, upright[0], image);
            downleft[0] = selonepoint(downleft, upleft[0], image);
            downright.push_back(cornersolve(image,upright[0], downleft[0], corners2));
        }
        if (ur == 0 && dl == 1)
        {
            upleft[0] = selonepoint(upleft, downleft[0], image);
            downright[0] = selonepoint(downright, downleft[0], image);
            upright.push_back(cornersolve(image, downright[0], upleft[0], corners2));
        }
        if (ur == 1 && dl == 0)
        {
            upleft[0] = selonepoint(upleft, upright[0], image);
            downright[0] = selonepoint(downright, upright[0], image);
            downleft.push_back(cornersolve(image, upleft[0], downright[0], corners2));
        }
        if (ul == 1 && dr == 0)
        {
            upright[0] = selonepoint(upright, upleft[0], image);
            downleft[0] = selonepoint(downleft, upleft[0], image);
            downright.push_back(cornersolve(image, upright[0], downleft[0], corners2));
        }
        if (ul == 0 && dr == 1)
        {
            upright[0] = selonepoint(upright, downright[0], image);
            downleft[0] = selonepoint(downleft, downright[0], image);
            upleft.push_back(cornersolve(image, downleft[0], upright[0], corners2));
        }
        if (ul == 0 && dl == 1)
        {
            downright[0] = selonepoint(downright, downleft[0], image);
            upright[0] = selonepoint(upright, downright[0], image);
            upleft.push_back(cornersolve(image, downleft[0], upright[0], corners2));
        }
        if (ul == 1 && dl == 0)
        {
            upright[0] = selonepoint(upright, upleft[0], image);
            downright[0] = selonepoint(downright, upright[0], image);
            downleft.push_back(cornersolve(image, upleft[0], downright[0], corners2));
        }
        if (ul == 0 && ur == 1)
        {
            downright[0] = selonepoint(downright, upright[0], image);
            downleft[0] = selonepoint(downleft, downright[0], image);
            upleft.push_back(cornersolve(image, downleft[0], upright[0], corners2));
        }
        if (ul == 1 && ur == 0)
        {
            downleft[0] = selonepoint(downright, upleft[0], image);
            downright[0] = selonepoint(downleft, downleft[0], image);
            upright.push_back(cornersolve(image, downright[0], upleft[0], corners2));
        }

    }
    if (onept == 3 && zeropt == 1)
    {
        flag = 1;
        if (ul == 0)
        {
            upleft.push_back(cornersolve(image, downleft[0], upright[0], corners2));
        }
        if (ur == 0)
        {
            upright.push_back(cornersolve(image, downright[0], upleft[0], corners2));
        }
        if (dl == 0)
        {
            downleft.push_back(cornersolve(image, upleft[0], downright[0], corners2));
        }
        if (dr == 0)
        {
            downright.push_back(cornersolve(image, upright[0], downleft[0], corners2));
        }
    }
    if (onept == 2 && zeropt == 2 && (ul*dr == 1 || ur*dl == 1))
    {
        flag = 1;
        if (ul*dr == 1)
        {
            upright.push_back(cornersolve(image, downright[0], upleft[0], corners2));
            downleft.push_back(cornersolve(image, upleft[0], downright[0], corners2));
        }
        if (ur*dl == 1)
        {
            upleft.push_back(cornersolve(image, downleft[0], upright[0], corners2));
            downright.push_back(cornersolve(image, upright[0], downleft[0], corners2));
        }
    }

    if (flag == 1)
    {
        cornersback.push_back(upleft[0]);
        cornersback.push_back(upright[0]);
        cornersback.push_back(downleft[0]);
        cornersback.push_back(downright[0]);

        return cornersback;
    }
    else
    {
        Point2f a, b, c, d;
        a.x = 0; a.y = 0;
        b.x = image.rows; b.y = 0;
        c.x = 0; c.y = image.cols;
        d.x = image.rows; d.y = image.cols;
        cornersback.push_back(a);
        cornersback.push_back(b);
        cornersback.push_back(c);
        cornersback.push_back(d);
        return cornersback;
    }

}


Point2f cornersolve(Mat image, Point2f corner1, Point2f corner2, vector<Point2f>corners)//计算corners1所在竖直线与corners2所在水平线之间的交点
{

    float k,result; int d; int dmax = 0; int resmax = 0; Point2f back1, back2,back3;
    for (int m = 0; m < corners.size(); m++)
    {
        k = fabs((corners[m].y - corner1.y) / (corners[m].x - corner1.x + 1e-6));
        if (k >= 1.73) //竖直的线大于75度角
        {
            d = (corners[m].y - corner1.y)*(corners[m].y - corner1.y) + (corners[m].x - corner1.x)*(corners[m].x - corner1.x);
            if (d >= halfy*halfy)
            {
                result = countwhitepoint(image, corners[m], corner1);
                if (result >= resmax && d>dmax && result >= 0.67)
                {
                    resmax = result;
                    dmax = d;
                    back1 = corners[m];


                }

            }
        }

    }



    dmax = 0; resmax = 0;
    for (int n = 0; n < corners.size(); n++)
    {
        k = fabs((corner2.y - corners[n].y) / (corner2.x - corners[n].x + 1e-6));

        if (k >= 0 && k <= 0.27)//水平的线小于15度角
        {
            d = (corner2.y - corners[n].y)*(corner2.y - corners[n].y) + (corner2.x - corners[n].x)*(corner2.x - corners[n].x);
            if (d >= halfx*halfx)
            {
                result = countwhitepoint(image, corner2, corners[n]);
                if (result >= resmax && d>dmax && result>=0.67)
                {
                    resmax = result;
                    dmax = d;
                    back2 = corners[n];


                }
            }
        }
    }

    double k1, k2,b1,b2;
    k1 = double((corner1.y - back1.y) / (corner1.x - back1.x));
    k2= double((corner2.y - back2.y) / (corner2.x - back2.x));

    b1 = double(corner1.y - k1*corner1.x);
    b2= double(corner2.y - k1*corner2.x);

    back3.x = (b2 - b2) / (k1 - k2);
    back3.y = k1*back3.x+b1;

    return back3;

}

vector<int> contourjudge(Mat image,Mat image1 ,vector<Point2f>corners )//经过该角点是否具有满足条件的竖直和水平直线进行删选
{

    float k;
    vector<Point2f> corners2,corners3;
    int d,flagx=0,flagy=0,flag4=0,numa=0 ;
    for (int m = 0; m < corners.size(); m++)
    {
        float result=0, result2=0;
        for (int n = 0; n < corners.size(); n++)
        {
            k = fabs((corners[m].y - corners[n].y) / (corners[m].x - corners[n].x + 1e-6));

            if (k >= 0 && k <= 0.27)//水平的线小于15度角
            {
                d = (corners[m].y - corners[n].y)*(corners[m].y - corners[n].y) + (corners[m].x - corners[n].x)*(corners[m].x - corners[n].x);
                if (d >= halfx*halfx)
                {
                    result = countwhitepoint(image, corners[m], corners[n]);
                    if (result >= 0.67)
                    {
                        flagx = 1;
                        numa++;

                    }
                }
            }
            if (k >= 1.73) //竖直的线大于75度角
            {
                d = (corners[m].y - corners[n].y)*(corners[m].y - corners[n].y) + (corners[m].x - corners[n].x)*(corners[m].x - corners[n].x);
                if (d >= halfy*halfy)
                {
                    result = countwhitepoint(image, corners[m], corners[n]);
                    if (result >= 0.67)
                    {
                        flagy = 1;
                        numa++;

                    }

                }
            }
        }
        if(flagx==1&&flagy==1)  //即这个角点即有满足条件的水平线也有满足条件的竖直线
        {


            corners2.push_back(corners[m]);


        }



        flagx = 0;
        flagy = 0;
        numa = 0;


    }

    corners3=contourselect(image, corners2, corners);
    for (int i = 0; i < corners3.size(); i++)
    {
        circle(image1, corners3[i], 5, Scalar(0, 0, 255), 2, 8);
        char ch[100];
        sprintf(ch, "%d", i);
        string str;
        str = ch;
        putText(image1, str, corners3[i], FONT_HERSHEY_SIMPLEX, 1, Scalar(255, 23, 0), 2, 4);
    }

    vector<int> mcorner;
    for(int i=0;i<4;i++)
    {
        mcorner.push_back(corners3[i].x);
        mcorner.push_back(corners3[i].y);
    }
    return mcorner;


}

float countwhitepoint(Mat image, Point2f corners, Point2f corners1)//判断两点之间白色像素点的个数
{

    int x1 = corners.x,x2=corners1.x,y1= corners.y,y2=corners1.y;
    int a = abs(corners.x - corners1.x);
    int b= abs(corners.y - corners1.y);
    int X,Y,num=0,num1=0;
    float k = b /( a+1e-6),Tmpb,delta,result;

    if (a - b > 0)
    {
        if (x2 < x1)
        {
            X = x2;
            Y = y2;
            x2 = x1;
            y2 = y1;
            x1 = X;
            y1 = Y;
        }
        for (int i = x1; i <= x2; i++)
        {

            num++;

            int j = (i - x1)*(y2 - y1) / (x2 - x1+1e-6) + y1;
            uchar *data = image.ptr<uchar>(j);
            int pixel = data[i];


            if (pixel == 255)
            {
                num1++;
            }



        }
    }


    else
    {

        if (y2 < y1)
        {
            X = x2;
            Y = y2;
            x2 = x1;
            y2 = y1;
            x1 = X;
            y1 = Y;
        }
        for (int j = y1; j <= y2; j++)
        {
            int i = (j - y1)*(x2 - x1) / (y2 - y1+1e-6) + x1;
            num++;

            uchar *data = image.ptr<uchar>(j);
            int pixel = data[i];

            if (pixel == 255)
            {
                num1++;
            }






        }


    }
    float num2 = num;
    result = num1 / num2;

    return result;



}
