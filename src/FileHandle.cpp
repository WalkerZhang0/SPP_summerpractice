#include "FileHandle.h"

using namespace std;

FileHandle::FileHandle(string filepath, string filepath2)
{
    filer_.open(filepath, std::ios_base::in);
    filen_.open(filepath2, std::ios_base::in);
}

bool FileHandle::ReadOHead()
{
    if (!filer_.is_open())
    {
        return false;
    }
    string line;
    while (getline(filer_, line))
    {
        if (line.length() >= 73 && line.substr(60, 13) == "END OF HEADER")
            return true;
        else if (line.length() >= 79 && line.substr(60, 19) == "ANTENNA: DELTA H/E/N")
        {
            string temp = line.substr(0, 14);
            Erase(temp);
            oheader_.atncoor[0] = stod(temp);
            temp = line.substr(14, 14);
            Erase(temp);
            oheader_.atncoor[1] = stod(temp);
            temp = line.substr(28, 14);
            Erase(temp);
            oheader_.atncoor[2] = stod(temp);
        }
        else if (line.length() >= 79 && line.substr(60, 19) == "APPROX POSITION XYZ")
        {
            string temp = line.substr(0, 14);
            Erase(temp);
            oheader_.appcoor[0] = stod(temp);
            temp = line.substr(14, 14);
            Erase(temp);
            oheader_.appcoor[1] = stod(temp);
            temp = line.substr(28, 14);
            Erase(temp);
            oheader_.appcoor[2] = stod(temp);
        }
        else if (line.length() >= 71 && line.substr(60, 11) == "MARKER NAME")
        {
            string temp = line.substr(0, 4);
            oheader_.markername = temp;
        }
        else if (line.length() >= 68 && line.substr(60, 8) == "INTERVAL")
        {
            string temp = line.substr(0, 10);
            Erase(temp);
            oheader_.interval = stod(temp);
        }
        else if (line.length() >= 77 && line.substr(60, 17) == "TIME OF FIRST OBS")
        {
            string temp = line.substr(2, 4);
            oheader_.starttime.tm_year = stoi(temp);
            temp = line.substr(10, 2);
            oheader_.starttime.tm_mon = stoi(temp);
            temp = line.substr(16, 2);
            oheader_.starttime.tm_mday = stoi(temp);
            temp = line.substr(22, 2);
            oheader_.starttime.tm_hour = stoi(temp);
            temp = line.substr(28, 2);
            oheader_.starttime.tm_min = stoi(temp);
            temp = line.substr(33, 10);
            oheader_.starttime.tm_sec = stoi(temp);
        }
        else if (line.length() >= 76 && line.substr(60, 16) == "TIME OF LAST OBS")
        {
            string temp = line.substr(2, 4);
            oheader_.endtime.tm_year = stoi(temp);
            temp = line.substr(10, 2);
            oheader_.endtime.tm_mon = stoi(temp);
            temp = line.substr(16, 2);
            oheader_.endtime.tm_mday = stoi(temp);
            temp = line.substr(22, 2);
            oheader_.endtime.tm_hour = stoi(temp);
            temp = line.substr(28, 2);
            oheader_.endtime.tm_min = stoi(temp);
            temp = line.substr(33, 10);
            oheader_.endtime.tm_sec = stoi(temp);
        }
        else if (line.length() >= 79 && line.substr(60, 19) == "SYS / # / OBS TYPES")
        {
            int count = 1;  // ��¼����
            int column = 6; // ��ȡα��۲�ֵ����
            if (line.substr(0, 1) == "G")
            {
                types = stoi(line.substr(4, 2));
                while (column < 61)
                {
                    string temp = line.substr(column, 4);
                    Erase(temp);
                    if (temp.substr(0, 2) == "C1" && C1 == 0)
                    {
                        C1 = count;
                    }
                    else if (temp.substr(0, 2) == "C2" && C2 == 0)
                    {
                        C2 = count;
                    }
                    else if (temp.substr(0, 2) == "S1" && S1 == 0)
                    {
                        S1 = count;
                    }
                    else if (temp.substr(0, 2) == "S2" && S2 == 0)
                    {
                        S2 = count;
                    }
                    count += 1;
                    column += 4;
                }
            }
            else
                continue;
        }
        else
            continue;
    }
    return true;
}

bool FileHandle::ReadOBody(vector<OEpoch> &aepoch)
{
    if (!filer_.is_open())
        return false;
    string line = "";
    int count = 0;
    while (true)
    {
        count++;
        if (count == 7)
            int cc = 1;
        OEpoch oepoch;
        if (!ReadOneBody(oepoch))
        { // �����ȡʧ�ܣ�������ѭ��
            break;
        }
        aepoch.push_back(oepoch);
    }
    filer_.close();
    return true;
}

