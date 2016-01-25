#pragma once

#include "include/capi/cef_render_process_handler_capi.h"

///
// Structure used to implement render process callbacks. The functions of this
// structure will be called on the render process main thread (TID_RENDERER)
// unless otherwise indicated.
///

///
// Called after the render process main thread has been created. |extra_info|
// is a read-only value originating from
// cef_browser_process_handler_t::on_render_process_thread_created(). Do not
// keep a reference to |extra_info| outside of this function.
///
void
CEF_CALLBACK
on_render_thread_created(
    struct _cef_render_process_handler_t* self,
    struct _cef_list_value_t* extra_info)
{ };

static cef_string_t *loadJavascript();
///
// Called after WebKit has been initialized.
///
void
CEF_CALLBACK
on_web_kit_initialized(
    struct _cef_render_process_handler_t* self)
{
	cef_string_t extension_name = {};
	cef_string_utf8_to_utf16("Capybara", 8, &extension_name);
	cef_register_extension(&extension_name, loadJavascript(), NULL);
};

///
// Called after a browser has been created. When browsing cross-origin a new
// browser will be created before the old browser with the same identifier is
// destroyed.
///
void
CEF_CALLBACK
on_browser_created(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser)
{ };

///
// Called before a browser is destroyed.
///
void
CEF_CALLBACK
on_browser_destroyed(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser)
{ };

///
// Return the handler for browser load status events.
///
struct
_cef_load_handler_t*
CEF_CALLBACK
get_render_process_load_handler(
    struct _cef_render_process_handler_t* self)
{
	return NULL;
};

///
// Called before browser navigation. Return true (1) to cancel the navigation
// or false (0) to allow the navigation to proceed. The |request| object
// cannot be modified in this callback.
///
int
CEF_CALLBACK
on_before_navigation(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_request_t* request, cef_navigation_type_t navigation_type,
    int is_redirect)
{
	return 0;
};

///
// Called immediately after the V8 context for a frame has been created. To
// retrieve the JavaScript 'window' object use the
// cef_v8context_t::get_global() function. V8 handles can only be accessed
// from the thread on which they are created. A task runner for posting tasks
// on the associated thread can be retrieved via the
// cef_v8context_t::get_task_runner() function.
///
void
CEF_CALLBACK
on_context_created(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_v8context_t* context)
{ };

///
// Called immediately before the V8 context for a frame is released. No
// references to the context should be kept after this function is called.
///
void
CEF_CALLBACK
on_context_released(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_v8context_t* context)
{ };

///
// Called for global uncaught exceptions in a frame. Execution of this
// callback is disabled by default. To enable set
// CefSettings.uncaught_exception_stack_size > 0.
///
void
CEF_CALLBACK
on_uncaught_exception(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_v8context_t* context, struct _cef_v8exception_t* exception,
    struct _cef_v8stack_trace_t* stackTrace)
{ };

///
// Called when a new node in the the browser gets focus. The |node| value may
// be NULL if no specific node has gained focus. The node object passed to
// this function represents a snapshot of the DOM at the time this function is
// executed. DOM objects are only valid for the scope of this function. Do not
// keep references to or attempt to access any DOM objects outside the scope
// of this function.
///
void
CEF_CALLBACK
on_focused_node_changed(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    struct _cef_domnode_t* node)
{ };

void
CEF_CALLBACK
handle_invocation_result(struct _cef_v8value_t* object)
{
	cef_string_t *value = object->get_string_value(object);

	cef_string_utf8_t out = {};
	if (value != NULL)
		cef_string_utf16_to_utf8(value->str, value->length, &out);

	printf("ok\n");
	if (out.length > 0) {
		printf("%zu\n", out.length);
		printf("%s", out.str);
	} else
		printf("0\n");

	fflush(stdout);
};

///
// Called when a new message is received from a different process. Return true
// (1) if the message was handled or false (0) otherwise. Do not keep a
// reference to or attempt to access the message outside of this callback.
///
int
CEF_CALLBACK
on_render_process_message_received(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser, cef_process_id_t source_process,
    struct _cef_process_message_t* message)
{
	cef_string_t *name = message->get_name(message);
	cef_string_utf8_t out = {};
	cef_string_utf16_to_utf8(name->str, name->length, &out);
	if (strcmp(out.str, "CapybaraInvocation") == 0) {
		cef_list_value_t *arguments = message->get_argument_list(message);

		cef_frame_t *frame = browser->get_main_frame(browser);
		cef_v8context_t *context = frame->get_v8context(frame);
		context->enter(context);

		cef_v8value_t *invocation = cef_v8value_create_object(NULL);

		cef_v8value_t *function_name = cef_v8value_create_string(arguments->get_string(arguments, 0));
		cef_string_t function_name_key = {};
		cef_string_utf8_to_utf16("functionName", 12, &function_name_key);
		invocation->set_value_bykey(invocation, &function_name_key, function_name, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_v8value_t *allow_unattached = cef_v8value_create_bool(arguments->get_bool(arguments, 1));
		cef_string_t allow_unattached_key = {};
		cef_string_utf8_to_utf16("allowUnattached", 15, &allow_unattached_key);
		invocation->set_value_bykey(invocation, &allow_unattached_key, allow_unattached, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_v8value_t *invocation_arguments = cef_v8value_create_array(1);
		cef_v8value_t *argument = cef_v8value_create_string(arguments->get_string(arguments, 2));
		invocation_arguments->set_value_byindex(invocation_arguments, 0, argument);

		cef_string_t arguments_key = {};
		cef_string_utf8_to_utf16("arguments", 9, &arguments_key);
		invocation->set_value_bykey(invocation, &arguments_key, invocation_arguments, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_v8value_t *object = context->get_global(context);

		cef_string_t capybara_invocation_key = {};
		cef_string_utf8_to_utf16("CapybaraInvocation", 18, &capybara_invocation_key);
		object->set_value_bykey(object, &capybara_invocation_key, invocation, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_string_t capybara_invoke = {};
		cef_string_utf8_to_utf16("Capybara.invoke()", 17, &capybara_invoke);

		cef_v8value_t *retval = NULL;
		cef_v8exception_t *exception = NULL;
		if (context->eval(context, &capybara_invoke, &retval, &exception))
			handle_invocation_result(retval);

		context->exit(context);

		return 1;
	} else {
		return 0;
	}
};
