cls
cl.exe /c /EHsc OGL.cpp /I "C:/libs/glew/include"
cl.exe /c /EHsc stb.cpp /I "C:/libs/glew/include"
rc.exe OGL.rc
link.exe OGL.obj stb.obj OGL.res User32.lib GDI32.lib /SUBSYSTEM:WINDOWS /LIBPATH:"C:\libs\glew\lib\Release\x64"