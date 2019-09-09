#ifndef WAVELET_H_
#define WAVELET_H_

#include <vector>
#include <string.h>
#include "Filter.h"



class Wavelet
{
public:
    Wavelet();
    ~Wavelet();
    int dwt(vector<double> sourceData, int dataLen, Filter db, vector<double> &cA, vector<double> &cD);
    void idwt(vector<double> cA, vector<double> cD, int cALength, Filter db, vector<double> &recData);

    vector<vector<double>> dwt2(vector<vector<double>> sourceData, int dataLen, Filter db, vector<vector<double>> & cA, vector<vector<double>> &cH, vector<vector<double>> &cV, vector<vector<double>>  &cD);
    vector<vector<double>> idwt2(vector<vector<double>> A,vector<vector<double>> H, vector<vector<double>> V,vector<vector<double>> D , int dataLen, Filter db);

};
#endif