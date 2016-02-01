struct _Context;

typedef struct _Command {
	char **arguments;
	void (*run)(struct _Command *self, struct _Context *context);
} Command;

void initialize_visit_command(Command *command, char *arguments[]);
void initialize_body_command(Command *command, char *arguments[]);
void initialize_find_css_command(Command *command, char *arguments[]);
void initialize_node_command(Command *command, char *arguments[]);
void initialize_find_xpath_command(Command *command, char *arguments[]);
void initialize_reset_command(Command *command, char *arguments[]);
