#pragma once

#include "include/capi/cef_browser_capi.h"

typedef struct {
	cef_browser_t *browser;
	void (*on_load_end)();
	void (*finish)(cef_string_utf8_t *);
} Context;

void initialize_context(Context *context);
