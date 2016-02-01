#pragma once

#include <stdatomic.h>

#include "include/capi/cef_client_capi.h"
#include "include/capi/cef_life_span_handler_capi.h"

#include "context.h"

typedef struct _life_span_handler_t {
	cef_life_span_handler_t handler;
	Context *context;
	atomic_int ref_count;
} life_span_handler_t;

int CEF_CALLBACK on_before_popup(struct _cef_life_span_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    const cef_string_t* target_url, const cef_string_t* target_frame_name,
    cef_window_open_disposition_t target_disposition, int user_gesture,
    const struct _cef_popup_features_t* popupFeatures,
    struct _cef_window_info_t* windowInfo, struct _cef_client_t** client,
    struct _cef_browser_settings_t* settings, int* no_javascript_access);

void CEF_CALLBACK on_after_created(struct _cef_life_span_handler_t* self,
    struct _cef_browser_t* browser);

int CEF_CALLBACK run_modal(struct _cef_life_span_handler_t* self,
    struct _cef_browser_t* browser);

int CEF_CALLBACK do_close(struct _cef_life_span_handler_t* self,
    struct _cef_browser_t* browser);

void CEF_CALLBACK on_before_close(struct _cef_life_span_handler_t* self,
    struct _cef_browser_t* browser);
