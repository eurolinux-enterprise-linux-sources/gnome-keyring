/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-prompt-test.h"

static void start_tests (void) {
	start_setup_pam ();
}

static void stop_tests (void) {
	stop_setup_pam ();
}

static void initialize_tests (void) {
	g_test_add("/pam/pam_open", int, NULL, NULL, test_pam_open, NULL);
	g_test_add("/pam/pam_env", int, NULL, NULL, test_pam_env, NULL);
	g_test_add("/pam/pam_close", int, NULL, NULL, test_pam_close, NULL);
}

#include "tests/gtest-helpers.c"
