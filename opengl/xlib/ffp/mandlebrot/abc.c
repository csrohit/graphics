#include <stdint.h>
#include <stdio.h>

struct abc
{
    int a[0];
};

int main()
{
    int a[4] = {10};
    // struct abc ppp;
    int r;
    int x[0];
    int p;
    int b[4] = {12};

    x[1] = 4;
    x[2] = 6;
    printf("a = %lu\n r = %lu\n x = %lu\n p = %lu\n b = %lu\n", 
           (uint64_t)&a, 
           (uint64_t)&r,
           (uint64_t)&x,
           (uint64_t)&p, 
           (uint64_t)&b);

    printf("\na[0] = %lu\n a[3] = %lu\n x[0] = %lu\n b[0] = %lu\n b[3] = %lu\n", 
           (uint64_t)&a[0], 
           (uint64_t)&a[3], 
           (uint64_t)&x[0], 
           (uint64_t)&b[0], 
           (uint64_t)&b[3]);
    
    printf("%d %d %d %d\n", a[0], a[1], a[2], a[3]);
    printf("%d %d %d %d\n", b[0], b[1], b[2], b[3]);

    return (0);
}
