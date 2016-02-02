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
#include "cef_base.h"
#include "string_visitor.h"
#include "command.h"

typedef struct {
	int argumentsExpected;
	char *commandName;
	char **arguments;
} ReceivedCommand;

void
startCommand(ReceivedCommand *cmd, Context *context)
{
	fprintf(stderr, "Received %s(%s)\n", cmd->commandName,
	    cmd->argumentsExpected != 0 ? cmd->arguments[0] : "");

	Command command = {};
	if (strcmp(cmd->commandName, "Visit") == 0) {
		initialize_visit_command(&command, cmd->arguments);
	} else if (strcmp(cmd->commandName, "Body") == 0) {
		initialize_body_command(&command, cmd->arguments);
	} else if (strcmp(cmd->commandName, "FindCss") == 0 ) {
		initialize_find_css_command(&command, cmd->arguments);
	} else if (strcmp(cmd->commandName, "Node") == 0 ) {
		initialize_node_command(&command, cmd->arguments, cmd->argumentsExpected);
	} else if (strcmp(cmd->commandName, "FindXpath") == 0 ) {
		initialize_find_xpath_command(&command, cmd->arguments);
	} else if (strcmp(cmd->commandName, "Reset") == 0 ) {
		initialize_reset_command(&command, cmd->arguments);
	} else {
		printf("ok\n");
		printf("0\n");
		fflush(stdout);
		return;
	}
	command.run(&command, context);
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
	Context *context = arg;
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

	cef_task_t *t = calloc(1, sizeof(cef_task_t));
	t->base.size = sizeof(cef_task_t);
	t->execute = cef_quit_message_loop;
	cef_post_task(TID_UI, t);

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
    app *a;
    a = calloc(1, sizeof(app));
    initialize_app_handler(a);
    cef_app_t *app = (cef_app_t *)a;
    app->base.add_ref((cef_base_t *)a);
    
    // Execute subprocesses.
    app->base.add_ref((cef_base_t *)a);
    int code = cef_execute_process(&mainArgs, app, NULL);
    if (code >= 0) {
        _exit(code);
    }
    
    // Application settings.
    // It is mandatory to set the "size" member.
    cef_settings_t settings = {};
    settings.size = sizeof(cef_settings_t);
    settings.no_sandbox = 1;

    // Initialize CEF.
    app->base.add_ref((cef_base_t *)a);
    cef_initialize(&mainArgs, &settings, app, NULL);
    app->base.release((cef_base_t *)a);

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
    initialize_context(&context);
    c.context = &context;
    initialize_client_handler(&c);

    // Create browser.
    cef_client_t *client = (cef_client_t *)&c;
    client->base.add_ref((cef_base_t *)client);
    context.client = client;

    cef_request_context_settings_t request_context_settings = {};
    request_context_settings.size = sizeof(cef_request_context_settings_t);

    cef_request_context_t *request_context =
	cef_request_context_create_context(&request_context_settings, NULL);

    client->base.add_ref((cef_base_t *)client);
    cef_browser_t *browser = cef_browser_host_create_browser_sync(&windowInfo,
	client, NULL, &browserSettings, request_context);
    browser->base.add_ref((cef_base_t *)browser);
    context.browser = browser;

    printf("Ready\n");
    fflush(stdout);

    pthread_t pth;
    pthread_create(&pth, NULL, f, &context);

    // Message loop.
    cef_run_message_loop();

    // Shutdown CEF.
    cef_shutdown();

    return 0;
}
