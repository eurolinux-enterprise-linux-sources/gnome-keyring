/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-prompt-test.h"

static void start_tests (void) {
	start_setup_daemon ();
}

static void stop_tests (void) {
	stop_setup_daemon ();
}

static void initialize_tests (void) {
	g_test_add("/login-prompt/create_unlock_login", int, NULL, NULL, test_create_unlock_login, NULL);
	g_test_add("/login-prompt/auto_keyring", int, NULL, NULL, test_auto_keyring, NULL);
	g_test_add("/login-prompt/auto_keyring_stale", int, NULL, NULL, test_auto_keyring_stale, NULL);
	g_test_add("/keyrings-prompt/stash_default", int, NULL, NULL, test_stash_default, NULL);
	g_test_add("/keyrings-prompt/create_prompt_keyring", int, NULL, NULL, test_create_prompt_keyring, NULL);
	g_test_add("/keyrings-prompt/change_prompt_keyring", int, NULL, NULL, test_change_prompt_keyring, NULL);
	g_test_add("/keyrings-prompt/acls", int, NULL, NULL, test_acls, NULL);
	g_test_add("/keyrings-prompt/application_secret", int, NULL, NULL, test_application_secret, NULL);
	g_test_add("/keyrings-prompt/unlock_prompt", int, NULL, NULL, test_unlock_prompt, NULL);
	g_test_add("/keyrings-prompt/find_locked", int, NULL, NULL, test_find_locked, NULL);
	g_test_add("/keyrings-prompt/get_info_locked", int, NULL, NULL, test_get_info_locked, NULL);
	g_test_add("/keyrings-prompt/cleanup", int, NULL, NULL, test_cleanup, NULL);
}

#include "tests/gtest-helpers.c"
