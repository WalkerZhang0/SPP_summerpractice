#include "SPP_Engine.h"

const int GPS_YEAR = 1980;
const int GPS_MONTH = 1;
const int GPS_DAY = 6;

SPP_Engine::SPP_Engine(Vector3d app)
{
    // state_ << 0,0,0,0; // ��ʼ����ǰ��վ����״̬Ϊ0��0��0��0
    state_ << app[0], app[1], app[2], 0.0; // ��ʼ����ǰ��վ����״̬Ϊ0��0��0��0
    E = 0;
}

double SPP_Engine::UTCtoTOW(int year, int month, int day, int hour, int min, double sec)
{
    std::tm utc_tm = {};
    utc_tm.tm_year = year - 1900;
    utc_tm.tm_mon = month - 1;
    utc_tm.tm_mday = day;
    utc_tm.tm_hour = hour;
    utc_tm.tm_min = min;
    utc_tm.tm_sec = sec;

    std::time_t utc_time = std::mktime(&utc_tm);

    std::tm gps_time_tm = {};
    gps_time_tm.tm_year = GPS_YEAR - 1900;
    gps_time_tm.tm_mon = GPS_MONTH - 1;
    gps_time_tm.tm_mday = GPS_DAY;

    std::time_t gps_time = std::mktime(&gps_time_tm);

    double diffsec = std::difftime(utc_time, gps_time);

    int gpsweek = static_cast<int>(diffsec) / (7 * 24 * 60 * 60); // ȡ��
    double sow = diffsec - gpsweek * 7.0 * 24 * 60 * 60;

    return sow;
}

void SPP_Engine::CalSatPos(GNSS *gnss, NBody nbody, double sow)
{
    E = 0;

    double tk = gnss->sow_sat - nbody.TOE; // �黯ʱ��
    if (tk > 302400)
        tk = tk - 604800;
    else if (tk < -302400)
        tk = tk + 604800;

    double n = sqrt(GM / pow(nbody.sqrtA, 6)) + nbody.deltan;

    double M = nbody.M0 + n * tk;
    M > 2 *PI ? M = M - 2 * PI : (M < 0 ? M = M + 2 *PI : M = M);

    E = M;
    while (true)
    {
        double temp = E;
        E = M + nbody.e * sin(temp);
        if (abs(E - temp) < 1e-7)
            break;
    }

    double f = atan2(sqrt(1 - nbody.e * nbody.e) * sin(E), (cos(E) - nbody.e)); // -p ~ p
    f > 2 *PI ? f = f - 2 *PI : f < -2 *PI ? f = f + 2 *PI
                                           : f = f; // right or wrong

    double u_ = f + nbody.omega;

    double deltauk = nbody.Cus * sin(2 * u_) + nbody.Cuc * cos(2 * u_);
    double deltark = nbody.Crs * sin(2 * u_) + nbody.Crc * cos(2 * u_);
    double deltaik = nbody.Cis * sin(2 * u_) + nbody.Cic * cos(2 * u_);

    double uk = u_ + deltauk;
    double rk = nbody.sqrtA * nbody.sqrtA * (1 - nbody.e * cos(E)) + deltark;
    double ik = nbody.i0 + deltaik + nbody.IDOT * tk;

    double xk = rk * cos(uk);
    double yk = rk * sin(uk);

    double Lk = nbody.OMEGA + (nbody.deltaomega - ROT) * tk - ROT * nbody.TOE;

    gnss->pos[0] = xk * cos(Lk) - yk * cos(ik) * sin(Lk);
    gnss->pos[1] = xk * sin(Lk) + yk * cos(ik) * cos(Lk);
    gnss->pos[2] = yk * sin(ik);

    tk = gnss->sow_sat - nbody.TOC;

    gnss->clock_sat = nbody.sa1 + nbody.sa2 * (gnss->sow_sat - nbody.TOC) + nbody.sa3 * (gnss->sow_sat - nbody.TOC) * (gnss->sow_sat - nbody.TOC);

    gnss->clock_sat += F * nbody.e * nbody.sqrtA * sin(E);
}

