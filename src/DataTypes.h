#include <Eigen/Dense>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <algorithm>

using namespace Eigen;

#define SQR(x) ((x) * (x))
#define F1 1575.42e6                         // L1 ��Ƶ��
#define F2 1227.60e6                         // L2 ��Ƶ��
#define FF1 (F1 * F1 / (F1 * F1 - F2 * F2))  // �����L1����
#define FF2 (F2 * F2 / (F1 * F1 - F2 * F2))  // �����L2����
#define C 299792458                          // ����
#define GM 3.986004418e14                    // ������������
#define ROT 7.2921151467e-5                  // ������ת�ٶ�
#define PI 3.1415926535897932384626433832795 // Բ����
#define F_inv 298.257223563                  // ���ʵ���
#define a 6378136.55
#define b (a - a / F_inv) // �̰���
#define D2R PI / 180
#define R2D 180 / PI
#define F -4.442807633e-10 // ����۸�������

typedef struct OHeader
{
    Vector3d appcoor;       // 近似坐标
    Vector3d atncoor;       // 天线坐标
    double interval;        // 时间间隔
    std::tm starttime;      // 起始时间
    std::tm endtime;        // 截止时间
    std::string markername; // 站点名
} OHeader;

typedef struct GNSS
{
    double C1C;       // 频率1的伪距
    double C2W;       // 频率2的伪距
    double C12;       // 无电离层组合观测值
    std::string prn;  // 卫星编号
    Vector3d pos;     // 卫星位置
    double sow_sat;   // 卫星的发射时刻
    double clock_sat; // 卫星钟差
    double dist_r;    // 站星距离
    Vector3d dcm;     // 方向余弦系数
    int eph;          // 历元数字
    int aoe;          // 是否筛除
    double azel[2];   // azimuth + elevation
    double SNR1;      // C1信噪比
    double SNR2;      // C2信噪比
    double S12;       // 信噪比组合
} GNSS;

typedef struct OEpoch
{

    std::vector<GNSS> data; // ��̬����
    int year;
    int month;
    int day;
    int hour;
    int min;
    double sec;
    int tag;
    int numsat;
    bool isvaild; // ����Ԫ�Ƿ����4��4�����ϵ�����

    Vector4d state;     // ��վλ��
    Vector4d state_std; // ״̬std
} OEpoch;

typedef struct NHeader
{
    double ver;
    double ION_ALPHA[4];
    double ION_BETA[4];
    double DELTA_UTC[4];
    int leap;
} NHeader;

typedef struct NBody
{

    int PRN;
    int year;
    int month;
    int day;
    int hour;
    int min;
    int TOC;

    double sec;
    double sa1; //�����Ӳ�
    double sa2; //������ƫ
    double sa3; //������Ư

    double IODE;   //���ݡ���������ʱ��(��������)
    double Crs;    //����뾶�����ҵ��͸�������������λ��m��
    double deltan; //����ƽ���˶����������ֵ֮��(rad/s)
    double M0;     //�ο�ʱ���ƽ�����(rad)

    //���ݿ���������ݣ�
    double Cuc;   //ά�ȷ��ǵ����ҵ��͸���������(rad)
    double e;     //���ƫ����
    double Cus;   //������ǵ����ҵ��͸���������(rad)
    double sqrtA; //������ƽ����

    //���ݿ���������ݣ�
    double TOE;   //�����Ĳο�ʱ��(GPS������)
    double Cic;   //�����ǵ����ҵ��͸���������(rad)
    double OMEGA; //�ο�ʱ�̵�������ྭ
    double Cis;   //ά����ǵ����ҵ��͸���������(rad)

    //���ݿ���������ݣ�
    double i0;         //�ο�ʱ��Ĺ�����(rad)
    double Crc;        //���ƽ�������ҵ��͸���������(m)
    double omega;      //���ص�Ǿ�
    double deltaomega; //������ྭ�仯��(rad)

    //���ݿ���������ݣ�
    double IDOT;    //���ص�Ǿ�(rad/s)
    double L2code;  //L2�ϵ���
    double GPSweek; //GPS��,��TOEһͬ��ʾ
    double L2Pflag; //L2,p�����ݱ��

    // seven
    double TGD; // Ⱥ�ӳ���Ϣ
} NBody;

typedef struct result
{

    Vector3d enu;

    double std_x;
    double std_y;
    double std_z;
} result;

class Common
{
public:
    static std::string prnTostring(int prn)
    {

        if (prn < 10)
            return "G" + std::to_string(0) + std::to_string(prn);
        else
            return "G" + std::to_string(prn);
    }

    static void xyz2blh(double &X, double &Y, double &Z)
    {
        double tempX, tempY, tempZ;

        const double e = sqrt(a * a - b * b) / a;

        double B = 0.0, N = 0.0, H = 0.0, R0, R1, deltaH, deltaB;
        R0 = sqrt(pow(X, 2) + pow(Y, 2));
        R1 = sqrt(pow(X, 2) + pow(Y, 2) + pow(Z, 2));

        X = atan2(Y, X); // ����

        N = a;
        H = R1 - a;
        B = atan2(Z * (N + H), R0 * (N * (1 - e * e) + H));
        do
        {
            deltaH = N; //�ж���������
            deltaB = B;
            N = a / sqrt(1 - e * e * pow(sin(B), 2));
            H = R0 / cos(B) - N;
            B = atan2(Z * (N + H), R0 * (N * (1 - e * e) + H));
        } while (fabs(deltaH - H) < 1.0e-3 && fabs(deltaB - B) < 1.0e-9);
        Y = B;
        Z = H;
    }

    static Vector3d ecef2enu(double xs, double xy, double xz, Vector4d state)
    {
        double deltax = state[0] - xs;
        double deltay = state[1] - xy;
        double deltaz = state[2] - xz;

        xyz2blh(xs, xy, xz);
        double wd = xy;
        double jd = xs;

        Vector3d res; // enu
        res[0] = -sin(jd) * deltax + cos(jd) * deltay;
        res[1] = -sin(wd) * cos(jd) * deltax - sin(wd) * sin(jd) * deltay + cos(wd) * deltaz;
        res[2] = cos(wd) * cos(jd) * deltax + cos(wd) * sin(jd) * deltay + sin(wd) * deltaz;

        return res;
    }
};
