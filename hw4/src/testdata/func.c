int g(int a[][2])
{
    return 0;
}
int f(int x)
{
    return x;
}
float h(){return 2;}
int x[2][2];
int main()
{
    int a = f();
    int b = f(1,2);
    int c = f(x[0]);
    int d = g(a);
}
