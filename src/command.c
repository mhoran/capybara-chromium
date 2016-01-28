#include <string.h>
#include <stdlib.h>

#include "command.h"
#include "string_visitor.h"
#include "cef_base.h"

static
void
handle_load_event()
{
	printf("ok\n");
	printf("0\n");
	fflush(stdout);
}

static
void
run_visit_command(Command *self, Context *context)
{
	cef_string_t url = {};
	cef_string_utf8_to_utf16(self->arguments[0], strlen(self->arguments[0]), &url);
	cef_frame_t *frame = context->browser->get_main_frame(context->browser);
	context->on_load_end = handle_load_event;
	frame->load_url(frame, &url);
	cef_string_clear(&url);
}

void
initialize_visit_command(Command *command, char *arguments[])
{
	command->arguments = arguments;
	command->run = run_visit_command;
}

IMPLEMENT_REFCOUNTING(string_visitor)
GENERATE_CEF_BASE_INITIALIZER(string_visitor)

static
void CEF_CALLBACK get_frame_source(struct _cef_string_visitor_t* self,
    const cef_string_t* string) {
	cef_string_userfree_utf8_t out = cef_string_userfree_utf8_alloc();
	cef_string_utf16_to_utf8(string->str, string->length, out);
	((string_visitor *)self)->context->finish(out);
}

static
void
run_body_command(Command *self, Context *context)
{
	string_visitor *v;
	v = calloc(1, sizeof(string_visitor));
	cef_string_visitor_t *visitor = (cef_string_visitor_t *)v;
	initialize_cef_base(v);
	v->context = context;
	visitor->visit = get_frame_source;
	visitor->base.add_ref((cef_base_t *)v);
	cef_frame_t *frame = context->browser->get_main_frame(context->browser);
	frame->get_source(frame, visitor);
}

void
initialize_body_command(Command *command, char *arguments[])
{
	command->arguments = arguments;
	command->run = run_body_command;
}

static
void
run_find_css_command(Command *self, Context *context)
{
	fprintf(stderr, "Received FindCss\n");
	cef_string_t name = {};
	cef_string_set(u"CapybaraInvocation", 18, &name, 0);
	cef_process_message_t *message = cef_process_message_create(&name);

	cef_list_value_t *args = message->get_argument_list(message);

	cef_string_t value = {};
	cef_string_set(u"findCss", 7, &value, 0);
	args->set_string(args, 0, &value);

	args->set_bool(args, 1, 1);

	cef_string_utf8_to_utf16(self->arguments[0], strlen(self->arguments[0]), &value);
	args->set_string(args, 2, &value);
	cef_string_clear(&value);

	context->browser->send_process_message(context->browser, PID_RENDERER, message);
}

void
initialize_find_css_command(Command *command, char *arguments[])
{
	command->arguments = arguments;
	command->run = run_find_css_command;
}

static
void
run_node_command(Command *self, Context *context)
{
	fprintf(stderr, "Received Node\n");
	cef_string_t name = {};
	cef_string_set(u"CapybaraInvocation", 18, &name, 0);
	cef_process_message_t *message = cef_process_message_create(&name);

	cef_list_value_t *args = message->get_argument_list(message);

	cef_string_t value = {};
	cef_string_utf8_to_utf16(self->arguments[0], strlen(self->arguments[0]), &value);
	args->set_string(args, 0, &value);

	args->set_bool(args, 1, strcmp(self->arguments[1], "true") == 0);

	cef_string_utf8_to_utf16(self->arguments[2], strlen(self->arguments[2]), &value);
	args->set_string(args, 2, &value);
	cef_string_clear(&value);

	context->browser->send_process_message(context->browser, PID_RENDERER, message);
}

void
initialize_node_command(Command *command, char *arguments[])
{
	command->arguments = arguments;
	command->run = run_node_command;
}