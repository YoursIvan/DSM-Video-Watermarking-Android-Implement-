#include <algorithm>
#include <iostream>
#include <vector>

#include <string.h>
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <math.h>
#include <cstdlib>
using namespace std;

class Polynom
{
public:
    string p1;

public: Polynom(string p)
    {
        p1 = p;
    }

public:Polynom operator* (Polynom poly2)
    {
        string p2 = poly2.p1;
        int s1 = p1.length();
        int s2 = p2.length();
        int sr = p1.length() + p2.length() - 1;
        string result(sr,'0');
        replace(result.begin(),result.end(),'\0','0');

        for (int i = 0; i < s1; i++)
        {
            for (int j = 0; j < s2; j++)
            {
                char mul = Mul(p1[s1-i-1], p2[s2-j-1]);
                int f = i + j;
                result[sr-f-1] = Plus(result[sr-f-1], mul);
            }
        }
        return Polynom(result);
    }

    Polynom operator / (Polynom &poly2)
    {
        string p2 = poly2.p1;
        int s1 = p1.length();
        int s2 = p2.length();
        string result = p1;
        Polynom realResult("");
        while (result.length() >= s2)
        {
            int f = result.length() - (s2-1);
            string a(f,'0');
            replace(a.begin(),a.end(),'\0','0');
            a[0] = '1';
            Polynom ap(a);
            realResult = realResult + ap;
            Polynom m = poly2 * ap;
            result = (Polynom(result) + m).p1;
        }
        return realResult;
    }

    Polynom operator % (Polynom &poly2)
    {
        string p2 = poly2.p1;
        int s1 = p1.length();
        int s2 = p2.length();
        string result = p1;
        Polynom realResult("");
        while (result.length() >= s2)
        {
            int f = result.length() - (s2 - 1);
            string a(f,'0');
            replace(a.begin(),a.end(),'\0','0');
            a[0] = '1';
            Polynom ap(a);
            realResult = realResult + ap;
            Polynom m = poly2 * ap;
            result = (Polynom(result) + m).p1;
        }
        string sresult = result;
        if (sresult.length() == 0) sresult = "0";
        return Polynom(result);
    }

    Polynom operator + (Polynom &poly2)
    {
        string p2 = poly2.p1;
        int s1 = p1.length();
        int s2 = p2.length();
        if (s1 > s2)
        {
            p2 = MakeString(s1 - s2) + p2;
            s2 = s1;
        }
        if (s2 > s1)
        {
            p1 = MakeString(s2 - s1) + p1;
            s1 = s2;
        }
        string r(s1,'0');
        for (int i = 0; i < s1; i++)
        {
            r[i] = Plus(p1[i], p2[i]);
        }
        int index = 0;
        for (; index < s1; index++)
        {
            if (r[index] != '0') break;
        }
        r.erase(0, index);
        return Polynom(r);
    }

    string MakeString(int len)
    {
        string r(len,'0');
        replace(r.begin(),r.end(),'\0','0');
        return r;
    }

    static char Plus(char x, char y)
    {
        if (x != y) return '1';
        else return '0';
    }

    static char Mul(char x, char y)
    {
        if (x == '0' || y == '0') return '0';
        else return '1';
    }

};



/********************  BCH(15,5,3)   *********************/
static int OnesCount(string s)
{
    int sum = 0;
    for (int i = 0; i < s.length(); ++i) {
        if (s[i] == '1') sum++;
    }

    return sum;
}

static string Bch_Encoding(string MessageStr, string GenPolynom)
{
    Polynom p1(MessageStr);
    Polynom p2(GenPolynom);
    string s1 = p1.p1;
    string s2 = p2.p1;
    string s3(s2.length(),'0');
    s3[0] = '1';
    p1 = p1 * Polynom(s3);
    Polynom pm =p1%p2;
    p1 = p1 + pm;
    return p1.p1;
}

static string Bch_Correcting(string CodedMessage, string GenPolynom, int &flag)
{
    Polynom p1(CodedMessage);
    int n1 = (p1.p1).length();
    Polynom p2(GenPolynom);
    {
        string remainder = (p1% p2).p1;
    }
    int shifts = 0;
    int CurruntCount = 1;
    while (true)
    {
        Polynom polyRemainder = p1%p2;
        string remainder = polyRemainder.p1;
        if (OnesCount(remainder) > 3)
        {
            string s = p1.p1;
            s = s.substr(1) + s[0];
            p1 = Polynom(s);
            shifts++;
            if (CurruntCount > 100)
            {
                flag = 1;
                break;
            }
        }
        else
        {
            p1 = p1 + polyRemainder;
            break;
        }
        CurruntCount += 1;
    }

    for (int i = 0; i < shifts; i++)
    {
        string s = p1.p1;
        if (s.length() < n1)
        {
            while (s.length() < n1)
            {
                s = '0' + s;
            }
        }
        s = s[s.length() - 1] + s.substr(0, s.length() - 1);
        p1 = Polynom(s);
    }
    return p1.p1;
}

static string Bch_Decoding(string CorrectMessage)
{
    Polynom p1(CorrectMessage);
    string s1 = p1.p1;
    return s1.substr(0, 5);
}


