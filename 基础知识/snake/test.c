#include <stdio.h>  
#include <stdlib.h>  
#include <curses.h>  
#include <signal.h>  
#include <sys/time.h>  
#include <fcntl.h>  
#include <time.h>  

#define INIT_POS_X         12  
#define INIT_POS_Y         40  
#define INIT_SNAKE_LENGTH  5  
#define MOVE_LEFT          1  
#define MOVE_RIGHT         2  
#define MOVE_UP            3  
#define MOVE_DOWN          4  

struct Snake  
{  
    int pos_x;  
    int pos_y;  
    struct Snake *pNext;  
    struct Snake *pPre;  
} *pSnakeHead,*pSnakeTail,*pSnakeTrace,*pSnakeFood;  

int  move_dir;  
int  is_game_over = 0;  
int  speed[5] = {200,150,120,100,80};  
int  snake_len = INIT_SNAKE_LENGTH;  
int  current_score;  
int  levelup_score[5] = {5,10,15,20,25};  
int  level = 0;  
int  life = 3;  
void init_wall();  
void init_snake();  
void show_snake(struct Snake *pSnake);  
void snake_move(int sig);  
int  set_ticker(long n_msecs);  
void getOrder();  
int  is_snake_dead();  
void put_food();  
void eat_food();  
void levelup();  
void over(int i);  
void reset_game();  
void delete_snake();  

int main(void)  
{  
    initscr();  
    clear();  
    curs_set(0);  
    init_wall();  
    init_snake();  
    put_food();  
    set_ticker(speed[level]);  
    noecho();  
    keypad(stdscr, true);  
    getOrder();  
    system("stty echo");  
    system("clear");  
    endwin();  
    exit(0);  
}  

void init_wall()  
{  
    int i;  
    for (i = 0; i <COLS ; ++i)  
    {  
	move(3,i);  
	addstr("*");  
    }  

    move(2,0);  
    printw("life: %d", life);  
    move(2,COLS-10);  
    printw("score:%2d",current_score);  
    move(2,COLS/2-5);  
    printw("level:%d",level+1);  
    refresh();  
}  

void init_snake()  
{  
    int i;  
    struct Snake *pSnakePre;  
    int flag=0;  
    pSnakeTrace = (struct Snake *)malloc(sizeof(struct Snake));  
    pSnakeTrace->pos_x = 0;  
    pSnakeTrace->pos_y = 0;  
    for (i = 0; i < INIT_SNAKE_LENGTH; ++i)  
    {  
	struct Snake *pSnakeBody = (struct Snake *)malloc(sizeof(struct Snake));  
	pSnakeBody->pos_x = INIT_POS_X;  
	pSnakeBody->pos_y = INIT_POS_Y+INIT_SNAKE_LENGTH-i;  
	if (flag == 0)  
	{  
	    pSnakeHead = pSnakePre = pSnakeBody;  
	    pSnakeBody->pPre = NULL;  
	    flag++;  
	}  
	else  
	{  
	    pSnakePre->pNext = pSnakeBody;  
	    pSnakeBody->pPre = pSnakePre;  
	    pSnakeBody->pNext = NULL;  
	    pSnakePre = pSnakeBody;  
	}  
    }  
    pSnakeTail = pSnakePre;  
    move_dir = MOVE_RIGHT;  
    signal(SIGALRM,snake_move);  
    show_snake(pSnakeHead);  
}  

int set_ticker( long n_msecs )  
{  
    struct itimerval new_timeset;  
    long    n_sec, n_usecs;  

    n_sec = n_msecs / 1000 ;  
    n_usecs = ( n_msecs % 1000 ) * 1000L ;  

    new_timeset.it_interval.tv_sec  = n_sec;  
    new_timeset.it_interval.tv_usec = n_usecs;  
    new_timeset.it_value.tv_sec     = n_sec  ;  
    new_timeset.it_value.tv_usec    = n_usecs ;  

    return setitimer(ITIMER_REAL, &new_timeset, NULL);  
}  


void show_snake(struct Snake *pSnake)  
{  
    while(pSnake)  
    {  
	move(pSnake->pos_x,pSnake->pos_y);  
	addstr("*");  
	pSnake = pSnake->pNext;  
    }  
    move(pSnakeTrace->pos_x,pSnakeTrace->pos_y);  
    addstr(" ");  
    refresh();  
}  

void snake_move(int sig)  
{  
    noecho();  
    pSnakeTrace->pos_x = pSnakeTail->pos_x;  
    pSnakeTrace->pos_y = pSnakeTail->pos_y;  
    struct Snake *ptemp;  
    ptemp = pSnakeTail->pPre;  
    pSnakeTail->pPre->pNext = NULL;  
    pSnakeTail->pPre = NULL;  
    pSnakeTail->pNext = pSnakeHead;  
    pSnakeHead->pPre = pSnakeTail;  
    pSnakeHead = pSnakeTail;  
    pSnakeTail = ptemp;  

    switch(move_dir)  
    {  
	case MOVE_LEFT:  
	    pSnakeHead->pos_y = pSnakeHead->pNext->pos_y-1;  
	    pSnakeHead->pos_x = pSnakeHead->pNext->pos_x;  
	    break;  
	case MOVE_RIGHT:  
	    pSnakeHead->pos_y = pSnakeHead->pNext->pos_y+1;  
	    pSnakeHead->pos_x = pSnakeHead->pNext->pos_x;  
	    break;  
	case MOVE_UP:  
	    pSnakeHead->pos_x = pSnakeHead->pNext->pos_x-1;  
	    pSnakeHead->pos_y = pSnakeHead->pNext->pos_y;  
	    break;  
	case MOVE_DOWN:  
	    pSnakeHead->pos_x = pSnakeHead->pNext->pos_x+1;  
	    pSnakeHead->pos_y = pSnakeHead->pNext->pos_y;  
	    break;  
	default:  
	    break;  
    }  

    if ((is_game_over = is_snake_dead()) != 0)  
    {  
	over(is_game_over);  
    }  

    if ((pSnakeHead->pos_x == pSnakeFood->pos_x)  
	    && (pSnakeHead->pos_y == pSnakeFood->pos_y))  
    {  
	eat_food();  
	put_food();  
	current_score++;  
	move(2,COLS-10);  
	printw("score:%2d",current_score);  
    }  
    show_snake(pSnakeHead);  
    if (current_score == levelup_score[level])  
    {  
	levelup();  
    }  
    signal(SIGALRM,snake_move);  
}  

