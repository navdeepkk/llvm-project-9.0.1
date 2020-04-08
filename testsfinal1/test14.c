/*
This test case implements recursive insertion sort.
i_sort() is a nested function which sorts array.

Expected output:
5 11 12 13 327
*/


#include<stdio.h>

int main()
{
	int arr[5] = {12, 11, 13, 5, 327};

   int n = 5;
	int i;

   i_sort:{
				if(n<=1)
				return;

				n--;
				i_sort();
				n++;

            int last = arr[n-1];
            int j = n - 2;

				while((j>=0) && arr[j] > last)
				{
					arr[j+1] = arr[j];
					j--;
				}
				arr[j+1] = last;
			}

    i_sort();
	 for(i=0;i<5;i++)
	 {
		printf("%d ", arr[i]);
    }
    printf("\n");
    return 0;
}
