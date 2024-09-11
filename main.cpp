#include "src/FileHandle.h"
#include<iomanip>

using namespace std;

int main(int argc, char** argv) {

    if (argc < 4) {
        cout << "Usage Error: No file input! " << endl;
        cout << "CALL FORMAT: observationfile broadcastfile outputfilepath" << endl; return 0;
    }
    // string filepath = "./brst2460.22o";
    // string filepath2 = "./brdc1000.24n";
    vector<OEpoch> aepoch;
    vector<NBody> nbody;
    vector<result> res;
    FileHandle fh(argv[1], argv[2]);
    if (!(fh.ReadNHead() && fh.ReadNBody(nbody) && fh.ReadOHead() && fh.ReadOBody(aepoch)))    return 1;
    SPP_Engine spp = SPP_Engine(fh.oheader_.appcoor);
    cout << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> SPP >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    cout << ">>>>>>>>>>>>>> POSTION(WGS84): " << endl;
    cout << endl;
    cout << endl;
    cout << "X/m" << setw(25) << "Y/m" << setw(25) << "Z/m" << endl;
    for (int i = 0; i < aepoch.size(); i++)
    {
        spp.SolveOne(aepoch[i], nbody, fh.oheader_);
        cout<<fixed << setprecision(5) <<  aepoch[i].state[0] << setw(25) << aepoch[i].state[1] << setw(25) << aepoch[i].state[2] << endl;
    }

    Vector3d res_ave;
    double x=0,y=0,z=0;

    for (int i = 0; i < aepoch.size(); i++)
    {
        x += aepoch[i].state[0];
        y += aepoch[i].state[1];
        z += aepoch[i].state[2];
    }
    res_ave[0] = x / aepoch.size();
    res_ave[1] = y / aepoch.size();
    res_ave[2] = z / aepoch.size();

    Vector3d enu_ave;
    enu_ave<<0,0,0;
    result r;
    
    for (int i = 0; i < aepoch.size(); i++)
    {
        Vector3d enu;
        enu = Common::ecef2enu(res_ave[0], res_ave[1], res_ave[2], aepoch[i].state);
        r.enu = enu;
        res.push_back(r);
    }
    for (int i = 0; i < aepoch.size(); i++)
    {
        aepoch[i].state_std[0] = aepoch[i].state[0] - res_ave[0];
        aepoch[i].state_std[1] = aepoch[i].state[1] - res_ave[1];
        aepoch[i].state_std[2] = aepoch[i].state[2] - res_ave[2];
    }

     ofstream outfile;
     //outfile.open("G:\\sppres\\stat_shao.txt");
     //outfile <<setw(25)<<"postion X(m)" <<setw(25)<<"postion Y(m)" <<setw(25)<<"position Z(m)"
     //    << setw(25) << "dX" << setw(25) << "dY"<<setw(25)<<"dZ"<< endl;
     //for (int i = 0; i < aepoch.size(); i++)
     //{
     //    outfile << fixed << setprecision(5) << setw(25) << aepoch[i].state[0] << setw(25) << aepoch[i].state[1] << setw(25) << aepoch[i].state[2] <<
     //        setw(25) << aepoch[i].state_std[0] << setw(25) << aepoch[i].state_std[1] << setw(25) << aepoch[i].state_std[2] <<endl;
     //}

     outfile.open(argv[3]);
     outfile << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>> SPP >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
     outfile << ">>>>>>>>>>>>>> 观测时段:  " << fh.oheader_.starttime.tm_year << "年" <<
         fh.oheader_.starttime.tm_mon << "月" << fh.oheader_.starttime.tm_mday << "日" <<
         fh.oheader_.starttime.tm_hour << "时" << fh.oheader_.starttime.tm_min << "分" <<
         fh.oheader_.starttime.tm_sec << "秒"<< "->" << fh.oheader_.endtime.tm_year << "年" <<
         fh.oheader_.endtime.tm_mon << "月" << fh.oheader_.endtime.tm_mday << "日" <<
         fh.oheader_.endtime.tm_hour << "时" << fh.oheader_.endtime.tm_min << "分" <<
         fh.oheader_.endtime.tm_sec << "秒" << endl;
     outfile << ">>>>>>>>>>>>>> 测站 :" << fh.oheader_.markername << endl;
     outfile << ">>>>>>>>>>>>>> POSTION(站心残差): " << endl;
     outfile << endl;
     outfile << endl;
     outfile << "E/m" << setw(25) << "N/m" << setw(25) << "U/m" << endl;
     for (int i = 0; i < aepoch.size(); i++)
     {
         outfile << fixed << setprecision(5)  
      << res[i].enu[0] << setw(25) << res[i].enu[1] << setw(25) << res[i].enu[2] << endl;
     }
     outfile.close();

     return 0;

}