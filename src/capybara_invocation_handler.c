#include <string.h>

#include "capybara_invocation_handler.h"

#include "cef_base.h"

IMPLEMENT_REFCOUNTING(capybara_invocation_handler)
GENERATE_CEF_BASE_INITIALIZER(capybara_invocation_handler)

static
void
execute_done_task(cef_task_t *self)
{
	BrowserMessageTask *task = (BrowserMessageTask *)self;
	task->browser->send_process_message(task->browser, PID_BROWSER, task->message);
	task->browser->base.release((cef_base_t *)task->browser);
}

static int CEF_CALLBACK
execute(struct _cef_v8handler_t* self,
    const cef_string_t* name, struct _cef_v8value_t* object,
    size_t argumentsCount, struct _cef_v8value_t* const* arguments,
    struct _cef_v8value_t** retval, cef_string_t* exception)
{
	int success;
	cef_string_utf8_t out = {};
	cef_string_utf16_to_utf8(name->str, name->length, &out);
	if (strcmp(out.str, "clickTest") == 0) {
		cef_v8value_t *result = cef_v8value_create_bool(1);
		*retval = result;
		success = 1;
	} else if (strcmp(out.str, "leftClick") == 0) {
		cef_v8context_t *context = cef_v8context_get_current_context();
		cef_browser_t *browser = context->get_browser(context);

		cef_string_t message_name = {};
		cef_string_set(u"SendMouseClickEvent", 19, &message_name, 0);
		cef_process_message_t *cef_message = cef_process_message_create(&message_name);

		cef_list_value_t *args = cef_message->get_argument_list(cef_message);

		int x = arguments[0]->get_int_value(arguments[0]);
		args->set_int(args, 0, x);
		int y = arguments[0]->get_int_value(arguments[1]);
		args->set_int(args, 1, y);

		browser->send_process_message(browser, PID_BROWSER, cef_message);
		browser->base.release((cef_base_t *)browser);

		success = 1;
	} else if (strcmp(out.str, "done") == 0) {
		cef_string_t name = {};
		cef_string_set(u"RequestInvocationResult", 23, &name, 0);
		cef_process_message_t *message = cef_process_message_create(&name);

		cef_v8context_t *context = cef_v8context_get_current_context();
		cef_browser_t *browser = context->get_browser(context);
		browser->base.add_ref((cef_base_t *)browser);

		BrowserMessageTask *t = calloc(1, sizeof(BrowserMessageTask));
		t->browser = browser;
		t->message = message;
		((cef_task_t *)t)->base.size = sizeof(BrowserMessageTask);
		((cef_task_t *)t)->execute = execute_done_task;
		cef_post_task(TID_RENDERER, (cef_task_t *)t);

		success = 1;
	} else if (strcmp(out.str, "keypress") == 0) {
		cef_v8context_t *context = cef_v8context_get_current_context();
		cef_browser_t *browser = context->get_browser(context);

		cef_string_t message_name = {};
		cef_string_set(u"SendKeyEvent", 12, &message_name, 0);
		cef_process_message_t *cef_message = cef_process_message_create(&message_name);

		cef_list_value_t *args = cef_message->get_argument_list(cef_message);

		int c = arguments[0]->get_int_value(arguments[0]);
		args->set_int(args, 0, c);

		browser->send_process_message(browser, PID_BROWSER, cef_message);
		browser->base.release((cef_base_t *)browser);

		success = 1;
	} else {
		success = 0;
	}
	cef_string_utf8_clear(&out);

	return success;
}

static void
initialize_cef_v8handler(cef_v8handler_t *handler)
{
	handler->execute = execute;
}

void initialize_capybara_invocation_handler(capybara_invocation_handler* handler)
{
    initialize_cef_base(handler);
    initialize_cef_v8handler((cef_v8handler_t *)handler);
}
