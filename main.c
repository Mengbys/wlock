#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <termios.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <pwd.h>
#include <locale.h>
#include "ui.h"
#include "auth.h"

// default timeout value 60.
#define TIMEOUT_VAL 10

int terminate_signal = 0;
int error_signal = 0;
int enter_signal = 0;
int leave_signal = 0;
int enter_root_signal = 0;
struct termios old_term;
struct winsize w;
struct AuthResult ar;

char *get_username(void)
{
    uid_t uid = getuid();
    char *username = NULL;

    /* Get the user name from the environment if started as root. */
    if (uid == 0)
        username = getenv("USER");

    if (username == NULL)
    {
        struct passwd *pw;

        /* Get the password entry. */
        pw = getpwuid(uid);

        if (pw == NULL)
            return NULL;

        username = pw->pw_name;
    }

    return strdup(username);
}

void getEnter()
{
    struct termios term;
    tcflag_t lflag;
    char c=0;

    /* switch off line buffering */
    (void) tcgetattr(STDIN_FILENO, &term);
    lflag = term.c_lflag;
    term.c_lflag &= ~ICANON;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);

    while(1)
    {
        fd_set readfds;

        /* Initialize file descriptor set. */
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);        /* Wait for a character. */

        if (select(STDIN_FILENO + 1, &readfds, NULL, NULL, NULL) != 1)
            perror("select() on stdin failed.");

        /* Read the character. */
        read(STDIN_FILENO, &c, 1);

        if(c=='\n')
            break;
    }

    /* restore line buffering */
    term.c_lflag = lflag;
    (void) tcsetattr(STDIN_FILENO, TCSANOW, &term);
}

/*
* disable some signal sent from other processes.
* INT  - interrupt process.
* QUIT - quit process.
* TSTP - temporarily stop process (suspend).
*/
void blockSignals(void)
{
    struct sigaction sa;

    /* Ignore some signals. */
    (void) sigemptyset(&(sa.sa_mask));
    sa.sa_flags = SA_RESTART;
    sa.sa_handler = SIG_IGN;
    (void) sigaction(SIGINT, &sa, NULL);
    (void) sigaction(SIGQUIT, &sa, NULL);
    (void) sigaction(SIGTSTP, &sa, NULL);
}

void setupTerminal()
{
    struct termios new_term;

    // clear the screen
    printf("\033[H\033[J");

    // clear cursor
    printf("\033[?25l");

    // disable echo and signal sent from keyboard such as Ctrl-z
    tcgetattr(STDIN_FILENO, &old_term);
    new_term = old_term;
    new_term.c_lflag &= ~(ECHO | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_term);
}

void restoreTerminal()
{
    printf("\033[H\033[J");
    printf("\033[?25h");

    tcsetattr(STDIN_FILENO, TCSANOW, &old_term);
}

/* routine1: authentication. */
void *routine1(void *ptr)
{
    char *username = NULL;
    username = get_username();
    struct timespec *prompt_timeout = malloc(sizeof * prompt_timeout);
    /*
    * when set value to a struct instance,
    * you better set up all the member of this stuct instance!
    */
    prompt_timeout->tv_sec = TIMEOUT_VAL;
    prompt_timeout->tv_nsec = 0;

    while(1)
    {
        /* getchar(); */
        getEnter();
        enter_signal = 1;
        setPromptFlag(1);

        if(auth(username, prompt_timeout, &ar))
        {
            terminate_signal = 1;
            free(prompt_timeout);
            pthread_exit(0);
        }

        error_signal = 1;

        if(strcmp(username, "root") != 0)
        {
            enter_root_signal = 1;
            if(auth("root", prompt_timeout, &ar))
            {
                terminate_signal = 1;
                free(prompt_timeout);
                pthread_exit(0);
            }
        }

        error_signal = 1;
        leave_signal = 1;
        setPromptFlag(0);
    }

    return NULL;
}

/* routine2: update ui presentation. */
void *routine2(void *ptr)
{
    char *username = NULL;
    username = get_username();

    int flg1 = 0, flg2 = 0, flg3 = 0;

    int old_width, old_length;

    ioctl(0, TIOCGWINSZ, &w);
    setlocale(LC_CTYPE, "");
    drawTimeLine(w);
    drawMiddlePic(w);
    drawBottomLine(w);
    drawLockedLine(w);

    int ac=0,ac2=0;
    while(1)
    {
        if(terminate_signal == 1)
            pthread_exit(0);

        // re-draw when window size changes.
        old_width = w.ws_col;
        old_length = w.ws_row;
        ioctl(0, TIOCGWINSZ, &w);
        if(w.ws_col != old_width || w.ws_row != old_length)
        {
            printf("\033[H\033[J");
            drawTimeLine(w);
            drawMiddlePic(w);
            drawBottomLine(w);
            drawLockedLine(w);
            drawPromptLine(w, username);
        }

        // display error message when get a error signal.
        if(error_signal == 1)
        {
            drawErrorMessage(w, ar);
            error_signal = 0;
            flg3 = 1;
        }

        // display prompt string when press 'ENTER'.
        if(enter_signal == 1)
        {
            drawPromptLine(w, username);
            enter_signal = 0;
        }

        // display prompt string for root's password when fail in user's.
        if(enter_root_signal == 1)
        {
            drawPromptLine(w, "root");
            enter_root_signal = 0;
        }

        // re-draw '[LOCKED]' string when fail in authentication.
        if(leave_signal == 1)
        {
            drawLockedLine(w);
            leave_signal = 0;
        }

        // update time-line every second.
        if(ac==100)
        {
            drawTimeLine(w);
            ac=0;
        }

        // update bottom-line every 0.1 second.
        if(ac%10==0)
        {
            drawBottomLine(w);
        }

        // erase the displayed error message after 2 seconds.
        if(flg3 == 1)
        {
            ac2++;
            if(ac2==200)
            {
                eraseErrorMessage(w);
                flg3 = 0;
                ac2=0;
            }
        }

        ac++;
        usleep(10000);
    }

}

int main(int argc, const char *argv[])
{
    blockSignals(); // please comment while debugging.
    setupTerminal();
    atexit(restoreTerminal);

    pthread_t thread1, thread2;
    pthread_create(&thread1, NULL, routine1, NULL);
    pthread_create(&thread2, NULL, routine2, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);



    return 0;
}
