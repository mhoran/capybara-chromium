#include <stdlib.h>

#include "command.h"
#include "context.h"

typedef struct {
	cef_task_t task;
	Context *context;
} ResetTask;

static void
execute_reset(cef_task_t *self)
{
    	ResetTask *task = (ResetTask *)self;

	cef_window_info_t windowInfo = {};
#ifdef WINDOWLESS
	windowInfo.windowless_rendering_enabled = 1;
#endif

	cef_browser_settings_t browserSettings = {};
	browserSettings.size = sizeof(cef_browser_settings_t);

	cef_string_t url = {};
	cef_string_set(u"about:blank", 11, &url, 0);

	cef_request_context_settings_t settings = {};
	settings.size = sizeof(cef_request_context_settings_t);

	cef_request_context_t *context =
	    cef_request_context_create_context(&settings, NULL);

	task->context->client->base.add_ref((cef_base_t *)task->context->client);
	cef_browser_t *browser = cef_browser_host_create_browser_sync(
	    &windowInfo, task->context->client, &url, &browserSettings,
	    context);
	browser->base.add_ref((cef_base_t *) browser);
	task->context->browser = browser;

	task->context->finish(task->context, NULL);
}

static void
run_reset_command(Command *self, Context *context)
{
	cef_browser_host_t *host = context->browser->get_host(context->browser);
	host->close_browser(host, 1);
	host->base.release((cef_base_t *)host);

	context->width = 1680;
	context->height = 1050;

	ResetTask *task = calloc(1, sizeof(ResetTask));
	task->context = context;
	cef_task_t *t= (cef_task_t *)task;
	t->base.size = sizeof(ResetTask);
	t->execute = execute_reset;
	cef_post_task(TID_UI, t);
}

void
initialize_reset_command(Command *command, char *arguments[])
{
	command->arguments = arguments;
	command->run = run_reset_command;
}
