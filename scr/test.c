#include "stdio.h"

int main(int argc, char **argv)
{
    int a[] = {1, 2, 3, 4, 5, 6};
    int i = 0;
    int *p = &i;
    for(*p; *p < 6; *p++)
        printf("%d\n", a[*p]);
    return 0;
}