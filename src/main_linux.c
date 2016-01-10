// Copyright (c) 2014 The cefcapi authors. All rights reserved.
// License: BSD 3-clause.
// Website: https://github.com/CzarekTomczak/cefcapi

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/prctl.h>

#include <gdk/gdkx.h>

#include "cef_app.h"
#include "cef_client.h"
#include "gtk.h"

typedef struct {
	int expectingDataSize;
	int argumentsExpected;
	char *commandName;
	char *argument;
} ReceivedCommand;

void processArgument(ReceivedCommand *cmd, const char *data) {
	if (cmd->argumentsExpected == -1) {
		int i = atoi(data);
		cmd->argumentsExpected = i;
	} else if (cmd->expectingDataSize == -1) {
		int i = atoi(data);
		cmd->expectingDataSize = i;
	} else {
		int len = strlen(data) + 1;
		cmd->argument = calloc(len, sizeof(char));
		// append...
		strncpy(cmd->argument, data, len);
	}

	if (cmd->argumentsExpected == 0 || (cmd->argumentsExpected == 1 && cmd->argument != NULL)) {
		printf("ok\n");
		printf("0\n");
		fflush(stdout);
		cmd->argumentsExpected = -1;
		cmd->commandName = NULL;
	}
}

void processNext(ReceivedCommand *cmd, const char *data) {
	if (cmd->commandName == NULL) {
		int len = strlen(data) + 1;
		cmd->commandName = calloc(len, sizeof(char));
		strncpy(cmd->commandName, data, len);
		cmd->argumentsExpected = -1;
	} else {
		processArgument(cmd, data);
	}
}

void *f(void *arg) {
	int c = 0;
	char buffer[128];
	ReceivedCommand *cmd = NULL;
	while((c = getchar()) != EOF) {
		ungetc(c, stdin);
		cmd = calloc(1, sizeof(ReceivedCommand));
		cmd->expectingDataSize = -1;
		cmd->commandName = NULL;
j:
		if (cmd->expectingDataSize == -1) {
			// readLine
			fgets(buffer, sizeof(buffer), stdin);
			buffer[strlen(buffer) - 1] = 0;

			processNext(cmd, buffer);

			goto j;
		} else {
			// readDataBlock
			char otherBuffer[cmd->expectingDataSize + 1];
			fread(otherBuffer, 1, cmd->expectingDataSize, stdin);
			otherBuffer[cmd->expectingDataSize] = 0;

			processNext(cmd, otherBuffer);

			cmd->expectingDataSize = -1;
			goto j;
		}
	};

	return NULL;
}

int main(int argc, char** argv) {
    prctl(PR_SET_PDEATHSIG, SIGHUP);

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

    // Create GTK window. You can pass a NULL handle 
    // to CEF and then it will create a window of its own.
    initialize_gtk();
    GtkWidget* hwnd = create_gtk_window("cefcapi example", 1024, 768);
    cef_window_info_t windowInfo = {};
    windowInfo.parent_window = gdk_x11_drawable_get_xid(gtk_widget_get_window(hwnd));
    // windowInfo.parent_window = hwnd;

    // Executable's directory
    char appPath[1024] = {};
    ssize_t ppLen = readlink("/proc/self/exe", appPath,
            sizeof(appPath)-1);
    if (ppLen != -1 && ppLen > 0) {
        appPath[ppLen] = '\0';
        do {
            ppLen -= 1;
            appPath[ppLen+1] = '\0';
        } while (appPath[ppLen] != '/' && ppLen > 0);
        // No slash at the end.
        if (ppLen > 0 && appPath[ppLen] == '/') {
            appPath[ppLen] = '\0';
        }
    }
    fprintf(stderr, "Executable's directory: %s\n", appPath);
    
    // Initial url.
    char url[1024];
    snprintf(url, sizeof(url), "file://%s/example.html", appPath);
    // There is no _cef_string_t type.
    cef_string_t cefUrl = {};
    cef_string_utf8_to_utf16(url, strlen(url), &cefUrl);
    
    // Browser settings.
    // It is mandatory to set the "size" member.
    cef_browser_settings_t browserSettings = {};
    browserSettings.size = sizeof(cef_browser_settings_t);
    
    // Client handler and its callbacks.
    // cef_client_t structure must be filled. It must implement
    // reference counting. You cannot pass a structure 
    // initialized with zeroes.
    cef_client_t client = {};
    initialize_client_handler(&client);

    // Create browser.
    fprintf(stderr, "cef_browser_host_create_browser\n");
    cef_browser_host_create_browser(&windowInfo, &client, &cefUrl,
            &browserSettings, NULL);

    pthread_t pth;
    pthread_create(&pth, NULL, f, NULL);

    printf("Ready\n");
    fflush(stdout);

    // Message loop.
    fprintf(stderr, "cef_run_message_loop\n");
    cef_run_message_loop();

    // Shutdown CEF.
    fprintf(stderr, "cef_shutdown\n");
    cef_shutdown();

    return 0;
}
