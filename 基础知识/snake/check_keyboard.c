/*************************************************************************
	> File Name: check_keyboard.c
	> Author: H233
	> Mail: 937138688@qq.com
	> Created Time: 2018年09月12日 星期三 23时25分47秒
 ************************************************************************/

#include <stdio.h>
#include <string.h>

int main(void)
{
    char key;
    key = getchar();
    getchar();
    while(key) {

	printf("%d, 0x%x\n", key, key);
    key = getchar();
    getchar();
    }

    return 0;
}
