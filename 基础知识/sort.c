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

}

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


int main(int argc, char **argv)
{
    int array[] = {1, 3, 4, 2, 6, 7, 0, 10, 12, 32, 5};
    print_array(array, ARRAY_SIZE(array));
    printf("***********************\n\n\n");

    sort_bubble(array, ARRAY_SIZE(array));
    print_array(array, ARRAY_SIZE(array));
    printf("***********************\n\n\n");

    return 0;
}
