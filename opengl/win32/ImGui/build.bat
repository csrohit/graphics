cls
del ogl.obj
del ogl.exe
cl.exe /c /EHsc OGL.cpp imgui/*.cpp /I "C:/libs/glew/include"
rc.exe OGL.rc
link.exe *.obj /OUT:ogl.exe OGL.res User32.lib GDI32.lib /SUBSYSTEM:WINDOWS /LIBPATH:"C:\libs\glew\lib\Release\x64"