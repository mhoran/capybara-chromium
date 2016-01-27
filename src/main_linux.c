// Copyright (c) 2014 The cefcapi authors. All rights reserved.
// License: BSD 3-clause.
// Website: https://github.com/CzarekTomczak/cefcapi

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "cef_app.h"
#include "cef_client.h"

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
	cef_string_utf8_clear(&out);
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
startCommand(ReceivedCommand *cmd, Context *context)
{
	if (strcmp(cmd->commandName, "Visit") == 0) {
		cef_string_t url = {};
		cef_string_utf8_to_utf16(cmd->arguments[0], strlen(cmd->arguments[0]), &url);
		cef_frame_t *frame = context->browser->get_main_frame(context->browser);
		context->on_load_end = handle_load_event;
		frame->load_url(frame, &url);
		cef_string_clear(&url);
	} else if (strcmp(cmd->commandName, "Body") == 0) {
		cef_string_visitor_t *visitor;
		visitor = calloc(1, sizeof(cef_string_visitor_t));
		visitor->base.size = sizeof(cef_string_visitor_t);
		initialize_cef_base((cef_base_t*)visitor);
		visitor->visit = get_frame_source;
		cef_frame_t *frame = context->browser->get_main_frame(context->browser);
		frame->get_source(frame, visitor);
	} else if (strcmp(cmd->commandName, "FindCss") == 0 ) {
		fprintf(stderr, "Received FindCss\n");
		cef_string_t name = {};
		cef_string_set(u"CapybaraInvocation", 18, &name, 0);
		cef_process_message_t *message = cef_process_message_create(&name);

		cef_list_value_t *args = message->get_argument_list(message);

		cef_string_t value = {};
		cef_string_set(u"findCss", 7, &value, 0);
		args->set_string(args, 0, &value);

		args->set_bool(args, 1, 1);

		cef_string_utf8_to_utf16(cmd->arguments[0], strlen(cmd->arguments[0]), &value);
		args->set_string(args, 2, &value);
		cef_string_clear(&value);

		context->browser->send_process_message(context->browser, PID_RENDERER, message);
	} else if (strcmp(cmd->commandName, "Node") == 0 ) {
		fprintf(stderr, "Received Node\n");
		cef_string_t name = {};
		cef_string_set(u"CapybaraInvocation", 18, &name, 0);
		cef_process_message_t *message = cef_process_message_create(&name);

		cef_list_value_t *args = message->get_argument_list(message);

		cef_string_t value = {};
		cef_string_utf8_to_utf16(cmd->arguments[0], strlen(cmd->arguments[0]), &value);
		args->set_string(args, 0, &value);

		args->set_bool(args, 1, strcmp(cmd->arguments[1], "true") == 0);

		cef_string_utf8_to_utf16(cmd->arguments[2], strlen(cmd->arguments[2]), &value);
		args->set_string(args, 2, &value);
		cef_string_clear(&value);

		context->browser->send_process_message(context->browser, PID_RENDERER, message);
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
	Context *context = (Context *)arg;
	while (!feof(stdin)) {
		ReceivedCommand *cmd;
		cmd = calloc(1, sizeof(ReceivedCommand));
		cmd->commandName = NULL;
		cmd->argumentsExpected = -1;
		int expectingDataSize = -1;
		int argument_index = 0;

		checkNext(cmd, &expectingDataSize, &argument_index);

		if (argument_index == cmd->argumentsExpected)
			startCommand(cmd, context);
	}

	cef_browser_host_t *host = context->browser->get_host(context->browser);
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
#ifdef WINDOWLESS
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
    client_t c = {};
    Context context = {};
    c.context = &context;
    initialize_client_handler(&c);

    // Create browser.
    fprintf(stderr, "cef_browser_host_create_browser\n");
    cef_client_t *client = (cef_client_t *)&c;
    client->base.add_ref((cef_base_t *)client);
    cef_browser_host_create_browser(&windowInfo, client, NULL,
            &browserSettings, NULL);

    pthread_t pth;
    pthread_create(&pth, NULL, f, &context);

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
