#ifndef __SNAKE__
#define __SNAKE__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define MAP 25

#define __DEBUG__

#ifdef __DEBUG__
//#define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %d: "format"\n", __LINE__, ##__VA_ARGS__)
#define MY_ERR(format,...) printf("FUNC:%s, LINE:%d :"format"\n", __func__, __LINE__, ##__VA_ARGS__)
#define PRINT(format,...) printf("%s: "format"\n", __func__, ##__VA_ARGS__)
#else

#define DEBUG(format, ...)

#endif

 
typedef struct node {
    char x;
    char y;
    struct node *next;
} snake_t;


void map_init();

snake_t *snake_init();













#endif