bool SPP_Engine::CorEarth(Vector3d &position, double transtime)
{

    Vector3d deltapos;
    Matrix3d cormat;
    /* cormat << 0, ROT * transtime, 0,
        -ROT * transtime, 0, 0,
        0, 0, 0;
    deltapos = cormat * position;*/

    cormat << cos(ROT * transtime), sin(ROT * transtime), 0,
        -sin(ROT * transtime), cos(ROT * transtime), 0,
        0, 0, 1;
    position = cormat * position;

    return true;
}

double SPP_Engine::TropSolve(OHeader oheader, const double *azel, double humi)
{
    Vector3d pos;
    pos = oheader.appcoor;

    double bb, ll, hh;
    bb = oheader.appcoor[0];
    ll = oheader.appcoor[1];
    hh = oheader.appcoor[2];

    Common::xyz2blh(bb, ll, hh);

    const double temp0 = 15.0; /* temperature at sea level*/
    double hgt, pres, temp, e, z, trph, trpw;
    double temp1 = azel[1];
    double temp2 = hh;

    if (hh < -100.0 || 1E4 < hh || azel[1] <= 0)
        return 0.0;

    hgt = hh < 0.0 ? 0.0 : hh;

    pres = 1013.25 * pow(1.0 - 2.2557E-5 * hgt, 5.2568);
    temp = temp0 - 6.5E-3 * hgt + 273.16;
    e = 6.108 * humi * exp((17.15 * temp - 4684.0) / (temp - 38.45));

    /* saastamoinen model */
    z = PI / 2.0 - azel[1];
    trph = 0.0022768 * pres / (1.0 - 0.00266 * cos(2.0 * bb) - 0.00028 * hgt / 1E3) / cos(z);
    trpw = 0.002277 * (1255.0 / temp + 0.05) * e / cos(z);

    return trph + trpw;
}

bool SPP_Engine::CodeBiasTGD(GNSS &gnss)
{
    double alpha = SQR(F1) / SQR(F2);
    if (gnss.C1C > 1e-3)
        gnss.C1C -= gnss.tgd * C;
    if (gnss.C2W > 1e-3)
        gnss.C2W -= gnss.tgd * C * alpha;
    return true;
}

bool SPP_Engine::lsp(MatrixXd A, Matrix4d &Q, VectorXd l, int n, Vector4d &res, MatrixXd P)
{
    VectorXd r(n);
    if (A.rows() != n || A.cols() != 4 || l.size() != n)
    {
        return false;
    }
    Q = (A.transpose() * P * A).inverse();
    r = A.transpose() * P * l;
    res = Q * r;
    state_ += res;

    return true;
}

bool SPP_Engine::seleph(GNSS *gnss, std::vector<NBody> nbody)
{
    int tmin, tmax, t;
    t = 0;
    tmax = 7200 + 1;
    tmin = tmax + 1;

    for (int i = 0; i < nbody.size(); i++)
    {
        if (gnss_->prn != Common::prnTostring(nbody[i].PRN))
            continue;
        if ((t = abs(gnss->sow_sat - nbody[i].TOE)) > tmax)
            continue;
        if (t < tmin)
        {
            gnss->eph = i;
            tmin = t;
        }
    }

    return true;
}

