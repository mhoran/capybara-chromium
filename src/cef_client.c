// Copyright (c) 2014 The cefcapi authors. All rights reserved.
// License: BSD 3-clause.
// Website: https://github.com/CzarekTomczak/cefcapi

#include <string.h>

#include "cef_life_span_handler.h"
#include "cef_render_handler.h"
#include "cef_load_handler.h"
#include "context.h"
#include "cef_client.h"
#include "cef_base.h"

IMPLEMENT_REFCOUNTING(client_t)
GENERATE_CEF_BASE_INITIALIZER(client_t)

// ----------------------------------------------------------------------------
// struct _cef_client_t
// ----------------------------------------------------------------------------

///
// Implement this structure to provide handler implementations.
///

///
// Return the handler for context menus. If no handler is provided the default
// implementation will be used.
///

struct _cef_context_menu_handler_t* CEF_CALLBACK get_context_menu_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_context_menu_handler\n");
    return NULL;
}

///
// Return the handler for dialogs. If no handler is provided the default
// implementation will be used.
///
struct _cef_dialog_handler_t* CEF_CALLBACK get_dialog_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_dialog_handler\n");
    return NULL;
}

///
// Return the handler for browser display state events.
///
struct _cef_display_handler_t* CEF_CALLBACK get_display_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_display_handler\n");
    return NULL;
}

///
// Return the handler for download events. If no handler is returned downloads
// will not be allowed.
///
struct _cef_download_handler_t* CEF_CALLBACK get_download_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_download_handler\n");
    return NULL;
}

///
// Return the handler for drag events.
///
struct _cef_drag_handler_t* CEF_CALLBACK get_drag_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_drag_handler\n");
    return NULL;
}

///
// Return the handler for focus events.
///
struct _cef_focus_handler_t* CEF_CALLBACK get_focus_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_focus_handler\n");
    return NULL;
}

///
// Return the handler for geolocation permissions requests. If no handler is
// provided geolocation access will be denied by default.
///
struct _cef_geolocation_handler_t* CEF_CALLBACK get_geolocation_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_geolocation_handler\n");
    return NULL;
}

///
// Return the handler for JavaScript dialogs. If no handler is provided the
// default implementation will be used.
///
struct _cef_jsdialog_handler_t* CEF_CALLBACK get_jsdialog_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_jsdialog_handler\n");
    return NULL;
}

///
// Return the handler for keyboard events.
///
struct _cef_keyboard_handler_t* CEF_CALLBACK get_keyboard_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_keyboard_handler\n");
    return NULL;
}

///
// Return the handler for browser life span events.
///
struct _cef_life_span_handler_t* CEF_CALLBACK get_life_span_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_life_span_handler\n");

    life_span_handler_t *h;
    h = calloc(1, sizeof(life_span_handler_t));

    h->context = ((client_t *)self)->context;
    cef_life_span_handler_t *handler = &h->handler;

    initialize_cef_base(h);
    handler->on_before_popup = on_before_popup;
    handler->on_after_created = on_after_created;
    handler->run_modal = run_modal;
    handler->do_close = do_close;
    handler->on_before_close = on_before_close;

    handler->base.add_ref((cef_base_t *)h);

    return handler;
}

///
// Return the handler for browser load status events.
///
struct _cef_load_handler_t* CEF_CALLBACK get_load_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_load_handler\n");

    load_handler *h;
    h = calloc(1, sizeof(load_handler));

    h->context = ((client_t *)self)->context;
    cef_load_handler_t *handler = &h->handler;

    initialize_cef_base(h);
    handler->on_loading_state_change = on_loading_state_change;
    handler->on_load_start = on_load_start;
    handler->on_load_end = on_load_end;
    handler->on_load_error = on_load_error;

    handler->base.add_ref((cef_base_t *)h);

    return handler;
}

///
// Return the handler for off-screen rendering events.
///
struct _cef_render_handler_t* CEF_CALLBACK get_render_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_render_handler\n");

#ifdef WINDOWLESS
    render_handler *h;
    h = calloc(1, sizeof(render_handler));

    cef_render_handler_t *handler = (cef_render_handler_t *)h;
    initialize_cef_base(h);

    handler->get_root_screen_rect = get_root_screen_rect;
    handler->get_screen_info = get_screen_info;
    handler->get_screen_point = get_screen_point;
    handler->get_view_rect = get_view_rect;
    handler->on_cursor_change = on_cursor_change;
    handler->on_paint = on_paint;
    handler->on_popup_show = on_popup_show;
    handler->on_popup_size = on_popup_size;
    handler->on_scroll_offset_changed = on_scroll_offset_changed;
    handler->start_dragging = start_dragging;
    handler->update_drag_cursor = update_drag_cursor;

    handler->base.add_ref((cef_base_t *)h);

    return handler;
#else
    return NULL;
#endif
}

///
// Return the handler for browser request events.
///
struct _cef_request_handler_t* CEF_CALLBACK get_request_handler(
        struct _cef_client_t* self) {
    DEBUG_CALLBACK("get_request_handler\n");
    return NULL;
}

///
// Called when a new message is received from a different process. Return true
// (1) if the message was handled or false (0) otherwise. Do not keep a
// reference to or attempt to access the message outside of this callback.
///
int CEF_CALLBACK on_process_message_received(
        struct _cef_client_t* self,
        struct _cef_browser_t* browser, cef_process_id_t source_process,
        struct _cef_process_message_t* message) {
    DEBUG_CALLBACK("on_process_message_received\n");
    int success;
    cef_string_userfree_t name = message->get_name(message);
    cef_string_utf8_t out = {};
    cef_string_utf16_to_utf8(name->str, name->length, &out);
    cef_string_userfree_free(name);
    if (strcmp(out.str, "InvocationResult") == 0) {
	    client_t *client = (client_t *)self;
	    cef_string_userfree_utf8_t result = NULL;

	    cef_list_value_t *arguments = message->get_argument_list(message);

	    cef_string_userfree_t value;
	    value = arguments->get_string(arguments, 0);

	    if (value != NULL) {
		    result = cef_string_userfree_utf8_alloc();
		    cef_string_utf16_to_utf8(value->str, value->length, result);
		    cef_string_userfree_free(value);
	    }

	    client->context->finish(client->context, result);

	    success = 1;
    } else {
	    success = 0;
    }
    cef_string_utf8_clear(&out);

    return success;
}

void initialize_client_handler(client_t* c) {
    DEBUG_CALLBACK("initialize_client_handler\n");
    cef_client_t *client = (cef_client_t *)c;
    initialize_cef_base(c);
    // callbacks
    client->get_context_menu_handler = get_context_menu_handler;
    client->get_dialog_handler = get_dialog_handler;
    client->get_display_handler = get_display_handler;
    client->get_download_handler = get_download_handler;
    client->get_drag_handler = get_drag_handler;
    client->get_focus_handler = get_focus_handler;
    client->get_geolocation_handler = get_geolocation_handler;
    client->get_jsdialog_handler = get_jsdialog_handler;
    client->get_keyboard_handler = get_keyboard_handler;
    client->get_life_span_handler = get_life_span_handler;
    client->get_load_handler = get_load_handler;
    client->get_render_handler = get_render_handler;
    client->get_request_handler = get_request_handler;
    client->on_process_message_received = on_process_message_received;
}
