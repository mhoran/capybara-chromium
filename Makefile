all:
	rm -f Release/capybara_server
	gcc -DWINDOWLESS -Wall -Werror -o Release/capybara_server -I. -Wl,-rpath,'$$ORIGIN' -Wl,--format=binary -Wl,src/capybara.js -Wl,--format=default -L./Release src/main_linux.c src/cef_app.c src/cef_client.c src/cef_render_process_handler.c src/cef_life_span_handler.c src/cef_render_handler.c src/cef_load_handler.c src/context.c -lcef -lpthread -std=c11
