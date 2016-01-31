#include <string.h>
#include <stdio.h>

#include "include/capi/cef_render_process_handler_capi.h"

#include "cef_render_process_handler.h"
#include "cef_base.h"

IMPLEMENT_REFCOUNTING(render_process_handler)
GENERATE_CEF_BASE_INITIALIZER(render_process_handler)

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
{ }

static
cef_string_t *
loadJavascript() {
	static cef_string_t *m_capybaraJavascript = NULL;

	if (m_capybaraJavascript != NULL)
		return m_capybaraJavascript;

	extern char _binary_src_capybara_js_start;
	extern char _binary_src_capybara_js_end;

	int size = (char *)&_binary_src_capybara_js_end - (char *)&_binary_src_capybara_js_start;

	m_capybaraJavascript = calloc(1, sizeof(cef_string_t));
	cef_string_utf8_to_utf16(&_binary_src_capybara_js_start, size, m_capybaraJavascript);
	return m_capybaraJavascript;
}

///
// Called after WebKit has been initialized.
///
void
CEF_CALLBACK
on_web_kit_initialized(
    struct _cef_render_process_handler_t* self)
{
	cef_string_t extension_name = {};
	cef_string_set(u"Capybara", 8, &extension_name, 0);
	cef_register_extension(&extension_name, loadJavascript(), NULL);
}

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
{ }

///
// Called before a browser is destroyed.
///
void
CEF_CALLBACK
on_browser_destroyed(
    struct _cef_render_process_handler_t* self,
    struct _cef_browser_t* browser)
{ }

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
}

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
}

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
{ }

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
{ }

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
{ }

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
{ }

void
CEF_CALLBACK
handle_invocation_result(struct _cef_browser_t *browser, struct _cef_v8value_t* object)
{
	cef_string_userfree_t value = object->get_string_value(object);

	cef_string_t name = {};
	cef_string_set(u"InvocationResult", 16, &name, 0);
	cef_process_message_t *message = cef_process_message_create(&name);

	cef_list_value_t *args = message->get_argument_list(message);

	args->set_string(args, 0, value);

	browser->send_process_message(browser, PID_BROWSER, message);
}

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
	int success;
	cef_string_userfree_t name = message->get_name(message);
	cef_string_utf8_t out = {};
	cef_string_utf16_to_utf8(name->str, name->length, &out);
	cef_string_userfree_free(name);
	if (strcmp(out.str, "CapybaraInvocation") == 0) {
		cef_list_value_t *arguments = message->get_argument_list(message);

		cef_frame_t *frame = browser->get_main_frame(browser);
		cef_v8context_t *context = frame->get_v8context(frame);
		frame->base.release((cef_base_t* )frame);
		context->enter(context);

		cef_v8value_t *invocation = cef_v8value_create_object(NULL);

		cef_string_userfree_t s;
		s = arguments->get_string(arguments, 0);
		cef_v8value_t *function_name = cef_v8value_create_string(s);
		cef_string_userfree_free(s);
		cef_string_t key = {};
		cef_string_set(u"functionName", 12, &key, 0);
		invocation->set_value_bykey(invocation, &key, function_name, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_v8value_t *allow_unattached = cef_v8value_create_bool(arguments->get_bool(arguments, 1));
		cef_string_set(u"allowUnattached", 15, &key, 0);
		invocation->set_value_bykey(invocation, &key, allow_unattached, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_v8value_t *invocation_arguments = cef_v8value_create_array(1);
		s = arguments->get_string(arguments, 2);
		cef_v8value_t *argument = cef_v8value_create_string(s);
		cef_string_userfree_free(s);
		invocation_arguments->set_value_byindex(invocation_arguments, 0, argument);

		cef_string_set(u"arguments", 9, &key, 0);
		invocation->set_value_bykey(invocation, &key, invocation_arguments, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_v8value_t *object = context->get_global(context);

		cef_string_set(u"CapybaraInvocation", 18, &key, 0);
		object->set_value_bykey(object, &key, invocation, V8_PROPERTY_ATTRIBUTE_NONE);

		cef_string_t script = {};
		cef_string_set(u"Capybara.invoke()", 17, &script, 0);

		cef_v8value_t *retval = NULL;
		cef_v8exception_t *exception = NULL;
		if (context->eval(context, &script, &retval, &exception))
			handle_invocation_result(browser, retval);

		context->exit(context);
		context->base.release((cef_base_t *)context);

		success = 1;
	} else {
		success = 0;
	}
	cef_string_utf8_clear(&out);

	return success;
}
