#ifndef FOURIER_H
#define FOURIER_H


struct Point
{
    float x;
    float y;
};


void initializeFourier();

void freeFourier();

void drawCircle(struct Point center, float radius, float startAngle, struct Point *pOut);

void drawLine(struct Point a, struct Point b);
void drawSqaure();

#endif // !FOURIER_H
