#include<stdio.h>

int main() {
    printf("This test case program demonstrate that 1D arrays are handled.\n");
    int arr1[5];

    foo : {
        int i=0;
        while(i<5)
        {
            arr1[i] = i;
            i++;
        }
    }

    foo();
    printf("arr1 in main after calling foo : ");
    int j;
    for ( j = 0; j < 5 ; j++ )
    {
        printf("%d ", arr1[j]);
    }
    printf("\n");
    return 0;
}