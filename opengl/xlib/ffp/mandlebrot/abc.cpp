#include <stdio.h>
#include <stdint.h>


struct abc
{
    int a[0];
};


int main()
{
    int a[4] = {10};
    // struct abc ppp;
    int x[0];
    int b[4] = {12};

   x[0] = 1;
    x[1] = 2;
    printf("a = %lu, x = %lu, b = %lu\n", (uint64_t)&a, (uint64_t)&x, (uint64_t)&b);
    printf("%d %d %d %d\n", a[0], a[1], a[2], a[3]);
    printf("%d %d %d %d\n", b[0], b[1], b[2], b[3]);


    return (0);
}
