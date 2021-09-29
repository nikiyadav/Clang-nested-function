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

struct func_var {
	int* x;
	int* y;
};


struct bar_var bar_v = { NULL };

struct car_var car_v = { NULL };

struct foo_var foo_v = { NULL, NULL };

struct func_var func_v = { NULL, NULL };

int x = 100;

void war ()
{
	printf("Hello from car()! %d\n", x);
 	printf("Hello from car()! %d\n", x++);
 }

void printHello() {
    printf("Following statements use global definition of x as there is no local definition.\n");
    printf("Hello from printHello()! %d\n", x);
    
    war();
    printf("Hello from printHello()! %d\n", x);
}


void car ()
{
	int b = 20;
  
	car_v.b = &b;	printf("x inside car = %d\n", (*func_v.x));
 	printf("a inside car = %f\n", (*bar_v.a));
 	printf("b inside car = %d\n", b);
 }

void bar ()
{
	float a = 10;
  
	bar_v.a = &a;	printf("x inside bar = %d\n", (*func_v.x));
 	printf("a inside bar = %f\n", a);
 	printf("b inside bar = %d\n", (*foo_v.b));
 	car();
 }

void foo ()
{
	float a = 5;
  
	foo_v.a = &a;	int b = 7;
  
	foo_v.b = &b;	printf("x inside foo = %d\n", (*func_v.x));
 	printf("a inside foo = %f\n", a);
 	printf("y inside foo = %d\n", (*func_v.y));
 	bar();
 	printf("Incrementing x inside foo\n");
 	(*func_v.x)++;
}

void func(int x, int y) {
    	func_v.x = &x;
    	func_v.y = &y;
    printf("Following statements use local definition of x, not global\n");
    printf("x before calling foo = %d\n", x);

    

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