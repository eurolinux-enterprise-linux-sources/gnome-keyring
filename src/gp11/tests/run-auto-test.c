/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/gp11-attributes/init_memory", int, NULL, NULL, test_init_memory, NULL);
	g_test_add("/gp11-attributes/init_boolean", int, NULL, NULL, test_init_boolean, NULL);
	g_test_add("/gp11-attributes/init_date", int, NULL, NULL, test_init_date, NULL);
	g_test_add("/gp11-attributes/init_ulong", int, NULL, NULL, test_init_ulong, NULL);
	g_test_add("/gp11-attributes/init_string", int, NULL, NULL, test_init_string, NULL);
	g_test_add("/gp11-attributes/init_invalid", int, NULL, NULL, test_init_invalid, NULL);
	g_test_add("/gp11-attributes/init_empty", int, NULL, NULL, test_init_empty, NULL);
	g_test_add("/gp11-attributes/new_memory", int, NULL, NULL, test_new_memory, NULL);
	g_test_add("/gp11-attributes/new_boolean", int, NULL, NULL, test_new_boolean, NULL);
	g_test_add("/gp11-attributes/new_date", int, NULL, NULL, test_new_date, NULL);
	g_test_add("/gp11-attributes/new_ulong", int, NULL, NULL, test_new_ulong, NULL);
	g_test_add("/gp11-attributes/new_string", int, NULL, NULL, test_new_string, NULL);
	g_test_add("/gp11-attributes/new_invalid", int, NULL, NULL, test_new_invalid, NULL);
	g_test_add("/gp11-attributes/new_empty", int, NULL, NULL, test_new_empty, NULL);
	g_test_add("/gp11-attributes/get_boolean", int, NULL, NULL, test_get_boolean, NULL);
	g_test_add("/gp11-attributes/get_date", int, NULL, NULL, test_get_date, NULL);
	g_test_add("/gp11-attributes/get_ulong", int, NULL, NULL, test_get_ulong, NULL);
	g_test_add("/gp11-attributes/get_string", int, NULL, NULL, test_get_string, NULL);
	g_test_add("/gp11-attributes/dup_attribute", int, NULL, NULL, test_dup_attribute, NULL);
	g_test_add("/gp11-attributes/copy_attribute", int, NULL, NULL, test_copy_attribute, NULL);
	g_test_add("/gp11-attributes/new_attributes", int, NULL, NULL, test_new_attributes, NULL);
	g_test_add("/gp11-attributes/newv_attributes", int, NULL, NULL, test_newv_attributes, NULL);
	g_test_add("/gp11-attributes/new_empty_attributes", int, NULL, NULL, test_new_empty_attributes, NULL);
	g_test_add("/gp11-attributes/new_valist_attributes", int, NULL, NULL, test_new_valist_attributes, NULL);
	g_test_add("/gp11-attributes/add_data_attributes", int, NULL, NULL, test_add_data_attributes, NULL);
	g_test_add("/gp11-attributes/add_attributes", int, NULL, NULL, test_add_attributes, NULL);
	g_test_add("/gp11-attributes/find_attributes", int, NULL, NULL, test_find_attributes, NULL);
	g_test_add("/gp11-module/invalid_modules", int, NULL, setup_load_module, test_invalid_modules, teardown_load_module);
	g_test_add("/gp11-module/module_equals_hash", int, NULL, setup_load_module, test_module_equals_hash, teardown_load_module);
	g_test_add("/gp11-module/module_props", int, NULL, setup_load_module, test_module_props, teardown_load_module);
	g_test_add("/gp11-module/module_info", int, NULL, setup_load_module, test_module_info, teardown_load_module);
	g_test_add("/gp11-module/module_enumerate", int, NULL, setup_load_module, test_module_enumerate, teardown_load_module);
	g_test_add("/gp11-slot/slot_info", int, NULL, setup_load_slots, test_slot_info, teardown_load_slots);
	g_test_add("/gp11-slot/slot_props", int, NULL, setup_load_slots, test_slot_props, teardown_load_slots);
	g_test_add("/gp11-slot/slot_equals_hash", int, NULL, setup_load_slots, test_slot_equals_hash, teardown_load_slots);
	g_test_add("/gp11-slot/slot_mechanisms", int, NULL, setup_load_slots, test_slot_mechanisms, teardown_load_slots);
	g_test_add("/gp11-session/session_props", int, NULL, setup_load_session, test_session_props, teardown_load_session);
	g_test_add("/gp11-session/session_info", int, NULL, setup_load_session, test_session_info, teardown_load_session);
	g_test_add("/gp11-session/open_close_session", int, NULL, setup_load_session, test_open_close_session, teardown_load_session);
	g_test_add("/gp11-session/open_reused", int, NULL, setup_load_session, test_open_reused, teardown_load_session);
	g_test_add("/gp11-session/login_logout", int, NULL, setup_load_session, test_login_logout, teardown_load_session);
	g_test_add("/gp11-session/auto_login", int, NULL, setup_load_session, test_auto_login, teardown_load_session);
	g_test_add("/gp11-object/object_props", int, NULL, setup_prep_object, test_object_props, teardown_prep_object);
	g_test_add("/gp11-object/object_equals_hash", int, NULL, setup_prep_object, test_object_equals_hash, teardown_prep_object);
	g_test_add("/gp11-object/create_object", int, NULL, setup_prep_object, test_create_object, teardown_prep_object);
	g_test_add("/gp11-object/destroy_object", int, NULL, setup_prep_object, test_destroy_object, teardown_prep_object);
	g_test_add("/gp11-object/get_attributes", int, NULL, setup_prep_object, test_get_attributes, teardown_prep_object);
	g_test_add("/gp11-object/get_data_attribute", int, NULL, setup_prep_object, test_get_data_attribute, teardown_prep_object);
	g_test_add("/gp11-object/set_attributes", int, NULL, setup_prep_object, test_set_attributes, teardown_prep_object);
	g_test_add("/gp11-object/find_objects", int, NULL, setup_prep_object, test_find_objects, teardown_prep_object);
	g_test_add("/gp11-object/explicit_sessions", int, NULL, setup_prep_object, test_explicit_sessions, teardown_prep_object);
	g_test_add("/gp11-crypto/encrypt", int, NULL, setup_crypto_session, test_encrypt, teardown_crypto_session);
	g_test_add("/gp11-crypto/decrypt", int, NULL, setup_crypto_session, test_decrypt, teardown_crypto_session);
	g_test_add("/gp11-crypto/login_context_specific", int, NULL, setup_crypto_session, test_login_context_specific, teardown_crypto_session);
	g_test_add("/gp11-crypto/sign", int, NULL, setup_crypto_session, test_sign, teardown_crypto_session);
	g_test_add("/gp11-crypto/verify", int, NULL, setup_crypto_session, test_verify, teardown_crypto_session);
}

#include "tests/gtest-helpers.c"
