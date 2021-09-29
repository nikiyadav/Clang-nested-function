#include<stdio.h>
struct car_var {
	int* b;
};

struct foo_var {
	struct foo_point {
		int x;
		float y;
	} *q;
};

struct main_var {
	struct main_point {
		int x;
		float y;
	} *p;
	int* x;
};


struct car_var car_v = { NULL };

struct foo_var foo_v = { NULL };

struct main_var main_v = { NULL, NULL };

void car ()
{
	int b = 20;
  
	car_v.b = &b;	printf("x inside car = %d\n", (*main_v.x));
 	printf("b inside car = %d\n", b);
 }

void bar ()
{
	printf("x inside bar = %d\n", (*main_v.x));
 	printf("In bar, struct point q has x = %d and y = %f\n", (foo_v.q -> x), (foo_v.q -> y));
 	printf("In bar, Modifying struct point p, element y with value 4.5\n");
 	(main_v.p -> y) = 4.5;	car();
 }

void foo ()
{
	struct point {
            int x;
            float y;
        };

 	struct point q = {.x=12, .y=34};

 
	foo_v.q = &q;	printf("x inside foo = %d\n", (*main_v.x));
 	bar();
 	printf("Incrementing x inside foo\n");
 	(*main_v.x)++;
	printf("In foo, Modifying struct point p,element x with value 5\n");
 	(main_v.p -> x) = 5;
 }

int main() {
    printf("This test case program demonstrate that locally defined structs are handled.\n");

    struct point {
        int x;
        float y;
    };

    struct point p = {.x=1, .y=2};
	main_v.p = &p;
    printf("struct point p, elements before calling foo: p.x = %d, p.y = %f\n", p.x, p.y);

    int x = 3;
	main_v.x = &x;
    printf("x before calling foo = %d\n", x);

    

    foo();
    
    printf("x after calling foo = %d\n", x);
    printf("struct point p, elements after calling foo: p.x = %d, p.y = %f\n", p.x, p.y);
    return 0;
}