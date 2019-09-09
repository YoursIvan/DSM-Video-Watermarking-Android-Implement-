

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <math.h>
#include <cstdlib>
#include <vector>
#include <random>
using namespace std;

int decToBin(int dec){
    int result = 0,temp = dec,j = 1;
    while(temp){
        result = result + j*(temp%2);
        temp = temp/2;
        j = j*10;
    }
    return result;
}


char BinTodec(vector<int> bin){
    int res = 0;
    int e = 1;
    for (int i = 7; i >=0 ; --i) {
        res += bin[i]* e;
        e*=2;
    }
    return char(res);
}


bool MatMul(vector<vector<double> > a, vector<vector<double> > b, vector<vector<double> > &c)
{
    for (int i = 0; i < a.size(); i++)
    {
        for (int j = 0; j < b[0].size(); j++)
        {
            c[i][j] = 0;
            for (int k = 0; k < b[0].size(); k++)
            {
                c[i][j] += a[i][k] * b[k][j];
            }
        }
    }
    return true;
}


vector<vector<double>> MatInv(vector<vector<double>> a)
{
    for (int i = 0; i < a[0].size(); i++) {
        for (int j = i + 1; j < a.size(); j++) {
            double temp = a[i][j];
            a[i][j] = a[j][i];
            a[j][i] = temp;
        }
    }
    return a;
}


vector<int> randperm(int Num,int seed)
{
    vector<int> temp;
    vector<int> flag(Num,0);
    vector<int> res;
    int step = (seed * seed) %Num;
    if(step<2)
        step += 2;
    int pos = step;
    for (int i = 0; i < Num; ++i)
    {
        temp.push_back(i);
    }
    for (int j = 0; j < Num ; ++j) {
        while(flag[pos])
            pos++;
        if(pos>=Num)
            pos = pos%Num;
        res.push_back(temp[pos]);
        flag[pos] = 1;
        pos += step;
        if(step<2)
            step += 2;
        step = (step+seed*seed)%Num;
        if(pos>=Num)
            pos = pos%Num;
    }
    return res;
}

string padstring(string s,int length)
{
    while(s.length()<length)
        s = '0' + s;
    return s;
}


double BER(string emb_msg,string ext_msg)
{
    ifstream file1,file2;

    file1.open(emb_msg,ios::in);
    file2.open(ext_msg,ios::in);
    string message1,message2;
    file1>>message1;
    file2>>message2;

    int len1 = message1.length();
    int len2 = message2.length();
    int len = len1;
    if(len2<len1)
        len = len2;
    int len_bits = len * 8 ;
    int errorCount = 0;
    for(int i = 0; i < len; i++)
    {
        int a_ascii_emb = int(message1[i]);
        int a_ascii_ext = int(message2[i]);
        string temp1 = to_string(decToBin(a_ascii_emb));
        string temp2 = to_string(decToBin(a_ascii_ext));
        temp1 = padstring(temp1,8);
        temp2 = padstring(temp2,8);

        for (int k = 0; k < 8; k++) {
            if(temp1[k]!= temp2[k])
                errorCount++;
        }
    }

    return double(errorCount)/double(len_bits);

}