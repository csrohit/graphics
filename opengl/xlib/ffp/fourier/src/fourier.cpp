#include "fourier.h"
#include "queue.h"
#include <GL/gl.h>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <math.h>
int gn = 2;
static CircularBuffer *bCarrier;
static CircularBuffer *bMessage;
static CircularBuffer *bOut;

void initializeFourier()
{
    bCarrier = createCircularBuffer(3000);
    bMessage = createCircularBuffer(3000);
    bOut = createCircularBuffer(3000);
}

void freeFourier()
{
    freeCircularBuffer(bCarrier);
    freeCircularBuffer(bMessage);
    freeCircularBuffer(bOut);
}

void drawWave(CircularBuffer* cb)
{
    glBegin(GL_LINE_STRIP);
    int head = cb->head;
    for (int32_t idx = 0U; idx < cb->count; idx++)
    {
        if(idx >= head)
        {
            head = cb->head + cb->count;
        }
        float data = cb->buffer[head - idx -1];
        glVertex2f(-6.0f + (idx)*0.005f, data);

    }
    printf("\n\n\n\n\n");
    glEnd();
}
void drawFM()
{
    static float angle  = 0.0f;
    struct Point pt     = {};
    struct Point pt1     = {0.0f, 1.0f};
    float fc = 50.0f;
    float fm = 10.0f;
    float ac = sinf(fc*angle);
    float am = sinf(fm*angle);
    float data = sinf((fc + cos(fm*angle))*angle);
    writeBuffer(bCarrier, ac);
    writeBuffer(bMessage, am);
    writeBuffer(bOut, data);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0f, 3.0f, 0.0f);
    drawWave(bCarrier);
    glTranslatef(0.0f, -3.0f, 0.0f);
    drawWave(bMessage);
    glTranslatef(0.0f, -3.0f, 0.0f);
    drawWave(bOut);
    glPopMatrix();
    angle += 0.005f;
}

void drawAM()
{
    static float angle  = 0.0f;
    struct Point pt     = {};
    struct Point pt1     = {0.0f, 1.0f};
    float fm = 1.0f;
    float fc = 30.0f;
    float ac = sinf(fc*angle + 0.2f);
    float am = sinf(fm*angle);
    float data = (1 + am)*ac;
    writeBuffer(bCarrier, ac);
    writeBuffer(bMessage, am);
    writeBuffer(bOut, data);
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(0.0f, 3.0f, 0.0f);
    drawWave(bCarrier);
    glTranslatef(0.0f, -3.0f, 0.0f);
    drawWave(bMessage);
    glTranslatef(0.0f, -3.0f, 0.0f);
    drawWave(bOut);
    glPopMatrix();
    angle += 0.01f;
}

void drawSqaure()
{
    // drawAM();
    // return;
    static float angle  = 0.0f;
    struct Point pt     = {};
    float        radius = 7.0f / M_PI;

    struct Point center = {-9.0f, 0.0f};
    for (int n = 1; n < gn; n += 2)
    {
        radius = 6.0f / (n * M_PI);
        drawCircle(center, radius, angle, &pt);
        pt.x = center.x + radius * cos(n * angle);
        pt.y = center.y + radius * sin(n * angle);
        drawLine(center, pt);
        center = pt;
    }
    drawLine(pt, {-6.0f, pt.y});
    writeBuffer(bCarrier, pt.y);
    drawWave(bCarrier);
    angle += 0.01f;
}

void drawCircle(struct Point center, float radius, float startAngle, struct Point *pOut)
{
    glBegin(GL_LINE_LOOP);
    for (float angle = startAngle; angle < 2 * M_PI + startAngle; angle += 0.01)
    {
        pOut->x = center.x + radius * cosf(angle);
        pOut->y = center.y + radius * sinf(angle);
        glVertex2fv((float *)pOut);
    }

    glEnd();
}

void drawLine(struct Point a, struct Point b)
{
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glVertex2fv((float *)&a);
    glVertex2fv((float *)&b);
    glEnd();
}
