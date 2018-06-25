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


node_t * create_list(int n)
{
    int i;
    node_t *p = NULL, *q = NULL;
    
    head = (node_t *)malloc(sizeof(node_t) * 1);
    if (head == NULL) {
        printf("create list head failed!\n");
        return NULL;
    }
    
    p = (node_t *)malloc(sizeof(node_t) * 1);
    if (p == NULL) {
        printf("create node failed!\n");
        return NULL;
    }
    printf("please input the data for this node:");
    scanf("%d", &(p->a));
    getchar();
    head->next = p;

    for(i = 0; i < n - 1; i++) {
        q = (node_t *)malloc(sizeof(node_t) * 1);
        if (q == NULL) {
            printf("create node failed!\n");
            return NULL;
        }

        printf("please input the data for this node:");
        scanf("%d", &(q->a));
        getchar();

        p->next = q;
        p = q;
    }

    p->next = NULL;
    
    return head;
}

void print_list(node_t *head)
{
    node_t *p = head->next;
    while (p) {
        printf("node's data is %d\n", p->a);
        p = p->next;
    }
}


int main(int argc, char **argv)
{
    printf("*****This is base list*****\n");
    printf("please input nodes nums:");
    int n;
    scanf("%d", &n);
    getchar();

    
    node_t *res = create_list(n);
    if (res == NULL) {
        printf("failed to create list...\n");
        return -1;
    }

    print_list(res);


    return 0;
}

