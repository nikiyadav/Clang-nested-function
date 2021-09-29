#include<stdio.h>

void funcHasNoLabelStmt() {
    int x = 5;
    int i;
    for ( i=0; i<5; i++ ) {
        printf("%d ", x+i);
    }
    printf("\n");
}

int main() {
    printf("This test case program demonstrate - \n(1) multiple closures are handled\n(2) Functions without label statements remain unchanged.\n");
    int x = 3, y = 4;

    printf("x before calling foo = %d\n", x);
    

    foo: {
        printf("x inside foo = %d\n", x);
        printf("Incrementing x inside foo\n");
        x++;
    }
    bar: {
        printf("y inside bar = %d\n", y);
        printf("Incrementing y inside bar\n");
        y++;
    }

    foo();
    printf("x after calling foo = %d\n", x);
    printf("y before calling bar = %d\n", y);
    bar();
    printf("y after calling bar = %d\n", y);
    return 0;
}