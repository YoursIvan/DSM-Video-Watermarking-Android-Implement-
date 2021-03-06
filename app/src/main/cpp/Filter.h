#ifndef FILTER_H_
#define FILTER_H_

#include <vector>
#include <string.h>

using namespace std;

#ifdef __cplusplus

class Filter
{
public:
    Filter(int type);
    ~Filter();

    vector<double> db1L = { 0.7071, 0.7071 };
    vector<double> db1H = { -0.7071, 0.7071 };
    vector<double> db1LR ={ 0.7071, 0.7071 };
    vector<double> db1HR ={ 0.7071, -0.7071 };

    vector<double> db2L  =  { -0.1294, 0.2241, 0.8365, 0.4830 };
    vector<double> db2H  =  { -0.4830, 0.8365, -0.2241, -0.1294 };
    vector<double> db2LR  =  { 0.4830, 0.8365, 0.2241, -0.1294 };
    vector<double> db2HR  =  { -0.1294, -0.2241, 0.8365, -0.4830 };

    vector<double> db3L  = { 0.0352, -0.0854, -0.1350, 0.4599, 0.8069, 0.3327 };
    vector<double> db3H  = { -0.3327, 0.8069, -0.4599, -0.1350, 0.0854, 0.0352 };
    vector<double> db3LR  = { 0.3327, 0.8069, 0.4599, -0.1350, -0.0854, 0.0352 };
    vector<double> db3HR  = { 0.0352, 0.0854, -0.1350, -0.4599, 0.8069, -0.3327 };

    vector<double> db4L  = { -0.0106, 0.0329, 0.0308, -0.1870, -0.0280, 0.6309, 0.7148, 0.2304 };
    vector<double> db4H  = { -0.2304, 0.7148, -0.6309, -0.0280, 0.1870, 0.0308, -0.0329, -0.0106 };
    vector<double> db4LR  = { 0.2304, 0.7148, 0.6309, -0.0280, -0.1870, 0.0308, 0.0329, -0.0106 };
    vector<double> db4HR  = { -0.0106, -0.0329, 0.0308, 0.1870, -0.0280, -0.6309, 0.7148, -0.2304 };

    vector<double> db5L  =   { 0.1601, 0.6038, 0.7243, 0.1384, -0.2423, -0.0322, 0.0776, -0.0062, -0.0126, 0.0033 };
    vector<double> db5H  =   { 0.0033, 0.0126, -0.0062, -0.0776, -0.0322, 0.2423, 0.1384, -0.7243, 0.6038, -0.1601 };
    vector<double> db5LR  =   {  0.1601, 0.6038, 0.7243, 0.1384, -0.2423,
        -0.0322, 0.0776, -0.0062, -0.0126, 0.0033};
    vector<double> db5HR  =   {0.0033, 0.0126, -0.0062, -0.0776, -0.0322,
        0.2423, 0.1384, -0.7243, 0.6038, -0.1601};

    vector<double> db6L  =   {-0.0011, 0.0048, 0.0006, -0.0316, 0.0275, 0.0975,
        -0.1298, -0.2263, 0.3153, 0.7511, 0.4946, 0.1115};
    vector<double> db6H  =   { -0.1115, 0.4946, -0.7511, 0.3153, 0.2263, -0.1298
        - 0.0975, 0.0275, 0.0316, 0.0006, -0.0048, -0.0011};
    vector<double> db6LR  =   { 0.1115, 0.4946, 0.7511, 0.3153, -0.2263, -0.1298,
        0.0975, 0.0275, -0.0316, 0.0006, 0.0048, -0.0011};
    vector<double> db6HR  =   {-0.0011, -0.0048, 0.0006, 0.0316, 0.0275, -0.0975,
        -0.1298, 0.2263, 0.3153, -0.7511, 0.4946, -0.1115};


    vector<double> db7L  =   {0.0004, -0.0018, 0.0004, 0.0126, -0.0166, -0.0380, 0.0806,
        0.0713, -0.2240, -0.1439, 0.4698, 0.7291, 0.3965, 0.0779};
    vector<double> db7H  =   {-0.0779, 0.3965, -0.7291, 0.4698, 0.1439, -0.2240, -0.0713,
        0.0806, 0.0380 - 0.0166, -0.0126, 0.0004, 0.0018, 0.0004};
    vector<double> db7LR  =   {0.0779, 0.3965, 0.7291, 0.4698, -0.1439, -0.2240, 0.0713,
        0.0806, -0.0380, -0.0166, 0.0126, 0.0004, -0.0018, 0.0004};
    vector<double> db7HR  =   { 0.0004, 0.0018, 0.0004, -0.0126, -0.0166, 0.0380, 0.0806,
        -0.0713, -0.2240, 0.1439, 0.4698, -0.7291, 0.3965, -0.0779};

