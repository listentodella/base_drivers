/*************************************************************************
	> File Name: sort.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: 2018年07月03日 星期二 22时18分06秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

void print_array(int a[], int size)
{
    int i;
    for (i = 0; i < size; i++) {
        printf("a[%d] = %d\n", i, a[i]);
    }

    printf("***********************\n\n\n");
}

#if 0
void sort_bubble(int a[], int size)
{
    int i, j, tmp;
    for (i = 0; i < size; i++) {
        for (j = i; j < size; j++) {
            if (a[i] > a[j]) {
                tmp = a[i];
                a[i] = a[j];
                a[j] = tmp;
            }
        }
    }

}
#endif

void sort_bubble(int a[], const int size)
{
    int i, j, tmp;
    //in fact, sort for (n - 1) times is OK too, because (n - 1) already sort all
    //for (i = 0; i < size; i++) {
    for (i = 0; i < size - 1; i++) {
        for (j = 0; j < size - i - 1; j++) {
            if (a[j] > a[j+1]) {
                tmp = a[j];
                a[j] = a[j+1];
                a[j+1] = tmp; 
            }
        }
    }
}


void sort_bubble_v2(int a[], const int size)
{
    int left = 0, right = size - 1;
    int i, j, tmp;
    while (left < right) {
        // traversing from left to right, 
        // and move the max to rightmost
        for (i = left; i < right; i++) {
            if (a[i] > a[i+1]) {
                tmp = a[i+1];
                a[i+1] = a[i];
                a[i] = tmp;
            }
        }

        right--;

        // traversing from right to left, 
        // and move the min to leftmost
        for (j = right; j >= left; j--) {
            if (a[j+1] < a[j]) {
                tmp = a[j+1];
                a[j+1] = a[j];
                a[j] = tmp;
            }
        }

        left++;
    }
    
}

void sort_select(int a[], const int size)
{
    int i, j, k, tmp;
    for (i = 0; i < size; i++) {
        k = i;
        for (j = i + 1; j < size; j++) {
            if (a[j] < a[k]) {
                k = j;
            }
        }

        if (k != i) {
            tmp = a[k];
            a[k] = a[i];
            a[i] = tmp;
        }
    } 
}



int main(int argc, char **argv)
{
    int array[] = {1, 3, 4, 2, 6, 7, 0, 10, 12, 32, 5};
    print_array(array, ARRAY_SIZE(array));

    //sort_bubble(array, ARRAY_SIZE(array));
    //sort_bubble_v2(array, ARRAY_SIZE(array));
    sort_select(array, ARRAY_SIZE(array));
    print_array(array, ARRAY_SIZE(array));

    return 0;
}
