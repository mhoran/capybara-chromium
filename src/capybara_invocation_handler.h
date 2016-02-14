#include <stdatomic.h>

#include "include/capi/cef_v8_capi.h"

typedef struct _capybara_invocation_handler {
	cef_v8handler_t handler;
	atomic_int ref_count;
} capybara_invocation_handler;

typedef struct {
	cef_task_t task;
	cef_browser_t *browser;
	cef_process_message_t *message;
} BrowserMessageTask;

void initialize_capybara_invocation_handler(capybara_invocation_handler* handler);
