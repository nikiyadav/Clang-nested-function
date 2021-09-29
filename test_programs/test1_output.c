#include<stdio.h>
struct bar_var {
	float* a;
};

struct car_var {
	int* b;
};

struct foo_var {
	float* a;
	int* b;
};

struct main_var {
	int* x;
};


struct bar_var bar_v = { NULL };

struct car_var car_v = { NULL };

struct foo_var foo_v = { NULL, NULL };

struct main_var main_v = { NULL };

void car ()
{
	int b = 20;
  
	car_v.b = &b;	printf("x inside car = %d\n", (*main_v.x));
 	printf("a inside car = %f\n", (*bar_v.a));
 	printf("b inside car = %d\n", b);
 }

void bar ()
{
	float a = 10;
  
	bar_v.a = &a;	printf("x inside bar = %d\n", (*main_v.x));
 	printf("a inside bar = %f\n", a);
 	printf("b inside bar = %d\n", (*foo_v.b));
 	car();
 }

void foo ()
{
	float a = 5;
  
	foo_v.a = &a;	int b = 7;
  
	foo_v.b = &b;	printf("x inside foo = %d\n", (*main_v.x));
 	printf("a inside foo = %f\n", a);
 	bar();
 	printf("Incrementing x inside foo\n");
 	(*main_v.x)++;
}

int main() {
    printf("This test case program demonstrate recursive nested function blocks.\nThe static scoping rules are met and can be verified with print statements.\n");

    int x = 3;
	main_v.x = &x;
    printf("x before calling foo = %d\n", x);

    

    foo();
    printf("x after calling foo = %d\n", x);
    return 0;
}