#pragma once

typedef struct {
	cef_browser_t *browser;
	void (*on_load_end)();
} Context;
