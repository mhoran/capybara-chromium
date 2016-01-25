all:
	rm -f Release/capybara_server
	gcc -DWINDOWLESS -Wall -Werror -o Release/capybara_server -I. -Wl,-rpath,'$$ORIGIN' -Wl,--format=binary -Wl,src/capybara.js -Wl,--format=default -L./Release src/main_linux.c -lcef `pkg-config --libs --cflags gtk+-2.0` -lpthread
