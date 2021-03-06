#include "cef_load_handler.h"
#include "cef_base.h"
#include "context.h"

IMPLEMENT_REFCOUNTING(load_handler)
GENERATE_CEF_BASE_INITIALIZER(load_handler)

///
// Implement this structure to handle events related to browser load status. The
// functions of this structure will be called on the browser process UI thread
// or render process main thread (TID_RENDERER).
///

///
// Called when the loading state has changed. This callback will be executed
// twice -- once when loading is initiated either programmatically or by user
// action, and once when loading is terminated due to completion, cancellation
// of failure.
///
void CEF_CALLBACK on_loading_state_change(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, int isLoading, int canGoBack,
    int canGoForward)
{
	if (isLoading == 1) {
		fprintf(stderr, "Load started\n");
	} else {
		fprintf(stderr, "Load finished\n");
		load_handler *handler;
		handler = (load_handler *)self;
		handler->context->on_load_end(handler->context);
	}
}

///
// Called when the browser begins loading a frame. The |frame| value will
// never be NULL -- call the is_main() function to check if this frame is the
// main frame. Multiple frames may be loading at the same time. Sub-frames may
// start or continue loading after the main frame load has ended. This
// function may not be called for a particular frame if the load request for
// that frame fails. For notification of overall browser load status use
// OnLoadingStateChange instead.
///
void CEF_CALLBACK on_load_start(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame) {}

///
// Called when the browser is done loading a frame. The |frame| value will
// never be NULL -- call the is_main() function to check if this frame is the
// main frame. Multiple frames may be loading at the same time. Sub-frames may
// start or continue loading after the main frame load has ended. This
// function will always be called for all frames irrespective of whether the
// request completes successfully.
///
void CEF_CALLBACK on_load_end(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    int httpStatusCode) {}

///
// Called when the resource load for a navigation fails or is canceled.
// |errorCode| is the error code number, |errorText| is the error text and
// |failedUrl| is the URL that failed to load. See net\base\net_error_list.h
// for complete descriptions of the error codes.
///
void CEF_CALLBACK on_load_error(struct _cef_load_handler_t* self,
    struct _cef_browser_t* browser, struct _cef_frame_t* frame,
    cef_errorcode_t errorCode, const cef_string_t* errorText,
    const cef_string_t* failedUrl) {}
