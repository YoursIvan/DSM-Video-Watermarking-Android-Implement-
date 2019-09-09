

#include <string.h>
#include <vector>
#include "Filter.cpp"

using namespace std;

class Wavelet {
public:
    Wavelet() {
    }


    ~Wavelet() {
    }

    int dwt(vector<double> sourceData, int dataLen, Filter db, vector<double> &cA,
                     vector<double> &cD) {
        //TODO
        //db.lowFilterDec = new double[] { 2, 3, 1 };
        //db.highFilterDec = new double[] { 2, 3, 1 };
        //db.length = 3;

        if (dataLen < 2)
            return -1;
        if ((sourceData.empty()))
            return -1;

        int filterLen = db.length;
        int n, k, p;
        int decLen = (dataLen + filterLen - 1) / 2;
        double tmp = 0;

        for (n = 0; n < decLen; n++) {
            cA[n] = 0;
            cD[n] = 0;
            for (k = 0; k < filterLen; k++) {
                p = 2 * n - k + 1;

                //�źű��ضԳ�����
                if ((p < 0) && (p >= -filterLen + 1))
                    tmp = sourceData[-p - 1];
                else if ((p > dataLen - 1) && (p <= dataLen + filterLen - 2))
                    tmp = sourceData[2 * dataLen - p - 1];
                else if ((p >= 0) && (p < dataLen))
                    tmp = sourceData[p];
                else
                    tmp = 0;

                //�ֽ��Ľ��Ʋ�������-��Ƶ����
                cA[n] += db.lowFilterDec[k] * tmp;

                //�ֽ���ϸ�ڲ�������-��Ƶ����
                cD[n] += db.highFilterDec[k] * tmp;
            }
        }
        return decLen;
    }

    void idwt(vector<double> cA, vector<double> cD, int cALength, Filter db,
                       vector<double> &recData) {

        if ((cA.empty()) || (cD.empty()))
            return;
        int filterLen = db.length;

        int n, k, p;
        int recLen = 2 * cALength - filterLen + 1;
        if ((recLen + filterLen - 1) % 2 == 0)
            recLen += 1;

        for (n = 0; n < recLen; n++) {
            recData[n] = 0;
            for (k = 0; k < cALength; k++) {
                p = n - 2 * k + filterLen - 2;

                //�ź��ع�
                if ((p >= 0) && (p < filterLen)) {
                    recData[n] += db.lowFilterRec[p] * cA[k] + db.highFilterRec[p] * cD[k];
                }
            }
        }
    }

