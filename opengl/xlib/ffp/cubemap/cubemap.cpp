
/*  cubemap.c
 *
 *  This program demonstrates cube map textures.
 *  Six different colored checker board textures are
 *  created and applied to a lit sphere.
 *
 *  Pressing the 'f' and 'b' keys translate the viewer
 *  forward and backward.
 */

#include "stb_image.h"
#include <GL/glut.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#define _USE_MATH_DEFINES
#include <math.h>

#define imageSize 4
static GLubyte image1[imageSize][imageSize][4];
static GLubyte image2[imageSize][imageSize][4];
static GLubyte image3[imageSize][imageSize][4];
static GLubyte image4[imageSize][imageSize][4];
static GLubyte image5[imageSize][imageSize][4];
static GLubyte image6[imageSize][imageSize][4];

static GLdouble ztrans = 0.0;

void makeImages(void)
{
    const char *names[] = {"res/right.jpg", "res/left.jpg", "res/top.jpg", "res/bottom.jpg", "res/front.jpg", "res/back.jpg"};
    int         width, height, nrChannels;

    for (uint32_t idx = 0U; idx < 6U; ++idx)
    {
        unsigned char *data = stbi_load(names[idx], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X_EXT + idx, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
    }
}

void init(void)
{
    GLfloat diffuse[4] = {1.0, 1.0, 1.0, 1.0};

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glEnable(GL_DEPTH_TEST);
    glShadeModel(GL_SMOOTH);


    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_WRAP_R, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    makeImages();
    // glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT);
    // glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT);
    // glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_NORMAL_MAP_EXT);
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP_EXT);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    glEnable(GL_TEXTURE_CUBE_MAP_EXT);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
}
static float angle = 0.00;

void display(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    angle += 20;
    if(angle > 360.0f)
    {
        angle -= 360.0f;
    }
    float x = 20.0f*cosf(angle);
    float z = 20.0f*sinf(angle);
    gluLookAt(x, 0.0f, z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    

    glPushMatrix();
    glTranslatef(0.0, 0.0, ztrans);
    glutSolidSphere(5.0, 100, 100);
    glPopMatrix();
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(40.0, (GLfloat)w / (GLfloat)h, 1.0, 300.0);
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'f':
            ztrans = ztrans - 0.2;
            glutPostRedisplay();
            break;
        case 'b':
            ztrans = ztrans + 0.2;
            glutPostRedisplay();
            break;
        case 27:
            exit(0);
            break;
        default:
            break;
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutMainLoop();
    return 0;
}
