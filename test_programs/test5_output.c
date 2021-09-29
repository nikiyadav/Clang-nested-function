#include<stdio.h>
struct foo_var {
	int* i;
};

struct main_var {
	int* arr1;
	int* j;
};


struct foo_var foo_v = { NULL };

struct main_var main_v = { NULL, NULL };

void foo ()
{
	int i=0;
  
	foo_v.i = &i;
	while(i<5)
        {         
            *(main_v.arr1 + i) = i;
            i++;
        }
  }

int main() {
    printf("This test case program demonstrate that 1D arrays are handled.\n");
    int arr1[5];
	main_v.arr1 = arr1;

    

    foo();
    printf("arr1 in main after calling foo : ");
    int j;
	main_v.j = &j;
    for ( j = 0; j < 5 ; j++ )
    {
        printf("%d ", arr1[j]);
    }
    printf("\n");
    return 0;
}