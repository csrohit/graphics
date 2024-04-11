cls
del *.obj *.exe *.res
cl.exe /c /EHsc OGL.cpp /I "C:/libs/glew/include" /I C:\Users\rohit\Downloads\Compressed\assimp-3.1.1-win-binaries\assimp-3.1.1-win-binaries\include
@REM cl.exe /c /EHsc shader.cpp /I "C:/libs/glew/include"
rc.exe OGL.rc
link.exe OGL.obj  OGL.res User32.lib GDI32.lib /SUBSYSTEM:WINDOWS /LIBPATH:"C:\libs\glew\lib\Release\x64"