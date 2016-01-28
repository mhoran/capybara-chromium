#include "context.h"

typedef struct _Command {
	char **arguments;
	void (*run)(struct _Command *self, Context *context);
} Command;

void initialize_visit_command(Command *command, char *arguments[]);
void initialize_body_command(Command *command, char *arguments[]);
void initialize_find_css_command(Command *command, char *arguments[]);
void initialize_node_command(Command *command, char *arguments[]);
