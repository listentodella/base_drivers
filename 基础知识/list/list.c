/*************************************************************************
	> File Name: list.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: 2018年06月25日 21:57:35
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"


node_t *head = NULL;


int main(int argc, char **argv)
{
    printf("*****This is base list*****\n");
    printf("please input nodes nums:");
    int n;
    scanf("%d", &n);
    getchar();

    
    node_t *res = create_list(n);
    if (res == NULL) {
        MY_ERR("failed to create list...\n");
        return -1;
    }

    print_list(res);
    printf("list size is %d\n", list_size());
    res = add_to_list();
    if (res == NULL) {
        MY_ERR("failed to add node to list...\n");
        return -1;
    }

    return 0;
}

