#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>
#include<linux/fb.h>
#include<fcntl.h>
#include<linux/kernel.h>
#include<linux/random.h>
#include<sys/select.h>
#include<termios.h>

#define H 20	//界面高度
#define L 20	//界面长度
#define N 10	//隐藏像素点个数。大部分电脑为0；一部分为10；还有一些是40多。只能试....
#define	 RGB888(r,g,b)	 ((r & 0xff) <<16 | (g & 0xff) << 8 | (b & 0xff))	//颜色定义
#define UP    0x415b1b
#define DOWN  0x425b1b
#define LEFT  0x445b1b
#define RIGHT 0x435b1b
#define ESC   0x1b

struct snake{
    int x;	//x,yx为蛇的坐标
    int y;
    int now;	//0，1，2，3分别表示当前向上，下，左，右移动。
    struct snake* next;
};
struct food{	//食物
    int x;
    int y;
};
static struct food* food = NULL;
static struct snake* head = NULL;
const int addx[4] = {-1,1,0,0};
const int addy[4] = {0,0,-1,1};
int t = 500000;		//设置多久走一步（微秒为单位）。
int n = 0;	//得分。
//init_keyboard,recover_keyboard
struct termios tcsave;
int flsave;
//check_over,main
int jump=1;
//show
int fb = -1;	
struct fb_var_screeninfo info; 
unsigned long* addr = NULL;
size_t len = 0;	

void show_open()
{
    fb = open("/dev/fb0", O_RDWR);
    if(fb < 0){
	perror("open err. \n");
	exit(EXIT_FAILURE);
    }

    if ( (ioctl(fb, FBIOGET_VSCREENINFO, &info)) < 0){
	perror("ioctl err. \n");
	exit(EXIT_FAILURE);
    }

    len = info.xres*info.yres*info.bits_per_pixel >> 3;	

    addr = mmap(NULL, len, PROT_WRITE|PROT_READ, MAP_SHARED, fb, 0);
    if(addr == (void*)-1){
	perror("mmap err. \n");
	exit(EXIT_FAILURE);
    }

}
void show(void)
{
    system("clear");	//清屏

    int i,j;
    //四个框。
    for(i=40; i<H*10+69;i++)
	for(j=90;j<99; j++)
	    *(addr+i*(info.xres+N)+j) = RGB888(255, 0, 0);

    for(i=40; i<49;i++)
	for(j=90;j<L*10+119; j++)
	    *(addr+i*(info.xres+N)+j) = RGB888(255, 0, 0);
    for(i=40; i<H*10+69;i++)
	for(j=L*10+110;j<L*10+119; j++)
	    *(addr+i*(info.xres+N)+j) = RGB888(255, 0, 0);
    for(i=H*10+60; i<H*10+69;i++)
	for(j=90;j<L*10+119; j++)
	    *(addr+i*(info.xres+N)+j) = RGB888(255, 0, 0);

    //食物颜色
    for(i=food->x*10+50; i<food->x*10+59;i++)
	for(j=food->y*10+100;j<food->y*10+109; j++)
	    *(addr+i*(info.xres+N)+j) = RGB888(0, 0, 255);
    //蛇身颜色
    struct snake* tmp = head;
    while(tmp!=NULL){
	for(i=tmp->x*10+50; i<tmp->x*10+59;i++)
	    for(j=tmp->y*10+100;j<tmp->y*10+109; j++)
		*(addr+i*(info.xres+N)+j) = RGB888(255, 0, 0);
	tmp = tmp->next;
    }
    //蛇头颜色
    for(i=head->x*10+50; i<head->x*10+59;i++)
	for(j=head->y*10+100;j<head->y*10+109; j++)
	    *(addr+i*(info.xres+N)+j) = RGB888(0, 255, 0);

}

