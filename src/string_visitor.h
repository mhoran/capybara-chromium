#pragma once

#include <stdatomic.h>

typedef struct _string_visitor {
	cef_string_visitor_t visitor;
	atomic_int ref_count;
	Context *context;
} string_visitor;
