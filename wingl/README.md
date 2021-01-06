# huh?

small experiment to cross-compile a Windows binary linking against OpenGL, and run it against wine.
no real purpose to it.

	adele:~/braindamage% x86_64-w64-mingw32-g++ -o main -lopengl32 main.cpp -lgdi32 -lopengl32
	zsh: exit 0
	adele:~/braindamage% file ./main.exe 
	./main.exe: PE32+ executable (console) x86-64, for MS Windows
	zsh: exit 0
	adele:~/braindamage% ./main.exe 
	^C00d2:fixme:console:CONSOLE_DefaultHandler Terminating process c7 on event 0
	zsh: exit 0
	adele:~/braindamage% ldd ./main.exe 
		not a dynamic executable
	zsh: exit 1