void show_close()
{
    int ret = munmap(addr, len);
    if(ret < 0){
	perror("munmap err. \n");
	exit(EXIT_FAILURE);
    }
    close(fb);

}
int init_keyboard(void)
{
    int ret;
    struct termios tc;
    // 获取标准输入的属性
    ret = tcgetattr(0, &tcsave);
    if(ret < 0)
	return -1;
    // 备份tcsave 的值
    tc = tcsave;

    // 不使用标准输入输出模式，不显示字符
    tc.c_lflag &= ~(ECHO|ICANON);

    // 设置标准输入的属性
    ret = tcsetattr(0, TCSANOW, &tc);
    if(ret < 0)
	return -1;

    // 设置终端的属性为非阻塞态,否则会一直等待按键的输入，导致蛇不能自动往前走。
    //	flsave = fcntl(0, F_GETFL);
    //	fcntl(0, F_SETFL, flsave|O_NONBLOCK);
    return 0;
}

void recover_keyboard(void)
{
    // 恢复终端属性
    tcsetattr(0, TCSANOW, &tcsave);
    //	fcntl(0, F_SETFL, flsave);
}

void creat_snake(void)
{
    srand(time(NULL));
    head = malloc(sizeof(struct snake));
    head->x = rand()%L;
    head->y = rand()%H;
    head->now = 3;
    head->next = NULL;
}

void creat_food(void)
{
    if(food==NULL)
	food = malloc(sizeof(struct food));
again:
    food->x = rand()%L;
    food->y = rand()%H;
    struct snake* tmp = head;
    while(tmp!=NULL){
	if(tmp->x==food->x && tmp->y==food->y )
	    goto again;
	tmp = tmp->next;
    }
}
void move_add(void)
{
    struct snake* tmp = malloc(sizeof(struct snake));
    tmp->x = head->x + addx[head->now];
    tmp->y = head->y + addy[head->now];
    tmp->now = head->now;
    tmp->next = head;
    head = tmp;
}

void move_del(void)
{
    struct snake* tmp=head;
    struct snake* tmpp=head;
    while(tmp->next != NULL){
	tmpp = tmp;
	tmp = tmp->next;
    }
    free(tmp);
    tmpp->next = NULL;
}

void del(void)
{
    free(food);
    struct snake* tmp;
    while(head!=NULL){
	tmp = head;
	head = head->next;
	free(tmp);
    }
}

void click(void)
{
    int key;
    struct timeval time;
    fd_set fd;
    time.tv_sec=0;
    time.tv_usec=t;
    FD_SET(0,&fd);
    select(1,&fd,NULL,NULL,&time);	//等待指定时间，期间，接收到信号立刻执行下一条语句。没接收到，等待时间到后，自行执行下一条语句。（使蛇非阻塞）
    if(FD_ISSET(0,&fd)){
	read(0,&key,4);	
	switch(key){
	    case UP:
		if(head->now!=1)
		    head->now = 0;
		break;
	    case DOWN:
		if(head->now!=0)
		    head->now = 1;
		break;
	    case LEFT:
		if(head->now!=3)
		    head->now = 2;
		break;
	    case RIGHT:
		if(head->now!=2)
		    head->now = 3;
		break;
	    case ESC:
		jump = 0;
	    default:
		break;
	}	
    }		
}

void check_over(void)
{
    struct snake* tmp = head->next;
    while(tmp!=NULL){
	if(head->x==tmp->x && head->y==tmp->y){
	    jump = 0;
	    return;
	}
	tmp = tmp->next;
    }
    if(head->x<0 || head->x>L || head->y<0 ||head->y>H){
	jump = 0;
	return;
    }
}

int main()
{
    creat_snake();	//创建随机蛇头
    creat_food();	//创建随机食物

    init_keyboard();	//设置系统参数（使输入不按回车；使蛇非阻塞）
    show_open();	//打开图形文件，完成映射。
    while(jump){
	show();	//显示
	click();	//按键
	move_add();	//蛇头加一节

	if(food->x!=head->x || food->y!=head->y)	//如果没吃到食物
	    move_del();	//蛇尾剪一截
	else{
	    creat_food();	//刷新一份食物
	    t = t*4/5;	//移动等待时间变为原来的4/5
	    n++;	//加一分
	}
	check_over();	//检查蛇头是否撞墙或自吃，是的话jump置0.
    }

    del();	//释放所有空间
    show_close();	//关闭图形文件
    recover_keyboard();	//还原系统设置
    printf("score:%d\n",n);	//输出分数
    exit(0);	
}
