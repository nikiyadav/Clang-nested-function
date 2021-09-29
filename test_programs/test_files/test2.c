#include<stdio.h>

int main() {
    printf("This test case program demonstrate that locally defined structs are handled.\n");

    struct point {
        int x;
        float y;
    };

    struct point p = {.x=1, .y=2};
    printf("struct point p, elements before calling foo: p.x = %d, p.y = %f\n", p.x, p.y);

    int x = 3;
    printf("x before calling foo = %d\n", x);

    foo: {
        struct point {
            int x;
            float y;
        };

        struct point q = {.x=12, .y=34};

        printf("x inside foo = %d\n", x);
        bar: {
            printf("x inside bar = %d\n", x);
            printf("In bar, struct point q has x = %d and y = %f\n", q.x, q.y);
            printf("In bar, Modifying struct point p, element y with value 4.5\n");
            p.y = 4.5;
            car: {
                int b = 20;
                printf("x inside car = %d\n", x);
                printf("b inside car = %d\n", b);
            }
            car();
        }
        bar();
        printf("Incrementing x inside foo\n");
        x++;
        printf("In foo, Modifying struct point p,element x with value 5\n");
        p.x = 5;
    }

    foo();
    
    printf("x after calling foo = %d\n", x);
    printf("struct point p, elements after calling foo: p.x = %d, p.y = %f\n", p.x, p.y);
    return 0;
}