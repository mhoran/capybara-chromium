#pragma once

#include <stdatomic.h>

#include "include/capi/cef_app_capi.h"

typedef struct _app {
	cef_app_t app;
	atomic_int ref_count;
} app;

void initialize_app_handler(app* app);
