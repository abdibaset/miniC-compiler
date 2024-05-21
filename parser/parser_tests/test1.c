extern void print(int);
extern int read();

int func(int i)
{
    int a;
    int b;

    a = read();

    if (a < i)
    {
        a = a + 1;
    }
    else
    {
        a = a + 5;
    }
    b = 10;
    return a + b;
}
