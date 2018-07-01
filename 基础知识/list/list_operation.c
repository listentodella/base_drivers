/*
 * This file supplies several classic operations for list
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "list.h"

extern node_t *head;

node_t * create_list(int n)
{
    int i;
    node_t *p = NULL, *q = NULL;
    
    head = (node_t *)malloc(sizeof(node_t) * 1);
    if (head == NULL) {
        MY_ERR("create list head failed!\n");
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
            MY_ERR("create node failed!\n");
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
        MY_ERR("node's data is %d\n", p->a);
        p = p->next;
    }
}


node_t *add_to_head()
{
    node_t *p = NULL, *tmp;
    p = (node_t *)malloc(sizeof(node_t) * 1);
    if (p == NULL) {
        MY_ERR("add nodes failed\n");
        return NULL;
    }

    printf("input data for new node:");
    scanf("%d", &p->a);
    getchar();

    tmp = head->next;
    head->next = p;
    p->next = tmp;

    return head;
}

node_t *add_to_tail()
{
    node_t *p = NULL, *tmp;
    p = (node_t *)malloc(sizeof(node_t) * 1);
    if (p == NULL) {
        MY_ERR("add nodes failed\n");
        return NULL;
    }

    printf("input data for new node:");
    scanf("%d", &p->a);
    getchar();

    tmp = head->next;
    while(tmp->next) {
        tmp = tmp->next;
    }
    tmp->next = p;
    p->next = NULL;

    return head;
}


node_t *add_to_list()
{
    printf("How to add?\n");
    printf("1. add to head\n");
    printf("2. add to middle\n");
    printf("3. add to tail\n");
    printf("please make choice:");
    int a;
    scanf("%d", &a);
    getchar();

    switch(a) {
    case 1:
        if(!add_to_head()) {
            MY_ERR("add to head failed\n");
            return NULL;
        }
        print_list(head);
        break;

    case 2:
        break;

    case 3:
        if(!add_to_tail()) {
            MY_ERR("add to tail failed\n");
            return NULL;
        }
        print_list(head);
        break;
    }

    return head;
}




