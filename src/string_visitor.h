#pragma once

typedef struct {
	cef_string_visitor_t visitor;
	atomic_int ref_count;
} string_visitor;