bool FileHandle::ReadOneBody(OEpoch &oepoch)
{
    string line, temp;
    int count = 0; // ��¼����Ԫ�Ƿ����Ŀ����ǵ�α��۲�ֵ
    vector<string> temps;
    if (getline(filer_, line))
    {
        if (line[0] == '>')
        {
            oepoch.year = stod(line.substr(2, 4));
            temp = line.substr(6, 3);
            Erase(temp);
            oepoch.month = stod(temp);
            temp = line.substr(9, 3);
            Erase(temp);
            oepoch.day = stod(temp);
            temp = line.substr(12, 3);
            Erase(temp);
            oepoch.hour = stod(temp);
            temp = line.substr(15, 3);
            Erase(temp);
            oepoch.min = stod(temp);
            temp = line.substr(18, 11);
            Erase(temp);
            oepoch.sec = stod(temp);
            temp = line.substr(29, 3);
            Erase(temp);
            oepoch.tag = stod(temp);
            temp = line.substr(32, 3);
            Erase(temp);
            oepoch.numsat = stod(temp);
        }
    }
    else
    {
        return false;
    } // û�л�����ݣ���ʾ�Ѿ������ļ���
    for (int i = 0; i < oepoch.numsat; i++)
    {
        // if(i == oepoch.numsat){break;}
        // else{
        getline(filer_, line);
        if (line[0] == 'G' && line.length() >= types * 16)
        {
            temps.clear();
            temp = line.substr(0, 3);
            Erase(temp);
            gnss_.prn = temp;
            for (int j = 0; j < types; j++)
            {
                temp = line.substr(3 + 16 * j, 16);
                temps.push_back(temp);
            }
            bool C1_space = std::all_of(temps[C1 - 1].begin(), temps[C1 - 1].end(), [](char c)
                                        { return c == ' '; });
            bool C2_space = std::all_of(temps[C2 - 1].begin(), temps[C2 - 1].end(), [](char c)
                                        { return c == ' '; });
            bool S1_space = std::all_of(temps[S1 - 1].begin(), temps[S1 - 1].end(), [](char c)
                                        { return c == ' '; });
            bool S2_space = std::all_of(temps[S2 - 1].begin(), temps[S2 - 1].end(), [](char c)
                                        { return c == ' '; });
            if (!C1_space && !C2_space && !S1_space && !S2_space)
            {
                count++;
                gnss_.C1C = stod(temps[C1 - 1]);
                gnss_.C2W = stod(temps[C2 - 1]);
                gnss_.SNR1 = stod(temps[S1 - 1]);
                gnss_.SNR2 = stod(temps[S2 - 1]);
                oepoch.data.push_back(gnss_);
            }
        }
    }
    if (count >= 4)
        oepoch.isvaild = true;
    else
        oepoch.isvaild = false;
    return true;
}

bool FileHandle::ReadNHead()
{
    string line, temp;
    vector<double> data;
    if (!filen_.is_open())
        return false;
    while (getline(filen_, line))
    {
        if (line.length() >= 80 && line.substr(60, 20) == "RINEX VERSION / TYPE")
        {
            temp = line.substr(0, 10);
            Erase(temp);
            nheader_.ver = stod(temp);
        }
        else if (line.substr(60, 9) == "ION ALPHA")
        {
            temp = line.substr(0, 59);
            DtoE(temp);
            data = stringToDouble(temp);
            nheader_.ION_ALPHA[0] = data[0];
            nheader_.ION_ALPHA[1] = data[1];
            nheader_.ION_ALPHA[2] = data[2];
            nheader_.ION_ALPHA[3] = data[3];
        }
        else if (line.substr(60, 8) == "ION BETA")
        {
            temp = line.substr(0, 59);
            DtoE(temp);
            data = stringToDouble(temp);
            nheader_.ION_BETA[0] = data[0];
            nheader_.ION_BETA[1] = data[1];
            nheader_.ION_BETA[2] = data[2];
            nheader_.ION_BETA[3] = data[3];
        }
        else if (line.substr(60, 20) == "DELTA-UTC: A0,A1,T,W")
        {
            temp = line.substr(0, 59);
            DtoE(temp);
            data = stringToDouble(temp);
            nheader_.DELTA_UTC[0] = data[0];
            nheader_.DELTA_UTC[1] = data[1];
            nheader_.DELTA_UTC[2] = data[2];
            nheader_.DELTA_UTC[3] = data[3];
        }
        else if (line.substr(60, 12) == "LEAP SECONDS")
        {
            temp = line.substr(0, 10);
            Erase(temp);
            nheader_.leap = stod(temp);
        }
        else if (line.substr(60, 13) == "END OF HEADER")
        {
            return true;
        }
        else
            continue;
    }
    return true;
}

