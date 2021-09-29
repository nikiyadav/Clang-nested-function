#include<stdio.h>
struct main_var {
	int* x;
	int* y;
};


struct main_var main_v = { NULL, NULL };

void funcHasNoLabelStmt() {
    int x = 5;
    int i;
    for ( i=0; i<5; i++ ) {
        printf("%d ", x+i);
    }
    printf("\n");
}

void foo ()
{
	printf("x inside foo = %d\n", (*main_v.x));
 	printf("Incrementing x inside foo\n");
 	(*main_v.x)++;
}

void bar ()
{
	printf("y inside bar = %d\n", (*main_v.y));
 	printf("Incrementing y inside bar\n");
 	(*main_v.y)++;
}

int main() {
    printf("This test case program demonstrate - \n(1) multiple closures are handled\n(2) Functions without label statements remain unchanged.\n");
    int x = 3, y = 4;
	main_v.x = &x;
	main_v.y = &y;

    printf("x before calling foo = %d\n", x);
    

    
    

    foo();
    printf("x after calling foo = %d\n", x);
    printf("y before calling bar = %d\n", y);
    bar();
    printf("y after calling bar = %d\n", y);
    return 0;
}