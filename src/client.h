#pragma once

typedef struct {
	cef_client_t client;
	atomic_int ref_count;
	Context *context;
} client_t;
