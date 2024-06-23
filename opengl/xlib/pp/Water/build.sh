#!/bin/bash

rm ogl ogl.o
g++ -c -o ogl.o -g3 -I/usr/include ./ogl.cpp -g3
g++ -o ogl -g3 -L/usr/lib/x86_64-linux-gnu -L . ogl.o -lX11 -lGL -lGLEW -lSphere
