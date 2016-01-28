#pragma once

#include <stdatomic.h>

#include "include/capi/cef_load_handler_capi.h"

#include "context.h"

typedef struct _load_handler {
	cef_load_handler_t handler;
	Context *context;
	atomic_int ref_count;
} load_handler;

void CEF_CALLBACK on_loading_state_change(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, int isLoading, int canGoBack,
    int canGoForward);

void CEF_CALLBACK on_load_start(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame);

void CEF_CALLBACK on_load_end(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    int httpStatusCode);

void CEF_CALLBACK on_load_error(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    cef_errorcode_t errorCode, const cef_string_t* errorText,
    const cef_string_t* failedUrl);