    vector<double> db8L  =   { -0.0001, 0.0007, -0.0004, -0.0049, 0.0087, 0.0140, -0.0441,
        -0.0174, 0.1287, 0.0005, -0.2840, -0.0158, 0.5854, 0.6756, 0.3129, 0.0544};
    vector<double> db8H  =   {-0.0544, 0.3129, -0.6756, 0.5854, 0.0158, -0.2840, -0.0005,
        0.1287, 0.0174, -0.0441, -0.0140, 0.0087, 0.0049, -0.0004, -0.0007, -0.0001};
    vector<double> db8LR  =   { 0.0544, 0.3129, 0.6756, 0.5854, -0.0158, -0.2840, 0.0005,
        0.1287, -0.0174, -0.0441, 0.0140, 0.0087, -0.0049, -0.0004, 0.0007, -0.0001};
    vector<double> db8HR  =   {-0.0001, -0.0007, -0.0004, 0.0049, 0.0087, -0.0140, -0.0441,
        0.0174, 0.1287, -0.0005, -0.2840, 0.0158, 0.5854, -0.6756, 0.3129, -0.0544};

    vector<double> db9L  =   {0.0000, -0.0003, 0.0002, 0.0018, -0.0043, -0.0047, 0.0224, 0.0003,
        -0.0676, 0.0307, 0.1485, -0.0968, -0.2933, 0.1332, 0.6573, 0.6048, 0.2438, 0.0381};
    vector<double> db9H  =   {  -0.0381, 0.2438, -0.6048, 0.6573, -0.1332, -0.2933, 0.0968, 0.1485,
        -0.0307, -0.0676, -0.0003, 0.0224, 0.0047, -0.0043, -0.0018, 0.0002, 0.0003, 0.0000};
    vector<double> db9LR  =   { 0.0381, 0.2438, 0.6048, 0.6573, 0.1332, -0.2933, -0.0968, 0.1485, 0.0307,
        -0.0676, 0.0003, 0.0224, -0.0047, -0.0043, 0.0018, 0.0002, -0.0003, 0.0000};
    vector<double> db9HR  =   {  0.0000, 0.0003, 0.0002, -0.0018, -0.0043, 0.0047, 0.0224, -0.0003, -0.0676,
        -0.0307, 0.1485, 0.0968, -0.2933, -0.1332, 0.6573, -0.6048, 0.2438, -0.0381};

    vector<double> db10L  =   { -0.0000, 0.0001, -0.0001, -0.0007, 0.0020, 0.0014, -0.0107,
        0.0036, 0.0332, -0.0295, -0.0714, 0.0931, 0.1274, -0.1959, -0.2498, 0.2812,
        0.6885, 0.5272, 0.1882, 0.0267};
    vector<double> db10H  =   { -0.0267, 0.1882, -0.5272, 0.6885, -0.2812, -0.2498, 0.1959,
        0.1274, -0.0931, -0.0714, 0.0295, 0.0332, -0.0036, -0.0107, -0.0014, 0.0020,
        0.0007, -0.0001, -0.0001, -0.0000};
    vector<double> db10LR  =   {  0.0267, 0.1882, 0.5272, 0.6885, 0.2812, -0.2498, -0.1959,
        0.1274, 0.0931, -0.0714, -0.0295, 0.0332, 0.0036, -0.0107, 0.0014, 0.0020,
        -0.0007, -0.0001, 0.0001, -0.0000};
    vector<double> db10HR  =   { -0.0000, -0.0001, -0.0001, 0.0007, 0.0020, -0.0014, -0.0107,
        -0.0036, 0.0332, 0.0295, -0.0714, -0.0931, 0.1274, 0.1959, -0.2498, -0.2812,
        0.6885, -0.5272, 0.1882, -0.0267};

     vector<double> lowFilterDec;
     vector<double> highFilterDec;
     vector<double> lowFilterRec;
     vector<double> highFilterRec;
     int length;

     void SetFilterType(int type);

};

#endif