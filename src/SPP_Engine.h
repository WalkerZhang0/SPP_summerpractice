#pragma once
#include "DataTypes.h"

class SPP_Engine {
public:
    SPP_Engine(Vector3d app);

    /// @brief UTC to TOW
    /// @param oepoch 
    /// @return 
    double UTCtoTOW(int year, int month, int day, int hour, int min, double sec);

    /// @brief ���������Ӳ�
    /// @param sow GPSʱ��Ϊtʱ��
    /// @param nbody ��������
    /// @return 
    double Clock(double sow, NBody nbody, double *var) {
        *var = 3.0 * 3.0;
        double res = 0.0;
        res = nbody.sa1 + nbody.sa2 * (sow - nbody.TOC) + nbody.sa3 * (sow - nbody.TOC) * (sow - nbody.TOC);
        return res;
    }

    /// @brief �������ǵ�λ��
    /// @param gnss 
    /// @param nbody 
    void CalSatPos(GNSS* gnss, NBody nbody,double sow);

    /// @brief �������λ�õĵ�����ת����
    /// @param position 
    /// @return 
    bool CorEarth(Vector3d& position, double transtime);

    /// @brief ���㵥��Ԫ��Ϣ
    /// @param oepoch 
    bool SolveOne(OEpoch &oepoch, std::vector<NBody> nbody, OHeader oheader);

    double Dist(Vector4d pos1, Vector3d pos2) {

        double dX = pos1[0] - pos2[0];
        double dY = pos1[1] - pos2[1];
        double dZ = pos1[2] - pos2[2];

        return sqrt(dX * dX + dY * dY + dZ * dZ);// +ROT / C * ((pos2[0] - pos1[0]) * pos2[1] - (pos2[1] - pos1[1] * pos2[0]));
    }

    void CalDCM(Vector4d state_, Vector3d pos, double R, GNSS* gnss) {

        Vector3d state;
        state << state_[0], state_[1], state_[2];

        gnss->dcm = (state - pos) / R;
    }

    void CalAzel(Vector4d &state, GNSS *gnss) {
        Vector3d deltaXYZ;
        double wd = state[1];
        double jd = state[0];
        double at = state[2];
        Common::xyz2blh(jd,wd,at);

        deltaXYZ[0] = gnss->pos[0] - state[0];
        deltaXYZ[1] = gnss->pos[1] - state[1];
        deltaXYZ[2] = gnss->pos[2] - state[2];

        double e = -sin(jd) * deltaXYZ[0] + cos(jd) * deltaXYZ[1];
        double n = -sin(wd) * cos(jd) * deltaXYZ[0] - sin(wd) * sin(jd) * deltaXYZ[1] + cos(wd) * deltaXYZ[2];
        double u = cos(wd) * cos(jd) * deltaXYZ[0] + cos(wd) * sin(jd) * deltaXYZ[1] + sin(wd) * deltaXYZ[2];

        gnss->azel[0] = atan2(e, n); // Azimuth 
        gnss->azel[0] = (gnss->azel[0] < 0) ? gnss->azel[0] + PI * 2 : (gnss->azel[0] > 2 * PI) ? gnss->azel[0] - 2 * PI : gnss->azel[0];
        gnss->azel[1] = asin(u / sqrt(e * e + u * u + n * n)); // Elevation
    }
    /// @brief eliminate error of trop
    /// @param oheader 
    /// @param azel 
    /// @param humi 
    /// @return 
    double TropSolve(OHeader oheader, const double* azel, double humi);

    bool CodeBiasTGD(GNSS& gnss_);

    bool lsp(MatrixXd A, Matrix4d &Q, VectorXd l, int n, Vector4d& res, MatrixXd P);

    bool seleph(GNSS *gnss, std::vector<NBody> nbody);
private:
    Vector4d state_; // ��ǰ��վ��״̬��Ϣ����վ����X,Y,Z ���ջ��Ӳ�
    GNSS* gnss_; // ��ǰ�����������µ�ĳһ������
    NBody nbody_; // ��ǰ��Ԫ������
    double E; 
};
