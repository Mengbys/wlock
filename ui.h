#ifndef UI_H
#define UI_H

#include "auth.h"

void drawMiddlePic(struct winsize w);
void drawTimeLine(struct winsize w);
void drawBottomLine(struct winsize w);
void setPromptFlag(int flg);
void drawLockedLine(struct winsize w);
void drawPromptLine(struct winsize w, char *username);
void drawErrorMessage(struct winsize w, struct AuthResult ar);
void eraseErrorMessage(struct winsize w);

#endif // !DEBUG