    vector<vector<double>> dwt2(vector<vector<double>> sourceData, int dataLen, Filter db,
                                         vector<vector<double>> &cA, vector<vector<double>> &cH,
                                         vector<vector<double>> &cV, vector<vector<double>> &cD) {
        vector<double> rowsData(dataLen);
        vector<double> colsData(dataLen);
        vector<vector<double>> rowsDwtData(dataLen, vector<double>(dataLen, 0));
        vector<vector<double>> rowcA(dataLen, vector<double>(dataLen, 0));
        vector<vector<double>> rowcD(dataLen, vector<double>(dataLen, 0));

        vector<vector<double>> colcA(dataLen / 2, vector<double>(dataLen / 2, 0));
        vector<vector<double>> colcD(dataLen / 2, vector<double>(dataLen / 2, 0));

        //Rows dwt
        for (int i = 0; i < dataLen; i++) {
            for (int j = 0; j < dataLen; j++) {
                rowsData[j] = sourceData[i][j];
            }
            dwt(rowsData, dataLen, db, rowcA[i], rowcD[i]);
        }

        //Cols dwt
        //MatInv
        vector<vector<double>> TempDataAH(dataLen / 2, vector<double>(dataLen, 0));
        for (int i = 0; i < dataLen; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                TempDataAH[j][i] = rowcA[i][j];
            }
        }
        vector<vector<double>> TempDataVD(dataLen / 2, vector<double>(dataLen, 0));
        for (int i = 0; i < dataLen; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                TempDataVD[j][i] = rowcD[i][j];
            }
        }

        //cA
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen; j++) {
                colsData[j] = TempDataAH[i][j];
            }
            dwt(colsData, dataLen, db, colcA[i], colcD[i]);
        }
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                cA[j][i] = colcA[i][j];
            }
        }

        //cH
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen; j++) {
                colsData[j] = TempDataAH[i][j];
            }
            dwt(colsData, dataLen, db, colcA[i], colcD[i]);
        }
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                cH[j][i] = colcD[i][j];
            }
        }

        //cV
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen; j++) {
                colsData[j] = TempDataVD[i][j];
            }
            dwt(colsData, dataLen, db, colcA[i], colcD[i]);
        }
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                cV[j][i] = colcA[i][j];
            }
        }

        //cD
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen; j++) {
                colsData[j] = TempDataVD[i][j];
            }
            dwt(colsData, dataLen, db, colcA[i], colcD[i]);
        }
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                cD[j][i] = colcD[i][j];
            }
        }

        //Wavelet Matrix
        vector<vector<double>> CoefMatrix(dataLen, vector<double>(dataLen, 0));
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                CoefMatrix[i][j] = cA[i][j];
            }
        }
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                CoefMatrix[dataLen / 2 + i][j] = cH[i][j];
            }
        }
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                CoefMatrix[i][dataLen / 2 + j] = cV[i][j];
            }
        }
        for (int i = 0; i < dataLen / 2; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                CoefMatrix[dataLen / 2 + i][dataLen / 2 + j] = cD[i][j];
            }
        }

        return CoefMatrix;
    }

    vector<vector<double>>
    idwt2(vector<vector<double>> A, vector<vector<double>> H, vector<vector<double>> V,
                   vector<vector<double>> D, int dataLen, Filter db) {
        vector<vector<double>> rowsData(dataLen, vector<double>(dataLen, 0));
        vector<vector<double>> colsData(dataLen, vector<double>(dataLen, 0));

        vector<vector<double>> rowsDwtData(dataLen, vector<double>(dataLen, 0));

        vector<vector<double>> waveletData(dataLen, vector<double>(dataLen, 0));
        for (int i = 0; i < dataLen; i++) {
            for (int j = 0; j < dataLen; j++) {
                if (i < dataLen / 2 && j < dataLen / 2)
                    waveletData[i][j] = A[i][j];
                if (i >= dataLen / 2 && j < dataLen / 2)
                    waveletData[i][j] = V[i - dataLen][j];
                if (i < dataLen / 2 && j >= dataLen / 2)
                    waveletData[i][j] = H[i][j - dataLen];
                if (i >= dataLen / 2 && j >= dataLen / 2)
                    waveletData[i][j] = D[i - dataLen][j - dataLen];
            }
        }


        vector<double> cA(dataLen / 2);
        vector<double> cD(dataLen / 2);

        //Rows idwt
        for (int i = 0; i < dataLen; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                cA[j] = waveletData[i][j];
                cD[j] = waveletData[i][dataLen / 2 + j];
            }
            idwt(cA, cD, dataLen / 2, db, rowsData[i]);
        }

        //Cols dwt
        //MatInv
        vector<vector<double>> TempData(dataLen / 2, vector<double>(dataLen, 0));
        for (int i = 0; i < dataLen; i++) {
            for (int j = 0; j < dataLen / 2; j++) {
                cA[j] = rowsData[j][i];
                cD[j] = rowsData[j + dataLen / 2][i];
            }
            idwt(cA, cD, dataLen / 2, db, colsData[i]);
        }

        vector<vector<double>> DataMatrix(dataLen, vector<double>(dataLen, 0));
        for (int i = 0; i < dataLen; i++) {
            for (int j = 0; j < dataLen; j++) {
                DataMatrix[i][j] = colsData[j][i];
            }
        }
        return DataMatrix;//split
    }

};