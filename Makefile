all:
	rm -f Release/capybara_server
	gcc -DWINDOWLESS -Wall -Werror -o Release/capybara_server -I. -Wl,-rpath,'$$ORIGIN' -L./Release src/main_linux.c -lcef `pkg-config --libs --cflags gtk+-2.0` -lpthread
