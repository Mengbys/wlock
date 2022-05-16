#include <stdio.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <wchar.h>
#include "ui.h"

// use for the time line
static char weekday[7][5] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

static char pic[200][200] =
{
    "⣿⣿⣿⣿⣿⠋⠁⠀⠈⣿⠸⠁⢸⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠈⠐⠀⣿⣯⠇⠙⠛⠛⠀⠄⠠⠆⠀⢄⠂⠀⠀⢠⣥⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
    "⢿⣿⣿⣿⣿⠀⠈⠛⠿⣿⠀⠀⣾⣿⣿⣿⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠀⠈⠁⢀⠀⠀⠀⠀⠀⠀⠐⠺⠄⠑⠂⠀⠀⠙⠻⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
    "⠀⠙⠿⣯⣴⣶⣶⣤⣀⠀⠀⠘⣿⣿⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⠀⠀⠙⠲⢌⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
    "⠀⠀⠀⠈⢻⣿⣿⣿⣿⣿⣦⡀⠹⣿⣿⣿⣿⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠱⡀⠀⠀⠀⠁⠀⢹⣿⣿⣿⣿⣿⣿⣿⣿",
    "⠀⠀⠀⠀⠀⠘⢿⣿⣿⣿⣿⣿⣄⠹⣿⠟⠛⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡇⠀⠀⠀⠀⠀⠀⡗⠀⠀⠄⠀⣀⣼⣿⣿⣿⣿⣿⣿⣿⣿",
    "⠀⠀⠀⠀⠀⠀⠀⠘⢿⣿⣿⣿⠟⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠸⠃⠀⠀⠀⠀⢀⠀⠀⠀⠀⢀⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣀⣤⣤⡄⠀⠀⠀⣀⠀⠀⠀⠀⠀⠐⠒⠒⠊⠁⢀⣴⡄⢰⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣤⣆⠙⠙⠉⢻⣿⣿⣿⣦⡀⠀⠐⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⣿⣷⠀⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
    "⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣀⣄⣴⣿⠅⣿⣿⣧⣼⣤⣤⣾⣷⣿⣿⣿⣄⠀⠐⠄⠀⠀⠀⠀⠀⠀⠀⠀⠀⣿⣿⣧⠈⠻⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿",
    "⡀⠀⠀⠀⣠⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣠⣎⠛⢚⠿⢧⣀⣨⣽⣯⣿⡛⠿⠟⠛⢉⣉⣤⣼⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢻⣿⣿⡄⠀⠙⢻⣿⣿⣿⣿⣿⣿⣿⣿",
    "⡿⡴⣠⣾⡏⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣴⣿⣿⣧⣶⡄⠲⣿⣿⣿⣿⣻⠟⠁⢀⠀⠀⠙⠛⠛⠛⠉⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⡄⣸⣿⣿⣇⠀⠀⠀⠈⠙⠛⠻⠿⣿⣿⣿",
    "⡇⣾⣿⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣎⠙⢿⢿⣿⣿⣦⣄⠀⠙⠛⠟⠁⠠⢾⡿⣿⢂⡈⠛⢶⡖⢶⠶⠀⠀⠀⠀⠀⠀⠀⠀⢠⡇⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣿⣿⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢺⣿⠿⣷⡀⠻⣿⣿⣿⣿⣿⣦⡈⠶⣦⣤⣄⣉⣉⡉⠁⠀⠀⠀⠀⠀⣀⡀⠀⠀⠀⠀⠀⢀⣿⡷⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣴⣧⡀⠻⣿⣿⣿⣦⡙⠻⣿⣿⣿⣿⣿⣦⡈⠻⣿⣿⡟⠘⠁⠀⠐⢀⡀⠀⠘⡧⠀⠀⠀⠀⠀⠻⠟⠰⠿⢿⡿⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠘⠻",
    "⠇⠀⠀⠀⠀⣀⠀⠀⠀⠀⠀⣰⣿⣿⣇⠀⠈⠻⠛⠋⣁⣠⡈⠉⠛⠻⠿⣿⣿⣦⡈⠻⡟⢀⣶⠀⡀⠺⣿⣦⠄⢃⣷⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⢿⡇⠀⠀⠀⡀⢸⣿⣿⣿⠀⢈⣤⡀⠙⠻⢝⠛⠳⡀⠲⢤⣤⣄⣀⡀⠀⠀⢙⣻⣷⣄⡙⢋⣥⣾⣿⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⠀⠀⠀⠀⠀⢸⣷⠀⠀⠀⣿⡄⢿⣿⢋⠀⣸⣿⣿⣶⣄⡀⠈⠐⠦⠀⠀⠈⠛⠿⠿⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⢁⣼⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠻⢶",
    "⡀⠀⠀⠀⠀⠀⡿⠆⠀⠀⢿⣷⠈⠻⣿⠀⢹⣿⣿⠿⠟⠛⠃⠀⢀⣀⣀⠀⠀⠘⠛⠿⢟⣻⣿⣿⣿⣿⣿⣿⣿⣴⠟⠁⠀⢠⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠐⢤⣄⣀",
    "⣿⣿⣿⣶⣶⡆⣧⢸⡄⠀⠈⢣⠀⠀⠙⠀⢸⣷⠆⠀⠀⠀⢐⣶⡆⠙⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠔⢂⣠⣾⡟⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢻⣿",
    "⣿⣿⣿⣿⣿⣷⢸⠀⢿⡄⠀⠀⠳⡀⠀⠀⠸⠂⠀⢾⣧⣤⠀⢩⣽⠆⣹⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣏⣴⣾⣿⣿⡿⢡⣧⠀⠀⠀⠀⠀⠀⣧⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠹",
    "⣿⣿⣿⣿⣿⣿⠐⠘⡌⢿⣦⠀⠀⠈⠀⠀⠀⣌⠳⣦⣍⡛⠓⣂⣴⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠟⢈⣿⣿⣿⣿⣿⢡⣾⣿⡄⠀⠀⠀⠀⢰⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⡆⠀⢳⡌⢿⣇⠀⠀⠀⠀⠀⠈⢷⣦⡙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠿⠛⠉⠀⢀⣼⣿⣿⣿⡿⢡⣿⡿⠟⠁⠀⠀⠀⠀⣾⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⣿⡄⠘⣿⠄⠻⣷⡀⠀⠀⠀⠀⠀⠙⠻⠦⠙⢿⣿⣿⣿⣿⣿⣿⣿⣿⣧⣤⣤⣤⣶⣿⣿⣿⣿⠿⣰⠟⠋⠀⠀⠀⠀⠀⠀⣼⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⣿⣿⡄⠈⠀⠀⠘⠿⣄⠀⠀⢀⡀⠀⠀⠀⠀⠀⠙⠻⢿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⠏⠴⠃⠀⠀⠀⠀⠀⠀⠀⣴⣿⣿⣿⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⡿⠋⠀⠀⠀⠀⠀⠀⠈⠑⢀⠸⣿⠀⠀⠀⠀⠀⠀⠀⠀⠈⠉⠉⠛⠛⠛⠛⠛⠛⠛⠻⠛⣡⠊⠀⠀⠀⠀⠀⠀⠀⣼⣤⣿⣿⣿⣿⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣿⣿⣿⡿⠟⠁⠀⠀⠀⠀⠀⣀⣶⠀⠀⠀⠀⠀⠈⠀⠀⠀⠀⠀⠀⠀⠀⠀⠃⠀⠀⠀⠀⠀⠀⠀⠀⢻⡿⠁⠀⠀⠀⠀⠀⠀⠀⣸⣾⡿⣿⡿⣿⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
    "⣉⣉⣀⣀⠀⢀⣀⣀⣠⣴⣿⣿⡟⠀⠀⠀⠀⣿⡇⡄⠀⠀⠀⠀⠀⠀⠀⠀⢸⡀⠀⠀⠀⠀⠀⠀⠀⠀⠁⠀⠀⠀⠀⠀⠀⠀⣸⣿⣿⣿⣿⠃⣿⠇⠀⠀⠀⠀⠀⠀⠀⣤⡀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠀⠀⠀⠀⣼⡟⢠⣿⠀⠀⠀⠀⠀⠀⠀⠀⠸⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣸⣿⣿⣿⠿⡿⢀⡟⠀⠀⠀⠀⠀⠀⣠⠁⠀⠀⢀⣀",
    "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠁⠀⠀⢀⣼⣿⠁⢸⣿⡄⠀⠀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡼⢿⣯⣿⣿⠾⠁⠘⠀⠀⠀⠀⠀⢀⣾⠉⠀⠀⠐⠀⠛",
    "⣿⣿⣿⣿⣿⣿⣿⣿⣿⠟⠁⠀⣀⣴⣿⣿⠃⠄⣼⢻⡇⢀⣼⣧⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣿⣽⣿⣝⠛⠃⠀⠀⠀⠀⠀⠀⠀⢠⡍⠁⠀⡄⠀⠀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⣿⣿⣥⣤⣶⣾⣿⣿⣿⠇⠀⠀⢻⡎⢳⠀⡿⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⡴⠏⣼⡿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⡿⠁⣤⣈⡀⠀⠀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡟⢠⠀⠀⣿⣷⡘⡇⠀⠀⠀⠀⠀⠀⠀⠀⢠⡄⠀⠀⠀⠀⠀⠀⢀⡴⠟⢀⡞⠗⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢰⠟⣠⣧⣾⣿⠀⠀⠀⠀⠀",
    "⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠀⢺⠀⠀⣿⣿⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⠀⠀⠀⣤⠖⠋⢀⣠⠋⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣠⣿⢿⣿⣿⠋⠁⠀⠀⠀⠀⠀"
};
static int pic_width = 65, pic_heigth = 33;

