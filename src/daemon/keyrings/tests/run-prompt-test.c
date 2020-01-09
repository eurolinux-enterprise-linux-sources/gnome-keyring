/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-prompt-test.h"

static void start_tests (void) {
	start_keyrings_login ();
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/login-prompt/keyrings_login", int, NULL, NULL, test_keyrings_login, NULL);
}

#include "tests/gtest-helpers.c"
