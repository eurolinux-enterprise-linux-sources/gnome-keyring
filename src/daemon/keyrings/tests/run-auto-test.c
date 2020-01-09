/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/keyring-login/keyrings_login", int, NULL, setup_keyrings_login, test_keyrings_login, NULL);
	g_test_add("/keyring-login/keyrings_login_master", int, NULL, setup_keyrings_login, test_keyrings_login_master, NULL);
	g_test_add("/keyring-login/keyrings_login_secrets", int, NULL, setup_keyrings_login, test_keyrings_login_secrets, NULL);
	g_test_add("/keyring-file/keyring_parse_encrypted", int, NULL, NULL, test_keyring_parse_encrypted, NULL);
	g_test_add("/keyring-file/keyring_parse_plain", int, NULL, NULL, test_keyring_parse_plain, NULL);
	g_test_add("/keyring-file/keyring_double_lock_encrypted", int, NULL, NULL, test_keyring_double_lock_encrypted, NULL);
	g_test_add("/keyring-file/keyring_double_lock_plain", int, NULL, NULL, test_keyring_double_lock_plain, NULL);
}

#include "tests/gtest-helpers.c"
