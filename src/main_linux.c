// Copyright (c) 2014 The cefcapi authors. All rights reserved.
// License: BSD 3-clause.
// Website: https://github.com/CzarekTomczak/cefcapi

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#ifndef WINDOWLESS

#include <gdk/gdkx.h>

#include "gtk.h"
#endif
#include "cef_app.h"
#include "cef_client.h"
#include "cef_life_span_handler.h"
#include "cef_load_handler.h"
#include "cef_render_handler.h"

typedef struct {
	int argumentsExpected;
	char *commandName;
	char **arguments;
} ReceivedCommand;

void CEF_CALLBACK get_frame_source(struct _cef_string_visitor_t* self,
    const cef_string_t* string) {
	printf("ok\n");
	cef_string_utf8_t out = {};
	cef_string_utf16_to_utf8(string->str, string->length, &out);
	printf("%zu\n", out.length);
	printf("%s", out.str);
	fflush(stdout);
};

void
handle_load_event()
{
	printf("ok\n");
	printf("0\n");
	fflush(stdout);
}

void
startCommand(ReceivedCommand *cmd, client_t *client)
{
	if (strcmp(cmd->commandName, "Visit") == 0) {
		cef_string_t some_url = {};
		cef_string_utf8_to_utf16(cmd->arguments[0], strlen(cmd->arguments[0]), &some_url);
		cef_frame_t *frame = client->browser->get_main_frame(client->browser);
		client->on_load_end = handle_load_event;
		frame->load_url(frame, &some_url);
	} else if (strcmp(cmd->commandName, "Body") == 0) {
		cef_string_visitor_t *visitor;
		visitor = calloc(1, sizeof(cef_string_visitor_t));
		visitor->base.size = sizeof(cef_string_visitor_t);
		initialize_cef_base((cef_base_t*)visitor);
		visitor->visit = get_frame_source;
		cef_frame_t *frame = client->browser->get_main_frame(client->browser);
		frame->get_source(frame, visitor);
	} else if (strcmp(cmd->commandName, "FindCss") == 0 ) {
		fprintf(stderr, "Received FindCss\n");
		cef_string_t message_name = {};
		cef_string_utf8_to_utf16("CapybaraInvocation", 18, &message_name);
		cef_process_message_t *message = cef_process_message_create(&message_name);

		cef_list_value_t *args = message->get_argument_list(message);

		cef_string_t name = {};
		cef_string_utf8_to_utf16("findCss", 7, &name);
		args->set_string(args, 0, &name);

		args->set_bool(args, 1, 1);

		cef_string_t argument = {};
		cef_string_utf8_to_utf16(cmd->arguments[0], strlen(cmd->arguments[0]), &argument);
		args->set_string(args, 2, &argument);

		client->browser->send_process_message(client->browser, PID_RENDERER, message);
	} else if (strcmp(cmd->commandName, "Node") == 0 ) {
		fprintf(stderr, "Received Node\n");
		cef_string_t message_name = {};
		cef_string_utf8_to_utf16("CapybaraInvocation", 18, &message_name);
		cef_process_message_t *message = cef_process_message_create(&message_name);

		cef_list_value_t *args = message->get_argument_list(message);

		cef_string_t function_name = {};
		cef_string_utf8_to_utf16(cmd->arguments[0], strlen(cmd->arguments[0]), &function_name);
		args->set_string(args, 0, &function_name);

		args->set_bool(args, 1, strcmp(cmd->arguments[1], "true") == 0);

		cef_string_t argument = {};
		cef_string_utf8_to_utf16(cmd->arguments[2], strlen(cmd->arguments[2]), &argument);
		args->set_string(args, 2, &argument);

		client->browser->send_process_message(client->browser, PID_RENDERER, message);
	} else {
		fprintf(stderr, "Received unknown command: %s\n", cmd->commandName);
		printf("ok\n");
		printf("0\n");
		fflush(stdout);
	}
}

void
processArgument(ReceivedCommand *cmd, const char *data, int *expectingDataSize, int *argument_index)
{
	if (cmd->argumentsExpected == -1) {
		int i = atoi(data);
		cmd->argumentsExpected = i;
		cmd->arguments = calloc(i, sizeof(char *));
	} else if (*expectingDataSize == -1) {
		int i = atoi(data);
		*expectingDataSize = i;
	} else {
		int len = strlen(data) + 1;
		cmd->arguments[*argument_index] = calloc(len, sizeof(char));
		strncpy(cmd->arguments[*argument_index], data, len);
		*argument_index += 1;
	}
}

void
processNext(ReceivedCommand *cmd, const char *data, int *expectingDataSize, int *argument_index)
{
	if (cmd->commandName == NULL) {
		int len = strlen(data) + 1;
		cmd->commandName = calloc(len, sizeof(char));
		strncpy(cmd->commandName, data, len);
	} else {
		processArgument(cmd, data, expectingDataSize, argument_index);
	}
}

