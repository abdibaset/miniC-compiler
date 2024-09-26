extern void print(int);
extern int read();

int func(int p) {
    int a;
    a = p + 10;

    if (p > 0) {
        int a;
        int i;
        a = 2;
        i = 0;

        while (i < p) i = i + a;
        if (i < 5) {
            int a;
            a = i;
        }
        print(a);
    } else {
        int a;
        a = -1 * p;
        while (i < a) {
            i = i + 1;
        }
    }

    return a;
}