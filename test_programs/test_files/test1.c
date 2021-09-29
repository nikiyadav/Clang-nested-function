#include<stdio.h>

int main() {
    printf("This test case program demonstrate recursive nested function blocks.\nThe static scoping rules are met and can be verified with print statements.\n");

    int x = 3;
    printf("x before calling foo = %d\n", x);

    foo: {
        float a = 5;
        int b = 7;
        printf("x inside foo = %d\n", x);
        printf("a inside foo = %f\n", a);
        bar: {
            float a = 10;
            printf("x inside bar = %d\n", x);
            printf("a inside bar = %f\n", a);
            printf("b inside bar = %d\n", b);
            car: {
                int b = 20;
                printf("x inside car = %d\n", x);
                printf("a inside car = %f\n", a);
                printf("b inside car = %d\n", b);
            }
            car();
        }
        bar();
        printf("Incrementing x inside foo\n");
        x++;
    }

    foo();
    printf("x after calling foo = %d\n", x);
    return 0;
}