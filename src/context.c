#include <stdio.h>

#include "context.h"

static
void finish(cef_string_utf8_t *message)
{
	printf("ok\n");

	if (message != NULL) {
		printf("%zu\n", message->length);
		printf("%s", message->str);
		cef_string_userfree_utf8_free(message);
	} else
		printf("0\n");

	fflush(stdout);
}

void initialize_context(Context *context)
{
    context->finish = finish;
}
