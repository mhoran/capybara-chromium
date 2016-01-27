// Copyright (c) 2014 The cefcapi authors. All rights reserved.
// License: BSD 3-clause.
// Website: https://github.com/CzarekTomczak/cefcapi

#pragma once

#include <stdatomic.h>

#include "include/capi/cef_base_capi.h"
#include "include/capi/cef_app_capi.h"

#include "cef_life_span_handler.h"
#include "client.h"

// Set to 1 to check if add_ref() and release()
// are called and to track the total number of calls.
// add_ref will be printed as "+", release as "-".
#define DEBUG_REFERENCE_COUNTING 0

// Print only the first execution of the callback,
// ignore the subsequent.
#define DEBUG_CALLBACK(x) { static int first_call = 1; if (first_call == 1) { first_call = 0; fprintf(stderr, x); } }

// ----------------------------------------------------------------------------
// cef_base_t
// ----------------------------------------------------------------------------

///
// Structure defining the reference count implementation functions. All
// framework structures must include the cef_base_t structure first.
///

///
// Increment the reference count.
///
void CEF_CALLBACK add_ref(cef_base_t* self) {
    DEBUG_CALLBACK("cef_base_t.add_ref\n");
    if (DEBUG_REFERENCE_COUNTING)
        printf("+");
}

///
// Decrement the reference count.  Delete this object when no references
// remain.
///
int CEF_CALLBACK release(cef_base_t* self) {
    DEBUG_CALLBACK("cef_base_t.release\n");
    if (DEBUG_REFERENCE_COUNTING)
        printf("-");
    return 1;
}

///
// Returns the current number of references.
///
int CEF_CALLBACK has_one_ref(cef_base_t* self) {
    DEBUG_CALLBACK("cef_base_t.get_refct\n");
    if (DEBUG_REFERENCE_COUNTING)
        printf("=");
    return 1;
}

#define ADD_REF(type) \
void \
CEF_CALLBACK \
type##_add_ref(cef_base_t *self) \
{ \
	type *handler = (type *)self; \
	atomic_fetch_add(&handler->ref_count, 1); \
}
ADD_REF(life_span_handler_t)
ADD_REF(client_t)

#define RELEASE(type) \
int \
CEF_CALLBACK \
type##_release(cef_base_t* self) { \
	type *handler = (type *)self; \
	if (atomic_fetch_sub(&handler->ref_count, 1) - 1 == 0) { \
		free(handler); \
		return 1; \
	} \
	return 0; \
}
RELEASE(life_span_handler_t)
RELEASE(client_t)

#define HAS_ONE_REF(type) \
int \
CEF_CALLBACK \
type##_has_one_ref(cef_base_t* self) { \
	type *handler = (type *)self; \
	return atomic_load(&handler->ref_count) == 1; \
}
HAS_ONE_REF(life_span_handler_t)
HAS_ONE_REF(client_t)

void
_initialize_cef_base(cef_base_t* base,
    void (CEF_CALLBACK *add_ref)(struct _cef_base_t* self),
    int (CEF_CALLBACK *release)(struct _cef_base_t* self),
    int (CEF_CALLBACK *has_one_ref)(struct _cef_base_t* self))
{
	size_t size = base->size;
	if (size <= 0) {
		fprintf(stderr, "FATAL: initialize_cef_base failed, size member not set\n");
		exit(1);
	}
	base->add_ref = add_ref;
	base->release = release;
	base->has_one_ref = has_one_ref;
}

#define INITIALIZE_CEF_BASE_FOR_TYPE(type, base) \
_initialize_cef_base((cef_base_t *)base, type##_add_ref, type##_release, type##_has_one_ref)

#define initialize_cef_base(T) \
    _Generic((T), \
	life_span_handler_t*: INITIALIZE_CEF_BASE_FOR_TYPE(life_span_handler_t, T), \
	client_t*: INITIALIZE_CEF_BASE_FOR_TYPE(client_t, T), \
	default: _initialize_cef_base((cef_base_t *)T, add_ref, release, has_one_ref))
