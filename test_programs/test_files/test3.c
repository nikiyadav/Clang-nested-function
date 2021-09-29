#include<stdio.h>

int x = 100;

void printHello() {
    printf("Following statements use global definition of x as there is no local definition.\n");
    printf("Hello from printHello()! %d\n", x);
    war: {
        printf("Hello from car()! %d\n", x);
        printf("Hello from car()! %d\n", x++);
    }
    war();
    printf("Hello from printHello()! %d\n", x);
}


void func(int x, int y) {
    printf("Following statements use local definition of x, not global\n");
    printf("x before calling foo = %d\n", x);

    foo: {
        float a = 5;
        int b = 7;
        printf("x inside foo = %d\n", x);
        printf("a inside foo = %f\n", a);
        printf("y inside foo = %d\n", y);
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
}

int main() {
    printf("This test case program demonstrate that local and global variables are handled according to static scoping rules.\n");
    
    int x = 3, y = 4;
    func(x,y);
    printHello();
    return 0;
}