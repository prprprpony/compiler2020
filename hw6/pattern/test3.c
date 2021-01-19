int func(int a, int b)
{
  if (a>1){
    return a*b-3;
  }
  else{
    return a*(b-3);
  }
}

int MAIN()
{
  int a=1,b=2;
  int result;
 
  write(a); 
  write("\n"); 
  write(b); 
  write("\n"); 
  result=func(a,b)-3;
  
  write(result);
  write("\n");
  return 0;
}

