#include<vector> 
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sstream>
#include <math.h>
#include <memory.h>
#include <string.h>
#include <cstdlib>

using namespace std;

//函数将A分解为U diag(S) V'；S[i],U[i],V[i]是A的第i大奇异值，及其对应的左歧义向量和右奇异向量
//S,U,V的size由K指定,K是需要分解的rank，0<K<=min(m,n)

const int MAX_ITER=100000;      //迭代器迭代次数
const double eps=0.0000001;      //允许的最大误差

double get_norm(double *x, int n)   //求sqrt(x[1]*x[1] + x[2]*x[2] +......+ x[n]*x[n])，                                     //求向量x的模长|x|；
{
    double r=0;
    for(int i=0;i<n;i++)
        r+=x[i]*x[i];
    return sqrt(r);
}


double normalize(double *x, int n)   //当求的的向量x的模小于eps最小值时，舍去；
{                        //当x的模大于eps时，保留；并将x[i]/|x|，将向量x归一化为单位向量e；
    double r=get_norm(x,n);        
    if(r<eps)
    return 0;
    for(int i=0;i<n;i++)
        x[i]/=r;
    return r;                 //返回向量x的模长，模长不能太小，
}


inline double product(double*a, double *b,int n)  //求的向量a和向量b的内积，即(a[1]*b[1] + a[2]*b[2] +......+ a[n]*b[n])；
{
    double r=0;
    for(int i=0;i<n;i++)
        r+=a[i]*b[i];
    return r;
}


void orth(double *a, double *b, int n)    //正交化；|a|=1 ，向量a为单位向量e，将向量b与单位
{                                         // 向量进行正交，
    double r=product(a,b,n);        //向量b与单位向量e的内积，即为向量b在单位向量e上的投影值
    for(int i=0;i<n;i++)
        b[i]-=r*a[i];                   //b'[i] = b[i] - (e, b)*b[i] ,得到与向量a正交的向                                         //量b' 即（a, b')的内积为0；
}


bool svd(vector<vector<double> > A, int K, std::vector<std::vector<double> > &U, std::vector<double> &S, std::vector<std::vector<double> > &V)
{                                //矩阵为A(M×N) ，A = U S (V^T)； U(M×K)  S(K×K)  V(N×K)
    int M=A.size();
    int N=A[0].size();
    U.clear();
    V.clear();
    S.clear();

    S.resize(K,0);      //S为对角矩阵，只需保存对角线上的奇异值即可，故S取向量保存，节约空间；                         //S保存K个元素即可   

    U.resize(K);              // U矩阵为K个列向量；每个列向量有M个元素；
    for(int i=0;i<K;i++)
    {
        U[i].resize(M,0);
    }

    V.resize(K);            // V矩阵也为K个列向量；每个列向量有N个元素；
    for(int i=0;i<K;i++)
    {
        V[i].resize(N,0);
    }

    srand(time(0));            //给一个随机种子数,

    double *left_vector=new double[M];          //动态分配内存,产生left_vector和                                                           //next_left_vector向量用于迭代运算
    double *next_left_vector=new double[M];
    double *right_vector=new double[N];
    double *next_right_vector=new double[N];

    while(1)
    {
        for(int i=0;i<M;i++)
            left_vector[i]= (float)rand() / RAND_MAX;    //随机生成一个向量vector（即α），                                                       //共有M个元素，用来求解左奇异矩阵 
        if(normalize(left_vector, M)>eps)               //当向量的模长大于eps时，生成结束；                                                        //生成一个模长合适的迭代向量
            break;
    }

    int col=0;
    for(int col=0;col<K;col++)    //按列计算左右奇异矩阵,迭代
    {
        double diff=1;
        double r=-1;

        for(int iter=0; diff>=eps && iter<MAX_ITER; iter++)     //迭代器iter的迭代次数为10000次
        {
            memset(next_left_vector,0,sizeof(double)*M);
            memset(next_right_vector,0,sizeof(double)*N);  //分配内存给左迭代向量和右迭代向量


            for(int i=0;i<M;i++)
            for(int j=0;j<N;j++)
                next_right_vector[j]+=left_vector[i]*A[i][j];   //向量α(1×M)×A(M×N)矩阵，得到右迭代向量β_next(1×N)

            r=normalize(next_right_vector,N);                 //单位化向量β_next

            if(r<eps) break;                                  //若β_next 模长太小，则退出内层循环
            for(int i=0;i<col;i++)
                orth(&V[i][0],next_right_vector,N);              //与右矩阵V正交，正交化得到β'_next
            normalize(next_right_vector,N);                      //单位化β'_next



            for(int i=0;i<M;i++)
            for(int j=0;j<N;j++)
                next_left_vector[i]+=next_right_vector[j]*A[i][j]; //矩阵A(M×N)×(β'_next)^T(N×1)向量，得到左迭代向量α_next(1×M)

            r=normalize(next_left_vector,M);                       //单位化向量α_next

            if(r<eps) break;                                       //若α_next 模长太小，则退出内层循环
            for(int i=0;i<col;i++)                           
                orth(&U[i][0],next_left_vector,M);                 //与左矩阵U正交，正交化得到α'_next
            normalize(next_left_vector,M);                         //单位化α'_next

            diff=0;

            for(int i=0;i<M;i++)
            {
                double d=next_left_vector[i]-left_vector[i];      //计算前后两个迭代向量的2范数的平方，即两个向量差距的平方
                diff+=d*d;
            }

            memcpy(left_vector,next_left_vector,sizeof(double)*M);     //拷贝迭代向量的值，使得迭代过程往前进行
            memcpy(right_vector,next_right_vector,sizeof(double)*N);
        }

        if(r>=eps)
        {
            S[col]=r;                                                  //若向量的模长大于最小值eps，则取为矩阵A的奇异值

            memcpy((char *)&U[col][0],left_vector,sizeof(double)*M);   //按列拷贝左迭代向量到左奇异矩阵U中
            memcpy((char *)&V[col][0],right_vector,sizeof(double)*N);  //按列拷贝右迭代向量到右奇异矩阵V中

        }
        else break;
    }

    delete [] next_left_vector;
    delete [] next_right_vector;
    delete [] left_vector;
    delete [] right_vector;

    return true;
}
