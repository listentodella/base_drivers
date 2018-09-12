/*************************************************************************
	> File Name: check_keyboard.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: 2018年09月12日 星期三 23时25分47秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define     ESC     "\033"
#define     UP      "\033[A"
#define     DOWN    "\033[B"
#define     LEFT    "\033[D"
#define     RIGHT   "\033[C"

int main(void)
{
    char *key = (char *)malloc(sizeof(char));

    printf("start track keyboard\n");
    while(1) {
        fgets(key, 10, stdin);
         /*用fgets()函数从stdin中读取字符串时，会自动在字符串末尾追加"\n"，这里将末尾字符改为"\0"    */
        key[strlen(key) - 1] = '\0';
        printf("%d, 0x%x\n", *key, *key);
        //printf("%s\n", key);
    }

    return 0;
}