void
checkNext(ReceivedCommand *cmd, int *expectingDataSize, int *argument_index)
{
	if (*expectingDataSize == -1) {
		// readLine
		char buffer[128];
		fgets(buffer, sizeof(buffer), stdin);
		if (feof(stdin))
		    return;
		buffer[strlen(buffer) - 1] = 0;

		processNext(cmd, buffer, expectingDataSize, argument_index);
	} else {
		// readDataBlock
		char otherBuffer[*expectingDataSize + 1];
		int bytesRead;
		bytesRead = fread(otherBuffer, 1, *expectingDataSize, stdin);
		if (bytesRead != *expectingDataSize)
			return;
		otherBuffer[*expectingDataSize] = 0;

		processNext(cmd, otherBuffer, expectingDataSize, argument_index);

		*expectingDataSize = -1;
	}

	if (*argument_index != cmd->argumentsExpected)
		checkNext(cmd, expectingDataSize, argument_index);
}

void *f(void *arg) {
	client_t *client = (client_t *)arg;
	while (!feof(stdin)) {
		ReceivedCommand *cmd;
		cmd = calloc(1, sizeof(ReceivedCommand));
		cmd->commandName = NULL;
		cmd->argumentsExpected = -1;
		int expectingDataSize = -1;
		int argument_index = 0;

		checkNext(cmd, &expectingDataSize, &argument_index);

		if (argument_index == cmd->argumentsExpected)
			startCommand(cmd, client);
	}

	cef_browser_host_t *host = client->browser->get_host(client->browser);
	host->close_browser(host, 1);

	return NULL;
}

int main(int argc, char** argv) {
    // Main args.
    cef_main_args_t mainArgs = {};
    mainArgs.argc = argc;
    mainArgs.argv = argv;
    
    // Application handler and its callbacks.
    // cef_app_t structure must be filled. It must implement
    // reference counting. You cannot pass a structure 
    // initialized with zeroes.
    cef_app_t app = {};
    initialize_app_handler(&app);
    
    // Execute subprocesses.
    fprintf(stderr, "cef_execute_process, argc=%d\n", argc);
    int code = cef_execute_process(&mainArgs, &app, NULL);
    if (code >= 0) {
        _exit(code);
    }
    
    // Application settings.
    // It is mandatory to set the "size" member.
    cef_settings_t settings = {};
    settings.size = sizeof(cef_settings_t);
    settings.no_sandbox = 1;

    // Initialize CEF.
    fprintf(stderr, "cef_initialize\n");
    cef_initialize(&mainArgs, &settings, &app, NULL);

    cef_window_info_t windowInfo = {};
#ifndef WINDOWLESS
    // Create GTK window. You can pass a NULL handle 
    // to CEF and then it will create a window of its own.
    initialize_gtk();
    GtkWidget* hwnd = create_gtk_window("cefcapi example", 1024, 768);
    windowInfo.parent_window = gdk_x11_drawable_get_xid(gtk_widget_get_window(hwnd));
    // windowInfo.parent_window = hwnd;
#else
    windowInfo.windowless_rendering_enabled = 1;
#endif
    
    // Browser settings.
    // It is mandatory to set the "size" member.
    cef_browser_settings_t browserSettings = {};
    browserSettings.size = sizeof(cef_browser_settings_t);
    
    // Client handler and its callbacks.
    // cef_client_t structure must be filled. It must implement
    // reference counting. You cannot pass a structure 
    // initialized with zeroes.
    client_t client = {};
    client.browser = NULL;
    client.on_load_end = NULL;
    initialize_client_handler((cef_client_t *)&client);

    // Create browser.
    fprintf(stderr, "cef_browser_host_create_browser\n");
    cef_browser_host_create_browser(&windowInfo, (cef_client_t *)&client, NULL,
            &browserSettings, NULL);

    pthread_t pth;
    pthread_create(&pth, NULL, f, &client);

    // Message loop.
    fprintf(stderr, "cef_run_message_loop\n");
    cef_run_message_loop();

    // Shutdown CEF.
    fprintf(stderr, "cef_shutdown\n");
    cef_shutdown();

    return 0;
}

static void ready() {
    printf("Ready\n");
    fflush(stdout);
}

static cef_string_t *m_capybaraJavascript = NULL;

static
cef_string_t *
loadJavascript() {
	if (m_capybaraJavascript != NULL)
		return m_capybaraJavascript;

	extern char _binary_src_capybara_js_start;
	extern char _binary_src_capybara_js_end;

	int size = (char *)&_binary_src_capybara_js_end - (char *)&_binary_src_capybara_js_start;

	m_capybaraJavascript = calloc(1, sizeof(cef_string_t));
	cef_string_utf8_to_utf16(&_binary_src_capybara_js_start, size, m_capybaraJavascript);
	return m_capybaraJavascript;
}
