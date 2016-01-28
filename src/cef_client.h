// Copyright (c) 2014 The cefcapi authors. All rights reserved.
// License: BSD 3-clause.
// Website: https://github.com/CzarekTomczak/cefcapi

#pragma once

#include "cef_life_span_handler.h"
#include "cef_render_handler.h"
#include "context.h"

typedef struct _client_t {
	cef_client_t client;
	atomic_int ref_count;
	Context *context;
} client_t;

struct _cef_context_menu_handler_t* CEF_CALLBACK get_context_menu_handler(
        struct _cef_client_t* self);

struct _cef_dialog_handler_t* CEF_CALLBACK get_dialog_handler(
        struct _cef_client_t* self);

struct _cef_display_handler_t* CEF_CALLBACK get_display_handler(
        struct _cef_client_t* self);

struct _cef_download_handler_t* CEF_CALLBACK get_download_handler(
        struct _cef_client_t* self);

struct _cef_drag_handler_t* CEF_CALLBACK get_drag_handler(
        struct _cef_client_t* self);

struct _cef_focus_handler_t* CEF_CALLBACK get_focus_handler(
        struct _cef_client_t* self);

struct _cef_geolocation_handler_t* CEF_CALLBACK get_geolocation_handler(
        struct _cef_client_t* self);

struct _cef_jsdialog_handler_t* CEF_CALLBACK get_jsdialog_handler(
        struct _cef_client_t* self);

struct _cef_keyboard_handler_t* CEF_CALLBACK get_keyboard_handler(
        struct _cef_client_t* self);

struct _cef_life_span_handler_t* CEF_CALLBACK get_life_span_handler(
        struct _cef_client_t* self);

struct _cef_load_handler_t* CEF_CALLBACK get_load_handler(
        struct _cef_client_t* self);

struct _cef_render_handler_t* CEF_CALLBACK get_render_handler(
        struct _cef_client_t* self);

struct _cef_request_handler_t* CEF_CALLBACK get_request_handler(
        struct _cef_client_t* self);

int CEF_CALLBACK on_process_message_received(
        struct _cef_client_t* self,
        struct _cef_browser_t* browser, cef_process_id_t source_process,
        struct _cef_process_message_t* message);

void initialize_client_handler(client_t* c);
