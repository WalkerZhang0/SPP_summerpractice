#include "SPP_Engine.h"
#include <fstream>
#include <iostream>

using namespace std;

class FileHandle
{
public:
    /// <summary>
    /// �ļ�����
    /// </summary>
    /// <param name="filepath">  </param>
    /// <param name="filepath2"></param>
    FileHandle(string filepath, string filepath2);
    /// @brief ��ȡͷ�ļ�
    /// @return
    bool ReadOHead();
    /// @brief ��ȡȫ����Ԫ��Ϣ
    /// @param oepoch
    /// @return
    bool ReadOBody(vector<OEpoch> &oepoch);
    /// @brief ��ȡ������Ԫ����
    /// @param opeoch
    /// @return
    bool ReadOneBody(OEpoch &opeoch);
    /// @brief ��ȡn�ļ�ͷ
    /// @return
    bool ReadNHead();
    /// @brief ��ȡn�ļ��۲�ֵ
    /// @param nbody
    /// @return
    bool ReadNBody(vector<NBody> &nbody);

    OHeader oheader_;
    NHeader nheader_;

protected:
    std::fstream filer_;
    std::fstream filen_;
    GNSS gnss_;

private:
    // �洢α��������
    vector<int> column_;
    int C1 = 0, C2 = 0;
    int S1 = 0, S2 = 0;
    int types;
    /// @brief �޳��ַ����еĶ�����
    /// @param str
    static void Erase(string &str);
    /// @brief ��string����ת����double����
    /// @param str
    static vector<double> stringToDouble(string &str);
    /// @brief D to E ��ͬ��ѧ�������ı�ʾ
    /// @param str
    static void DtoE(string &str);
};
