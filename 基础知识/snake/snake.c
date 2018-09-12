/*************************************************************************
	> File Name: snake.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: 2018年09月12日 星期三 22时05分55秒
 ************************************************************************/

#include "snake.h"

snake_t *head = NULL;

void map_init()
{
    for (int i = 0; i < MAP; i++) {
	for (int j = 1; j <= MAP; j++) {
	    if (i != 0 && i != MAP-1) {
		if (j==1 || j==25)
		    printf("■ ");
		else
		    printf("  ");

		if (j % MAP == 0) {
		    printf("\n");
		}
	    } else {
		printf("■ ");
		if (j % MAP == 0)
		    printf("\n");
	    }
	}
    }
}

snake_t *snake_init()
{
    snake_t *p = NULL;
    snake_t *q = NULL;
    head = (snake_t *)malloc(sizeof(snake_t) * 1);
    if(!head) {
	MY_ERR("create head failed!");
	return NULL;
    }
    head->next = NULL;

    p = (snake_t *)malloc(sizeof(snake_t) * 1);
    if(!p) {
	MY_ERR("create body failed!");
	return NULL;
    }
    p->x = '@';
    p->y = '~';	
    head->next = p;
    p->next = NULL;

    for (int i = 0; i < 4; i++) {
	q = (snake_t *)malloc(sizeof(snake_t) * 1);
	if(!q) {
	    MY_ERR("create body failed!");
	    return NULL;
	}
	q->x = '@';
	q->y = '~';
	p->next = q;
	p = q;
    }

    p->next = NULL;

    return head;
    
}

int main(int argc, char *argv[])
{
    printf("******  ################   *******\n");
    printf("**********HELLO SNAKE*************\n");
    printf("******  ################   *******\n\n\n");
    printf("         HOW TO CONTROL           \n");
    printf("         ↑   ↓   ←  →             \n");   
    printf("   press any key to start the game\n");
    getchar();

    map_init();
    if(!snake_init()) {
	MY_ERR("snake init failed");
    }


    return 0;
}
