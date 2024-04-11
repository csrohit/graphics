cls
del *.obj *.exe
cl.exe /c /EHsc model-obj.c
cl.exe /c /EHsc OGL.cpp /I "C:/libs/glew/include"
rc.exe OGL.rc
link.exe OGL.obj model-obj.obj OGL.res User32.lib GDI32.lib /SUBSYSTEM:WINDOWS /LIBPATH:"C:\libs\glew\lib\Release\x64"