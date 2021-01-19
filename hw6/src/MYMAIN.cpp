#include<iostream>
#include<cstdio>
using namespace std;
int MAIN();
void write(int x) {cout << x;}
void write(float x) {printf("%f",x);}
void write(const char * x) {cout << x;}
int read() {int x; cin >> x; return x;}
float fread() {float x; cin >> x; return x;}
int main()
{
    MAIN();
    return 0;
}
