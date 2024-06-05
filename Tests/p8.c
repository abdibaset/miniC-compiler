extern void print(int);
extern int read();
int func(int n)
{
    int a1;
    int a2;
    int i;
    a1 = read();
    a2 = read();
    i = 0;
    while (i < n)
    {
        int t;
        print(i);
        print(a1);
        t = a1 + a2;
        print(a1);
        a1 = a2;
        a2 = t;
        i = i + 1;
    }
    return (a1 + a2);
}
