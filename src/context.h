#pragma once

#include "include/capi/cef_browser_capi.h"
#include "include/capi/cef_client_capi.h"
#include "include/capi/cef_task_capi.h"

typedef struct _Response {
	cef_string_userfree_utf8_t message;
} Response;

typedef struct _Context {
	cef_browser_t *browser;
	void (*on_load_end)(struct _Context *self);
	void (*finish)(struct _Context *self, cef_string_userfree_utf8_t);
	void (*finishFailure)(struct _Context *self, cef_string_userfree_utf8_t);
	Response *pending_response;
	cef_client_t *client;
} Context;

typedef struct {
	cef_task_t task;
	Context *context;
	cef_string_userfree_utf8_t message;
} Task;

void initialize_context(Context *context);
