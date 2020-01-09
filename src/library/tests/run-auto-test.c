/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
	start_setup_daemon ();
}

static void stop_tests (void) {
	stop_setup_daemon ();
}

static void initialize_tests (void) {
	g_test_add("/memory/alloc_free", int, NULL, NULL, test_alloc_free, NULL);
	g_test_add("/memory/alloc_two", int, NULL, NULL, test_alloc_two, NULL);
	g_test_add("/memory/realloc", int, NULL, NULL, test_realloc, NULL);
	g_test_add("/memory/realloc_across", int, NULL, NULL, test_realloc_across, NULL);
	g_test_add("/keyrings/remove_incomplete", int, NULL, NULL, test_remove_incomplete, NULL);
	g_test_add("/keyrings/create_keyring", int, NULL, NULL, test_create_keyring, NULL);
	g_test_add("/keyrings/set_default_keyring", int, NULL, NULL, test_set_default_keyring, NULL);
	g_test_add("/keyrings/delete_keyring", int, NULL, NULL, test_delete_keyring, NULL);
	g_test_add("/keyrings/recreate_keyring", int, NULL, NULL, test_recreate_keyring, NULL);
	g_test_add("/keyrings/create_list_items", int, NULL, NULL, test_create_list_items, NULL);
	g_test_add("/keyrings/find_keyrings", int, NULL, NULL, test_find_keyrings, NULL);
	g_test_add("/keyrings/find_invalid", int, NULL, NULL, test_find_invalid, NULL);
	g_test_add("/keyrings/lock_keyrings", int, NULL, NULL, test_lock_keyrings, NULL);
	g_test_add("/keyrings/change_password", int, NULL, NULL, test_change_password, NULL);
	g_test_add("/keyrings/keyring_info", int, NULL, NULL, test_keyring_info, NULL);
	g_test_add("/keyrings/list_keyrings", int, NULL, NULL, test_list_keyrings, NULL);
	g_test_add("/keyrings/keyring_grant_access", int, NULL, NULL, test_keyring_grant_access, NULL);
	g_test_add("/keyrings/store_password", int, NULL, NULL, test_store_password, NULL);
	g_test_add("/keyrings/find_password", int, NULL, NULL, test_find_password, NULL);
	g_test_add("/keyrings/delete_password", int, NULL, NULL, test_delete_password, NULL);
	g_test_add("/keyrings/cleanup", int, NULL, NULL, test_cleanup, NULL);
	g_test_add("/other/set_display", int, NULL, NULL, test_set_display, NULL);
	g_test_add("/other/setup_environment", int, NULL, NULL, test_setup_environment, NULL);
	g_test_add("/other/result_string", int, NULL, NULL, test_result_string, NULL);
	g_test_add("/other/is_available", int, NULL, NULL, test_is_available, NULL);
}

#include "tests/gtest-helpers.c"