void getOrder()  
{  
    while(1)  
    {  
	char c = getch();  
	switch(c)  
	{  
	    case 'a':  
		move_dir=MOVE_LEFT;  
		break;  
	    case 'd':  
		move_dir=MOVE_RIGHT;  
		break;  
	    case 'w':  
		move_dir=MOVE_UP;  
		break;  
	    case 's':  
		move_dir=MOVE_DOWN;  
		break;  
	    case 'q':  
		system("stty echo");  
		system("clear");  
		endwin();  
		exit(0);  
		break;  
	    default:  
		break;  
	}  
    }  
}  
int is_snake_dead()  
{  
    struct Snake *pSnakeBody = pSnakeHead->pNext;  
    int k=0;  
    while(pSnakeBody)  
    {  
	if((pSnakeHead->pos_x == pSnakeBody->pos_x)  
		&& (pSnakeHead->pos_y == pSnakeBody->pos_y) )  
	{  
	    return 2;  
	}  
	pSnakeBody = pSnakeBody->pNext;  
	k++;  
    }  
    if ((pSnakeHead->pos_x == 3) || (pSnakeHead->pos_x == LINES)  
	    || (pSnakeHead->pos_y == 0) || (pSnakeHead->pos_y == COLS) )  
    {  
	return 1;  
    }  
    return 0;  
}  

void put_food()  
{  
    int food_pos_x,food_pos_y;  
    int rangex,rangey;  
    int flag=0;  
    srand(time(NULL));  
    rangex = LINES-4;  
    rangey = COLS-1;  
    struct Snake *pTemp = pSnakeHead;  
    while(1)  
    {  
	flag = 0;  
	pTemp = pSnakeHead;  
	food_pos_x = rand()%rangex + 4;  
	food_pos_y = rand()%rangey + 1;  
	while(pTemp)  
	{  
	    if ((pTemp->pos_x == food_pos_x)  
		    && (pTemp->pos_y == food_pos_y) )  
	    {  
		flag++;  
	    }  
	    pTemp = pTemp->pNext;  
	}  
	if (flag ==0)  
	{  
	    break;  
	}  
    }  
    struct Snake *pPutFood = (struct Snake *)malloc(sizeof(struct Snake));  
    pSnakeFood = pPutFood;  
    pSnakeFood->pos_x = food_pos_x;  
    pSnakeFood->pos_y = food_pos_y;  
    pSnakeFood->pPre = NULL;  
    pSnakeFood->pNext = NULL;  
    move(pSnakeFood->pos_x,pSnakeFood->pos_y);  
    addstr("0");  
}  

void eat_food()  
{  
    pSnakeFood->pNext = pSnakeHead->pNext;  
    pSnakeHead->pNext->pPre = pSnakeFood;  
    pSnakeHead->pNext = NULL;  
    pSnakeTail->pNext = pSnakeHead;  
    pSnakeHead->pPre = pSnakeTail;  
    pSnakeTail = pSnakeHead;  
    pSnakeHead = pSnakeFood;  
}  

void levelup()  
{  
    if (level == 4)  
    {  
	move(0, 1);  
	addstr("finish game ,press any key to reset game");  
	refresh();  
	set_ticker(0);  
	char c = getchar();  
	level = 0;  
	life = 3;  
	reset_game();  
    }  
    else  
    {  
	move(0, 1);  
	addstr("level up ,press any key to contiune     ");  
	refresh();  
	set_ticker(0);  
	char c = getchar();  
	level++;  
	reset_game();  
    }  
}  

void delete_snake()  
{  
    struct Snake *pSnake;  
    pSnake = pSnakeHead;  
    while(pSnake)  
    {  
	free(pSnake);  
	pSnake = pSnake->pNext;  
    }  
    free(pSnakeFood);  
}  

void reset_game()  
{  
    current_score = 0;  
    delete_snake();  
    initscr();  
    clear();  
    init_wall();  
    init_snake();  
    put_food();  
    set_ticker(speed[level]);  
}  

void over(int i)  
{  
    life--;  
    move(2,COLS/2-5);  
    printw("life: %d", life);  
    if (life == 0)  
    {  
	move(0, 1);  
	addstr("Game Over,press any key to reset game                ");  
	refresh();  
	life = 3;  
	set_ticker(0);  
	char c = getchar();  
	level = 0;  
	reset_game();  
    }  
    else  
    {  
	move(0, 1);  
	if(1 == i)  
	{  
	    addstr("Crash the wall,press any key to contiune this level");  
	}  
	else if(2 == i)  
	{  
	    addstr("Crash itself,press any key to contiune this level   ");  
	}  
	refresh();  
	set_ticker(0);  
	char c = getchar();  
	reset_game();  
    }  
}  
