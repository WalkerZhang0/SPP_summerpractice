#include <Eigen/Dense>
#include <iostream>

using namespace std;
using namespace Eigen;


int main(){
    MatrixXd A(3,2);
    Matrix3d P;
    P << 5,0,0,
        0,10,0,
        0,0,2.5;
    Vector3d l;
    l << 0, 0, -0.07;
    A << -1 ,0,
        0, 1,
        -1 ,1;
    
    Vector2d x;
    x = (A.transpose()*P*A).inverse()*A.transpose()*P*l;

    cout << x.transpose() <<endl;
    return 0;
}