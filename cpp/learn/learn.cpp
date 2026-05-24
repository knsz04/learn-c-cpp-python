#include <bits/stdc++.h>
using namespace std;


 double lengths(int s[], int t[])
        {
            return sqrt(pow(s[0] - t[0], 2) + pow(s[1] - t[1], 2));
        }   
        double sss(int s[],int t[],int q[])
        {
            double p = (lengths(s,t) + lengths(t,q) + lengths(q,s)) / 2;
            return sqrt(p * (p - lengths(s,t)) * (p - lengths(t,q)) * (p - lengths(q,s)));
        }
int main(){


   /* int a,b;
    cin >> a >> b;
    cout << a << b << endl;*/
    

    // float a = 2345678900.001;
    // float b = 3;
    // cout << a << endl;
    // float c = a + b;
    // cout << fixed << c << endl;
    // return 0;


    // int a = 2147483647;
    // //a += 1;
    // cout << a << endl;

    
    // const int a = 10;
    // cout << a << endl;


    // int a = 65;
    // cout << (char)a << endl;


    // char a , b , c;
    // cin >> a >> b >> c;
    // a = a + 32;
    // b = b + 32;
    // c = c + 32;
    // cout << a << b << c << endl;

    
    // cout << "He said: \"The symbol is \'\\\'.\""<< endl;


    // string s = "23453(Mary)24565";
    // s.erase(0,6);
    // s.erase(4,6);
    // cout << s << endl;


//     string s = "abcdefg";
    
//     cout << string::npos << endl;
    
//     return 0;

    // float a = 1.0;
    // cout << setprecision(5) << fixed << a << endl;
        // int a = 3, b = 4, c = 5;
        // int p = (a + b + c) / 2;
        // cout << sqrt(p * (p - a) * (p - b) * (p - c)) << endl;

       
        int s[] = {0,0}, t[] = {3,0}, q[] = {0,4};

        cout << sss(s,t,q) << endl;

        

 }