bool FileHandle::ReadNBody(vector<NBody> &nbody)
{
    string line;
    int count = 0;
    vector<double> data;
    vector<string> body;
    NBody nb;

    //std::streampos beginpos = filen_.tellg();
    if (!filen_.is_open())
    {
        return false;
    }
    while (getline(filen_, line))
    {
        if (!line.empty())
            count++;
        body.push_back(line);
    }
    //filen_.clear();  // ������ܵĴ����־
    //filen_.seekg(beginpos);

    for (int i = 0; i < count / 8; i++)
    {
        string temp;
        for (int j = 0; j < 8; j++)
        {
            line = body[i * 8 + j];
            DtoE(line);
            switch (j)
            {
            case 0:
                temp = line.substr(0, 22);
                data = stringToDouble(temp);
                nb.PRN = data[0];
                nb.year = data[1] + 2000;
                nb.month = data[2];
                nb.day = data[3];
                nb.hour = data[4];
                nb.min = data[5];
                nb.sec = data[6];
                temp = line.substr(22, 19);
                Erase(temp);
                nb.sa1 = stod(temp);
                temp = line.substr(41, 19);
                Erase(temp);
                nb.sa2 = stod(temp);
                temp = line.substr(60, 19);
                Erase(temp);
                nb.sa3 = stod(temp);
                break;
            case 1:
                temp = line.substr(0, 22);
                Erase(temp);
                nb.IODE = stod(temp);
                temp = line.substr(22, 19);
                Erase(temp);
                nb.Crs = stod(temp);
                temp = line.substr(41, 19);
                Erase(temp);
                nb.deltan = stod(temp);
                temp = line.substr(60, 19);
                Erase(temp);
                nb.M0 = stod(temp);
                break;
            case 2:
                temp = line.substr(0, 22);
                Erase(temp);
                nb.Cuc = stod(temp);
                temp = line.substr(22, 19);
                Erase(temp);
                nb.e = stod(temp);
                temp = line.substr(41, 19);
                Erase(temp);
                nb.Cus = stod(temp);
                temp = line.substr(60, 19);
                Erase(temp);
                nb.sqrtA = stod(temp);
                break;
            case 3:
                temp = line.substr(0, 22);
                Erase(temp);
                nb.TOE = stod(temp);
                temp = line.substr(22, 19);
                Erase(temp);
                nb.Cic = stod(temp);
                temp = line.substr(41, 19);
                Erase(temp);
                nb.OMEGA = stod(temp);
                temp = line.substr(60, 19);
                Erase(temp);
                nb.Cis = stod(temp);
                break;
            case 4:
                temp = line.substr(0, 22);
                Erase(temp);
                nb.i0 = stod(temp);
                temp = line.substr(22, 19);
                Erase(temp);
                nb.Crc = stod(temp);
                temp = line.substr(41, 19);
                Erase(temp);
                nb.omega = stod(temp);
                temp = line.substr(60, 19);
                Erase(temp);
                nb.deltaomega = stod(temp);
                break;
            case 5:
                temp = line.substr(0, 22);
                Erase(temp);
                nb.IDOT = stod(temp);
                temp = line.substr(22, 19);
                Erase(temp);
                nb.L2code = stod(temp);
                temp = line.substr(41, 19);
                Erase(temp);
                nb.GPSweek = stod(temp);
                temp = line.substr(60, 19);
                Erase(temp);
                nb.L2Pflag = stod(temp);
                break;
            case 6:
                temp = line.substr(41, 19);
                Erase(temp);
                nb.TGD = stod(temp);
            case 7:
                continue;
            }
        }
        nbody.push_back(nb);
    }
    filen_.close();
    return true;
}

void FileHandle::Erase(string &str)
{
    str.erase(std::remove(str.begin(), str.end(), ' '), str.end());
}

vector<double> FileHandle::stringToDouble(string &str)
{
    vector<double> result;
    istringstream iss(str);
    double number;

    while (iss >> number)
    {
        result.push_back(number);
    }

    return result;
}

void FileHandle::DtoE(string &str)
{
    for (int i = 0; i < str.length(); i++)
        str[i] == 'D' ? str[i] = 'E' : str[i] = str[i];
}