bool SPP_Engine::SolveOne(OEpoch &oepoch, std::vector<NBody> nbody, OHeader oheader)
{

    int sow = UTCtoTOW(oepoch.year, oepoch.month, oepoch.day, oepoch.hour, oepoch.min, oepoch.sec); // sec of week
    int n = oepoch.data.size();                                                                     // number of sat
    double var_clock = 0.0;                                                                         // var of clock
    double var_iono = 0.0;                                                                          // var of iono
    double var_trop = 0.0, trop = 0.0;                                                              // var of trop / trop
    double pr = 0.0;                                                                                // the
    double var = 0.0;                                                                               // clock error
    int count = 0;                                                                                  // iternation numbers
    int j, coco;

    Matrix4d Q;
    MatrixXd A_temp = MatrixXd::Zero(n, 4);
    VectorXd l_temp = VectorXd::Zero(n);
    MatrixXd P_temp = MatrixXd::Zero(n, n);
    Vector4d res;

    if (oepoch.isvaild == false)
        return false;

    /* single satellite */
    while (true)
    {
        coco = 0;
        n = oepoch.data.size();
        for (int i = 0; i < oepoch.data.size(); i++)
        {
            gnss_ = &oepoch.data[i];
            /* IF combination */
            gnss_->C12 = FF1 * gnss_->C1C - FF2 * gnss_->C2W;
            gnss_->S12 = FF1 * gnss_->SNR1 + FF2 * gnss_->SNR2;
            double trans = gnss_->C12 / C;
            /* transmiss time */
            gnss_->sow_sat = sow - trans;
            /*  select eph */
            if (!seleph(gnss_, nbody))
                return false;
            j = gnss_->eph;
            nbody[j].TOC = UTCtoTOW(nbody[j].year, nbody[j].month, nbody[j].day, nbody[j].hour, nbody[j].min, nbody[j].sec);
            /* clock corrections */
            trans += Clock(gnss_->sow_sat, nbody[j], &var_clock);
            // gnss_->clock_sat = Clock(gnss_->sow_sat, nbody[j], &var_clock);
            gnss_->sow_sat -= Clock(gnss_->sow_sat, nbody[j], &var_clock);
            double dist = 0, trans2 = 0;
            /* iterations to calculate satellite postion */
            for (int k = 0; k < 10; k++)
            {
                if (k != 0)
                    gnss_->sow_sat = sow - trans;
                /* Calculate Satellite Postion */
                CalSatPos(gnss_, nbody[j], sow);
                if (!CorEarth(gnss_->pos, trans))
                    return false;
                dist = Dist(state_, gnss_->pos);
                trans2 = dist / C;
                if (abs(trans2 - trans) < 1e-7)
                    break;
                else
                    trans = trans2;
            }
            gnss_->aoe = 1;
            /* calcultae azimuth and elevation */
            CalAzel(state_, gnss_);
            /* cutoff 15 */
            if (gnss_->azel[1] < (15 * PI / 180))
            {
                gnss_->aoe = 0;
                n--;
                // continue;
            }
            if (gnss_->aoe == 0)
                continue;

            /* eliminate error of trop*/
            trop = TropSolve(oheader, gnss_->azel, 0.7);
            // var_trop = SQR(0.3 / (sin(gnss_->azel[1]) + 0.1));
            gnss_->dist_r = Dist(state_, gnss_->pos);
            /* calculate dcm of single satellite */
            CalDCM(state_, gnss_->pos, gnss_->dist_r, gnss_);
            /* calculate the last */
            pr = gnss_->C12 - gnss_->dist_r + C * gnss_->clock_sat - trop - state_[3];
            /* weight */
            //P_temp(coco, coco) = SQR(gnss_->S12 / 40.0); // sol 1: SNR
            P_temp(coco, coco) = 1; // sol 2: same
            //P_temp(coco, coco) = 1 / sin(gnss_->azel[1]); // sol 3: elevation
            //P_temp(coco, coco) = 1 / (pr*pr); // sol 4: residuals
            //P_temp(coco, coco) = SQR(gnss_->S12 / 40.0) / (pr*pr); // sol 5: residuals + SNR
            /* design matrix */
            for (int k = 0; k < 4; k++)
            {
                A_temp(coco, k) = (k != 3) ? gnss_->dcm[k] : 1.0;
            }
            l_temp[coco] = pr;
            coco++;
        }
        MatrixXd A = A_temp.block(0, 0, n, 4);
        VectorXd l = l_temp.block(0, 0, n, 1);
        MatrixXd P = P_temp.block(0, 0, n, n);
        /* lsp solve */
        if (!lsp(A, Q, l, n, res, P))
            return false;
        count++;
        /* end judge */
        if (((res[0] < 1E-3) && (res[1] < 1E-3) && (res[2] < 1E-3)) || count == 10)
        {
            oepoch.state = state_;
            count = 0;
            break;
        }
    }
    return 1;
}