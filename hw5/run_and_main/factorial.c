int n;
int fact() 
{
    int ret;
    if (n == 1) {
        return n;
    } else {
        n =n-1;
        ret = n;
        ret = ret * fact();
        return ret;
    }
}
int MAIN()
{
    int result;
    write("Enter a number:");

    n = read();
    n = n+1;
    if (n > 1) { 
        result = fact();
    } else {
        result = 1;
    }
    write("The factorial is ");
    write(result);
    write("\n");
}