// use for bottom line
static char str1[50] = "This TTY is locked.";
static char str2[50] = "Please press ENTER to unlock.";
/* static char bline[500] = {0}; */
static wchar_t bline[500] = {0};
static char bline_output[2000] = {0};
/* static wchar_t msg_str[200] = */
/* L" This TTY is locked  Please press ENTER to unlock "; */
static wchar_t msg_str[200] = 
L"/ / / / / / / / / / / / / / / / This TTY is locked / / / / / / / / / / / / / / / / Please press ENTER to unlock ";

// use for locked line
static char lock_str[50] = "[LOCKED]";

// use for prompt line
static int prompt_flg = 0;


void drawMiddlePic(struct winsize w)
{
    if(w.ws_col < pic_width || w.ws_row < pic_heigth + 6)
        return;

    int diff_x = (w.ws_col - pic_width) / 2;
    int diff_y = (w.ws_row - pic_heigth) / 2;
    for(int i = 0; i < pic_heigth; i++)
    {
        printf("\033[%d;%dH", diff_y + i - 1, diff_x);
        printf("\033[31m%s\033[0m", pic[i]);
    }
}

void drawTimeLine(struct winsize w)
{
    time_t rawtime;
    struct tm* timeinfo = NULL;

    time(&rawtime);
    timeinfo = localtime(&rawtime);

    int diff_len = (w.ws_col - 23) / 2;
    printf("\033[0;%dH", diff_len);
    printf("%d-%02d-%02d %s %02d:%02d:%02d\r",
           timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday,
           weekday[timeinfo->tm_wday],
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    fflush(stdout);
}

void drawBottomLine(struct winsize w)
{
    int msg_len = wcslen(msg_str);
    for(int i = 0; i < w.ws_col; i++)
    {
        if(i < msg_len)
            bline[i] = msg_str[i];
        else
            bline[i] = msg_str[i % msg_len];
    }
    bline[w.ws_col] = '\0';

    wchar_t tmp = msg_str[0];
    for(int i = 0; i < msg_len - 1; i++)
    {
        msg_str[i] = msg_str[i + 1];
    }
    msg_str[msg_len - 1] = tmp;

    wcstombs(bline_output, bline, sizeof(bline) * 4);

    printf("\033[%d;0H", w.ws_row - 1);
    printf("\033[1;38;5;227;48;5;236m%s\033[m\n", bline_output);
}

/* void drawBottomLine(struct winsize w) */
/* { */
/*     int msg_len = strlen(msg_str); */
/*     for(int i = 0; i < w.ws_col; i++) */
/*     { */
/*         if(i < msg_len) */
/*             bline[i] = msg_str[i]; */
/*         else */
/*             bline[i] = msg_str[i % msg_len]; */
/*     } */
/*     bline[w.ws_col] = '\0'; */

/*     char tmp = msg_str[0]; */
/*     for(int i = 0; i < msg_len - 1; i++) */
/*     { */
/*         msg_str[i] = msg_str[i + 1]; */
/*     } */
/*     msg_str[msg_len - 1] = tmp; */


/*     printf("\033[%d;0H", w.ws_row - 1); */
/*     printf("\033[1;38;5;227;48;5;236m%s\033[m\n", bline); */
/* } */

void setPromptFlag(int flg)
{
    prompt_flg = flg;
}

void drawLockedLine(struct winsize w)
{
    if(prompt_flg)
        return;

    int lock_len = strlen(lock_str);
    int diff = (w.ws_col - lock_len) / 2;

    printf("\033[%d;0H\033[K", w.ws_row - 4);
    printf("\033[%d;%dH", w.ws_row - 4, diff);
    printf("%s\n", lock_str);
}

void drawPromptLine(struct winsize w, char *username)
{
    if(!prompt_flg)
        return;

    // "'s password", length 11
    int prompt_len = strlen(username) + 11;
    int diff = (w.ws_col - prompt_len) / 2;

    printf("\033[%d;0H\033[K", w.ws_row - 4);
    printf("\033[%d;%dH", w.ws_row - 4, diff);
    printf("%s's password: _\n", username);
}

void drawErrorMessage(struct winsize w, struct AuthResult ar)
{
    int len = strlen(ar.error_msg);
    int diff = (w.ws_col - len) / 2;
    printf("\033[%d;0H\033[K", w.ws_row - 3);
    printf("\033[%d;%dH", w.ws_row - 3, diff);
    printf("\033[31m%s\r\n\033[m", ar.error_msg);
}

void eraseErrorMessage(struct winsize w)
{
    printf("\033[%d;0H\033[K", w.ws_row - 3);
    printf("\n");
}
