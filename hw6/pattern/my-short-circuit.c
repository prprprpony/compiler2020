int f()
{
    write("f\n");
    return 1;
}
int g()
{
    write("g\n");
    return 0;
}
float F()
{
    write("F\n");
    return 8.7;
}
float G()
{
    write("G\n");
    return 0.;
}
int MAIN()
{
    if (g() && g())
        write("ERROR\n");
    if (f() || f())
        write("OK\n");
    if (G() && G())
        write("ERROR\n");
    if (F() || F())
        write("OK\n");
    if (F() && (g() || F()))
        write("OK\n");
    if (G() && (g() || F()))
        write("ERROR\n");
    return 0;
}
