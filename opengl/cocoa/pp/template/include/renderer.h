#ifndef RENDERER_H
#define RENDERER_H

#include <stdio.h>

extern FILE *gpFile;

int  initialize();
void display();
void update();
void resize();
void uninitialize();
void onKeyDown(int key);
#endif // !RENDERER_H
