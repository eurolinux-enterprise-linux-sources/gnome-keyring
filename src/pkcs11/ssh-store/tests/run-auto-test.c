/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/ssh-openssh/parse_public", int, NULL, NULL, test_parse_public, NULL);
	g_test_add("/ssh-openssh/parse_private", int, NULL, NULL, test_parse_private, NULL);
	g_test_add("/private-key/private_key_parse_plain", int, NULL, setup_private_key_setup, test_private_key_parse_plain, teardown_private_key_teardown);
	g_test_add("/private-key/private_key_parse_and_unlock", int, NULL, setup_private_key_setup, test_private_key_parse_and_unlock, teardown_private_key_teardown);
}

#include "tests/gtest-helpers.c"
