#include <stdio.h>
#include <stdlib.h>

#include "context.h"
#include "command.h"

static
void
CEF_CALLBACK
execute(cef_task_t *self)
{
	Task *t = ((Task *)self);
	if (t->context->browser->is_loading(t->context->browser)) {
		fprintf(stderr, "Blocking response on page load\n");
		Response *response = calloc(1, sizeof(Response));
		response->message = t->message;
		t->context->pending_response = response;
		return;
	}

	printf("ok\n");

	if (t->message != NULL) {
		printf("%zu\n", t->message->length);
		printf("%s", t->message->str);
		fprintf(stderr, "Wrote response true \"%s\"\n", t->message->str);
		cef_string_userfree_utf8_free(t->message);
	} else {
		printf("0\n");
		fprintf(stderr, "Wrote response true \"\"\n");
	}

	fflush(stdout);
}

static
void finish(Context *self, cef_string_userfree_utf8_t message)
{
	fprintf(stderr, "Command finished with response Success(%s)\n",
	    message ? message->str : "");
	Task *t = calloc(1, sizeof(Task));
	t->context = self;
	t->message = message;
	((cef_task_t *)t)->base.size = sizeof(Task);
	((cef_task_t *)t)->execute = execute;
	cef_post_task(TID_UI, (cef_task_t *)t);
}

static
void
handle_load_event(Context *self)
{
	if (self->pending_response != NULL) {
		fprintf(stderr, "Finishing pending response\n");
		Response *response = self->pending_response;
		self->pending_response = NULL;
		self->finish(self, response->message);
	}
}

void initialize_context(Context *context)
{
    context->finish = finish;
    context->on_load_end = handle_load_event;
}
