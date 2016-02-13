#pragma once

#include <stdatomic.h>

#include "include/capi/cef_render_handler_capi.h"

#include "context.h"

typedef struct _render_handler {
	cef_render_handler_t handler;
	Context *context;
	atomic_int ref_count;
} render_handler;

int
CEF_CALLBACK
get_root_screen_rect(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, cef_rect_t* rect);

int
CEF_CALLBACK
get_view_rect(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, cef_rect_t* rect);

int
CEF_CALLBACK
get_screen_point(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, int viewX, int viewY, int* screenX,
    int* screenY);

int
CEF_CALLBACK
get_screen_info(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_screen_info_t* screen_info);

void
CEF_CALLBACK
on_popup_show(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, int show);

void
CEF_CALLBACK
on_popup_size(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, const cef_rect_t* rect);

void
CEF_CALLBACK
on_paint(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, cef_paint_element_type_t type,
    size_t dirtyRectsCount, cef_rect_t const* dirtyRects, const void* buffer,
    int width, int height);

void
CEF_CALLBACK
on_cursor_change(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, cef_cursor_handle_t cursor,
    cef_cursor_type_t type,
    const struct _cef_cursor_info_t* custom_cursor_info);

int
CEF_CALLBACK
start_dragging(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_drag_data_t* drag_data,
    cef_drag_operations_mask_t allowed_ops, int x, int y);

void
CEF_CALLBACK
update_drag_cursor(struct _cef_render_handler_t* self,
    struct _cef_browser_t* browser, cef_drag_operations_mask_t operation);

void
CEF_CALLBACK
on_scroll_offset_changed(
    struct _cef_render_handler_t* self, struct _cef_browser_t* browser,
    double x, double y);
