#pragma once

#include <stdatomic.h>

#include "include/capi/cef_render_process_handler_capi.h"

typedef struct _render_process_handler {
	cef_render_process_handler_t handler;
	atomic_int ref_count;
} render_process_handler;

void
CEF_CALLBACK
on_render_thread_created(
    struct _cef_render_process_handler_t* self,
    struct _cef_list_value_t* extra_info);

void
CEF_CALLBACK
on_web_kit_initialized(
    struct _cef_render_process_handler_t* self);

void
CEF_CALLBACK
on_browser_created(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser);

void
CEF_CALLBACK
on_browser_destroyed(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser);

struct
_cef_load_handler_t*
CEF_CALLBACK
get_render_process_load_handler(
    struct _cef_render_process_handler_t* self);

int
CEF_CALLBACK
on_before_navigation(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_request_t* request, cef_navigation_type_t navigation_type,
    int is_redirect);

void
CEF_CALLBACK
on_context_created(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_v8context_t* context);

void
CEF_CALLBACK
on_context_released(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_v8context_t* context);

void
CEF_CALLBACK
on_uncaught_exception(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_v8context_t* context, struct _cef_v8exception_t* exception,
    struct _cef_v8stack_trace_t* stackTrace);

void
CEF_CALLBACK
on_focused_node_changed(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_domnode_t* node);

int
CEF_CALLBACK
on_render_process_message_received(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, cef_process_id_t source_process,
    struct _cef_process_message_t* message);

