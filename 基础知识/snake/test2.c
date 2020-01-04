#include <iostream>
#include <cstdio>
#include <cstring>
#include <curses.h>//graph lib
#include <unistd.h>//usleep
#include <pthread.h>// mul process
#include <time.h>
#include <cstdlib>
const int DMENU=1;//the manu status
const int DGAME=2;//start game
const int MENU_START=1;
const int MENU_EXIT=2;
const int MENU_ABOUT=3;
const int HEAD_LEFT=1;
const int HEAD_DOWN=2;
const int HEAD_UP=3;
const int HEAD_RIGHT=4;
using namespace std;
struct curPoint
{
    int x, y;   
};
struct MainSnack
{
    curPoint body[10000]    ;
    int len, arrowHead, score, dead;
};
char StrMenuOption[10][255];
int MenuTotalLine;
int MenuChosed;
pthread_t Pid;
MainSnack Ms;
void xcInit();//Initial for the program
void msgDeal(int status);//The process to deal key msg
void xcDrawMenu(int chose);//Draw the menu
void xcDrawAbout();//Draw the about window
void initGame();//Initial the variable for the game
void *GameProcess(void *arg);
int main()
{
    xcInit();
    endwin();
    return 0;
}
void *GameProcess(void *arg)
{
    int cnt=0, i, j;
    curPoint food;
    clear();
    //food.y = rand()%(LINES-1)+1;
    //food.x = rand()%(COLS-1)+1;
    food.y = 0;
    food.x = 0;
    while(1)    
    {
        curPoint tmp;
        if(food.y == 0)
        {
            food.y = rand()%(LINES-1)+1;
            food.x = rand()%(COLS-1)+1;     
            move(food.y, food.x);
            addch('O');
        }
        box(stdscr, ACS_VLINE, ACS_HLINE);
        if(cnt==5)  
        {
            move(Ms.body[Ms.len].y, Ms.body[Ms.len].x);
            tmp.x=Ms.body[Ms.len].x;
            tmp.y=Ms.body[Ms.len].y;
            addch(' ');
            for(i=Ms.len-1; i>=1; i--)
            {
                Ms.body[i+1].x = Ms.body[i].x;
                Ms.body[i+1].y = Ms.body[i].y;
            }
            switch(Ms.arrowHead)
            {
                case HEAD_RIGHT:
                    Ms.body[1].x++;
                    break;
                case HEAD_LEFT:
                    Ms.body[1].x--;
                    break;
                case HEAD_UP:
                    Ms.body[1].y--;
                    break;
                case HEAD_DOWN:
                    Ms.body[1].y++;
                    break;
            }
            if(Ms.body[1].x == food.x && Ms.body[1].y == food.y)
            {
                food.y=0;
                Ms.len++;
            }
            if(Ms.body[1].x == 0 || Ms.body[1].x == COLS-1 || Ms.body[1].y == 0 || Ms.body[1].y == LINES-1)
                Ms.dead=1;
            for(i=1; i<=Ms.len; i++)
            {
                for(j=i+1; j<=Ms.len; j++)
                {
                    if(Ms.body[i].x == Ms.body[j].x && Ms.body[i].y == Ms.body[j].y)
                    {
                        Ms.dead=1;
                        i=Ms.len;
                        break;
                    }
                }
            }
            cnt=0;
        }

        if(1==Ms.dead)
        {
            move(tmp.y, tmp.x);
            addch('*');
            refresh();
            return NULL;
        }else
        {
            for(i=1; i<=Ms.len; i++)
            {
                move(Ms.body[i].y, Ms.body[i].x);
                if(1==i)
                    addch('@');
                else
                    addch('*');

            }
            refresh();
        }
        cnt++;
        usleep(15*1000);
    }
}
void initGame()
{
    curPoint tmp;
    tmp.x = COLS/2;
    tmp.y = LINES/2;
    Ms.len = 4;
    for(int i=1; i<=Ms.len; i++)
    {
        Ms.body[i].x = tmp.x-(i-1);
        Ms.body[i].y = tmp.y;
    }
    Ms.arrowHead = HEAD_RIGHT;
    Ms.score = 0;
    Ms.dead = 0;
}
void msgDeal(int status)
{
    int ch;
    while(1)
    {
        if(status == DMENU)
        {
            ch = getch();
            switch(ch)
            {
                case KEY_UP:
                    if(MenuChosed > 1)
                        MenuChosed--;
                    break;
                case KEY_DOWN:
                    if(MenuChosed < MenuTotalLine)
                        MenuChosed++;
                    break;  
                case '\n':
                    switch(MenuChosed)
                    {
                        case MENU_EXIT:
                            return;
                            break;
                        case MENU_ABOUT:
                            clear();
                            xcDrawAbout();
                            break;
                        case MENU_START:
                            clear();
                            initGame();
                            srand(time(NULL));
                            pthread_create(&Pid, NULL, GameProcess, NULL);
                            //GameProcess();//mul process
                            msgDeal(DGAME);
                            //getch();
                            //return;
                            break;
                    }
                    break;
            }
            xcDrawMenu(MenuChosed);
        }else if(status==DGAME)
        {
            ch=getch();
            if(1==Ms.dead)
                return;
            switch(ch)
            {
                case KEY_UP:
                    if(Ms.arrowHead!=HEAD_DOWN)
                        Ms.arrowHead=HEAD_UP;
                    break;
                case KEY_DOWN:
                    if(Ms.arrowHead != HEAD_UP)
                        Ms.arrowHead=HEAD_DOWN;
                    break;
                case KEY_LEFT:
                    if(Ms.arrowHead != HEAD_RIGHT)
                        Ms.arrowHead=HEAD_LEFT;
                    break;
                case KEY_RIGHT:
                    if(Ms.arrowHead != HEAD_LEFT)
                        Ms.arrowHead=HEAD_RIGHT;
                    break;
            }
        }
    }
}
void xcInit()
{
    strcpy(StrMenuOption[1],"Start");
    strcpy(StrMenuOption[2],"Exit");
    strcpy(StrMenuOption[3],"About");
    MenuTotalLine=3;
    MenuChosed=MENU_START;
    initscr();//begin curses mode
    curs_set(0);
    noecho();//not echo the character that user input
    keypad(stdscr, TRUE);//accep arrow key
    xcDrawMenu(MENU_START);//draw the menu
    msgDeal(DMENU);//like windows msg deal
}
void xcDrawMenu(int chose)
{
    int i;
    char info[]="Snack for superxc V0.8 20151210";
    clear();
    box(stdscr, ACS_VLINE, ACS_HLINE);//draw the border line
    curPoint tmpPos;
    tmpPos.y = LINES/2-MenuTotalLine/2;
    tmpPos.x = COLS/2;
    move(tmpPos.y/2, tmpPos.x-strlen(info)/2);
    waddstr(stdscr, info);
    for(i=1; i<=MenuTotalLine; i++)
    {
        if(i==chose)
            attron(A_REVERSE);
        move(tmpPos.y+i-1, tmpPos.x-strlen(StrMenuOption[i])/2);
        waddstr(stdscr, StrMenuOption[i]);
        if(i==chose)            
            attroff(A_REVERSE);

    }

    refresh();  
}
void xcDrawAbout()
{
    char info[15][255];
    box(stdscr, ACS_VLINE, ACS_HLINE);
    strcpy(info[1], "                                                                             ");
    strcpy(info[2], "                                                                             ");
    strcpy(info[3], "  mmm   m   m  mmmm    mmm    m mm  m   m   mmm           mmm    mmm   mmmmm ");
    strcpy(info[4], " #   \"  #   #  #\" \"#  #\"  #   #\"  \"  #m#   #\"  \"         #\"  \"  #\" \"#  # # # ");
    strcpy(info[5], "  \"\"\"m  #   #  #   #  #\"\"\"\"   #      m#m   #             #      #   #  # # # ");
    strcpy(info[6], " \"mmm\"  \"mm\"#  ##m#\"  \"#mm\"   #     m\" \"m  \"#mm\"    #    \"#mm\"  \"#m#\"  # # # ");
    strcpy(info[7], "               #                                                             ");
    strcpy(info[8], "               \"                                                             ");
    strcpy(info[9], "Develop by superxc, Copyright 2011-2015.");
    for(int i=1; i<=9; i++)
    {
        move(i, 1);
        waddstr(stdscr, info[i]);
    }
    refresh();
    getch();
}