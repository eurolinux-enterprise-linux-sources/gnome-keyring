/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/attributes/attribute_equal_zero_len_null_ptr", int, NULL, NULL, test_attribute_equal_zero_len_null_ptr, NULL);
	g_test_add("/attributes/attribute_consume", int, NULL, NULL, test_attribute_consume, NULL);
	g_test_add("/attributes/attribute_get_bool", int, NULL, NULL, test_attribute_get_bool, NULL);
	g_test_add("/attributes/attribute_get_bool_invalid", int, NULL, NULL, test_attribute_get_bool_invalid, NULL);
	g_test_add("/attributes/attribute_set_time", int, NULL, NULL, test_attribute_set_time, NULL);
	g_test_add("/attributes/attribute_set_time_empty", int, NULL, NULL, test_attribute_set_time_empty, NULL);
	g_test_add("/attributes/attribute_set_time_length", int, NULL, NULL, test_attribute_set_time_length, NULL);
	g_test_add("/attributes/attribute_get_time", int, NULL, NULL, test_attribute_get_time, NULL);
	g_test_add("/attributes/attribute_get_time_empty", int, NULL, NULL, test_attribute_get_time_empty, NULL);
	g_test_add("/attributes/attribute_get_time_invalid", int, NULL, NULL, test_attribute_get_time_invalid, NULL);
	g_test_add("/attributes/attribute_get_time_invalid_length", int, NULL, NULL, test_attribute_get_time_invalid_length, NULL);
	g_test_add("/crypto/parse_key", int, NULL, setup_crypto_setup, test_parse_key, teardown_crypto_setup);
	g_test_add("/crypto/sexp_key_to_public", int, NULL, setup_crypto_setup, test_sexp_key_to_public, teardown_crypto_setup);
	g_test_add("/data-asn1/asn1_integers", int, NULL, setup_asn1_tree, test_asn1_integers, teardown_asn1_tree);
	g_test_add("/data-der/der_rsa_public", int, NULL, setup_preload, test_der_rsa_public, teardown_preload);
	g_test_add("/data-der/der_dsa_public", int, NULL, setup_preload, test_der_dsa_public, teardown_preload);
	g_test_add("/data-der/der_rsa_private", int, NULL, setup_preload, test_der_rsa_private, teardown_preload);
	g_test_add("/data-der/der_dsa_private", int, NULL, setup_preload, test_der_dsa_private, teardown_preload);
	g_test_add("/data-der/der_dsa_private_parts", int, NULL, setup_preload, test_der_dsa_private_parts, teardown_preload);
	g_test_add("/data-der/read_public_key_info", int, NULL, setup_preload, test_read_public_key_info, teardown_preload);
	g_test_add("/data-der/read_certificate", int, NULL, setup_preload, test_read_certificate, teardown_preload);
	g_test_add("/data-der/write_certificate", int, NULL, setup_preload, test_write_certificate, teardown_preload);
	g_test_add("/data-der/read_ca_certificates_public_key_info", int, NULL, setup_preload, test_read_ca_certificates_public_key_info, teardown_preload);
	g_test_add("/data-der/read_basic_constraints", int, NULL, setup_preload, test_read_basic_constraints, teardown_preload);
	g_test_add("/data-der/read_key_usage", int, NULL, setup_preload, test_read_key_usage, teardown_preload);
	g_test_add("/data-der/read_enhanced_usage", int, NULL, setup_preload, test_read_enhanced_usage, teardown_preload);
	g_test_add("/data-der/read_all_pkcs8", int, NULL, setup_preload, test_read_all_pkcs8, teardown_preload);
	g_test_add("/data-der/read_pkcs8_bad_password", int, NULL, setup_preload, test_read_pkcs8_bad_password, teardown_preload);
	g_test_add("/data-der/write_pkcs8_plain", int, NULL, setup_preload, test_write_pkcs8_plain, teardown_preload);
	g_test_add("/data-der/write_pkcs8_encrypted", int, NULL, setup_preload, test_write_pkcs8_encrypted, teardown_preload);
	g_test_add("/object/object_create_destroy_transient", int, NULL, setup_object_setup, test_object_create_destroy_transient, teardown_object_teardown);
	g_test_add("/object/object_transient_transacted_fail", int, NULL, setup_object_setup, test_object_transient_transacted_fail, teardown_object_teardown);
	g_test_add("/object/object_create_transient_bad_value", int, NULL, setup_object_setup, test_object_create_transient_bad_value, teardown_object_teardown);
	g_test_add("/object/object_create_auto_destruct", int, NULL, setup_object_setup, test_object_create_auto_destruct, teardown_object_teardown);
	g_test_add("/object/object_create_auto_destruct_not_transient", int, NULL, setup_object_setup, test_object_create_auto_destruct_not_transient, teardown_object_teardown);
	g_test_add("/object/object_create_auto_destruct_bad_value", int, NULL, setup_object_setup, test_object_create_auto_destruct_bad_value, teardown_object_teardown);
	g_test_add("/authenticator/authenticator_create", int, NULL, setup_authenticator_setup, test_authenticator_create, teardown_authenticator_teardown);
	g_test_add("/authenticator/authenticator_create_missing_pin", int, NULL, setup_authenticator_setup, test_authenticator_create_missing_pin, teardown_authenticator_teardown);
	g_test_add("/authenticator/authenticator_create_no_object", int, NULL, setup_authenticator_setup, test_authenticator_create_no_object, teardown_authenticator_teardown);
	g_test_add("/authenticator/authenticator_create_invalid_object", int, NULL, setup_authenticator_setup, test_authenticator_create_invalid_object, teardown_authenticator_teardown);
	g_test_add("/authenticator/authenticator_get_attributes", int, NULL, setup_authenticator_setup, test_authenticator_get_attributes, teardown_authenticator_teardown);
	g_test_add("/authenticator/authenticator_uses_property", int, NULL, setup_authenticator_setup, test_authenticator_uses_property, teardown_authenticator_teardown);
	g_test_add("/authenticator/authenticator_object_property", int, NULL, setup_authenticator_setup, test_authenticator_object_property, teardown_authenticator_teardown);
	g_test_add("/authenticator/authenticator_login_property", int, NULL, setup_authenticator_setup, test_authenticator_login_property, teardown_authenticator_teardown);
	g_test_add("/timer/timer_extra_initialize", int, NULL, setup_timer_setup, test_timer_extra_initialize, teardown_timer_teardown);
	g_test_add("/timer/timer_simple", int, NULL, setup_timer_setup, test_timer_simple, teardown_timer_teardown);
	g_test_add("/timer/timer_cancel", int, NULL, setup_timer_setup, test_timer_cancel, teardown_timer_teardown);
	g_test_add("/timer/timer_immediate", int, NULL, setup_timer_setup, test_timer_immediate, teardown_timer_teardown);
	g_test_add("/timer/timer_multiple", int, NULL, setup_timer_setup, test_timer_multiple, teardown_timer_teardown);
	g_test_add("/timer/timer_outstanding", int, NULL, setup_timer_setup, test_timer_outstanding, teardown_timer_teardown);
	g_test_add("/transaction/transaction_empty", int, NULL, NULL, test_transaction_empty, NULL);
	g_test_add("/transaction/transaction_fail", int, NULL, NULL, test_transaction_fail, NULL);
	g_test_add("/transaction/transaction_signals_success", int, NULL, NULL, test_transaction_signals_success, NULL);
	g_test_add("/transaction/transaction_signals_failure", int, NULL, NULL, test_transaction_signals_failure, NULL);
	g_test_add("/transaction/transaction_order_is_reverse", int, NULL, NULL, test_transaction_order_is_reverse, NULL);
	g_test_add("/transaction/transaction_dispose_completes", int, NULL, NULL, test_transaction_dispose_completes, NULL);
	g_test_add("/transaction/remove_file_success", int, NULL, NULL, test_remove_file_success, NULL);
	g_test_add("/transaction/remove_file_abort", int, NULL, NULL, test_remove_file_abort, NULL);
	g_test_add("/transaction/remove_file_non_exist", int, NULL, NULL, test_remove_file_non_exist, NULL);
	g_test_add("/transaction/write_file", int, NULL, NULL, test_write_file, NULL);
	g_test_add("/transaction/write_file_abort_gone", int, NULL, NULL, test_write_file_abort_gone, NULL);
	g_test_add("/transaction/write_file_abort_revert", int, NULL, NULL, test_write_file_abort_revert, NULL);
	g_test_add("/store/store_schema", int, NULL, setup_store, test_store_schema, teardown_store);
	g_test_add("/store/store_schema_flags", int, NULL, setup_store, test_store_schema_flags, teardown_store);
	g_test_add("/memory-store/get_attribute_default", int, NULL, setup_memory_store, test_get_attribute_default, teardown_memory_store);
	g_test_add("/memory-store/read_value_default", int, NULL, setup_memory_store, test_read_value_default, teardown_memory_store);
	g_test_add("/memory-store/read_string", int, NULL, setup_memory_store, test_read_string, teardown_memory_store);
	g_test_add("/memory-store/get_invalid", int, NULL, setup_memory_store, test_get_invalid, teardown_memory_store);
	g_test_add("/memory-store/get_sensitive", int, NULL, setup_memory_store, test_get_sensitive, teardown_memory_store);
	g_test_add("/memory-store/get_internal", int, NULL, setup_memory_store, test_get_internal, teardown_memory_store);
	g_test_add("/memory-store/set_invalid", int, NULL, setup_memory_store, test_set_invalid, teardown_memory_store);
	g_test_add("/memory-store/set_internal", int, NULL, setup_memory_store, test_set_internal, teardown_memory_store);
	g_test_add("/memory-store/set_get_attribute", int, NULL, setup_memory_store, test_set_get_attribute, teardown_memory_store);
	g_test_add("/memory-store/write_read_value", int, NULL, setup_memory_store, test_write_read_value, teardown_memory_store);
	g_test_add("/memory-store/set_no_validate", int, NULL, setup_memory_store, test_set_no_validate, teardown_memory_store);
	g_test_add("/memory-store/set_transaction_default", int, NULL, setup_memory_store, test_set_transaction_default, teardown_memory_store);
	g_test_add("/memory-store/set_transaction_revert_first", int, NULL, setup_memory_store, test_set_transaction_revert_first, teardown_memory_store);
	g_test_add("/memory-store/set_notifies", int, NULL, setup_memory_store, test_set_notifies, teardown_memory_store);
	g_test_add("/memory-store/set_object_gone_first", int, NULL, setup_memory_store, test_set_object_gone_first, teardown_memory_store);
	g_test_add("/login/test_login", int, NULL, NULL, test_test_login, NULL);
	g_test_add("/login/test_null_terminated", int, NULL, NULL, test_test_null_terminated, NULL);
	g_test_add("/login/test_null", int, NULL, NULL, test_test_null, NULL);
	g_test_add("/login/test_empty", int, NULL, NULL, test_test_empty, NULL);
	g_test_add("/data-file/test_file_create", int, NULL, setup_file_store, test_test_file_create, teardown_file_store);
	g_test_add("/data-file/test_file_write_value", int, NULL, setup_file_store, test_test_file_write_value, teardown_file_store);
	g_test_add("/data-file/test_file_read_value", int, NULL, setup_file_store, test_test_file_read_value, teardown_file_store);
	g_test_add("/data-file/test_file_read", int, NULL, setup_file_store, test_test_file_read, teardown_file_store);
	g_test_add("/data-file/test_file_lookup", int, NULL, setup_file_store, test_test_file_lookup, teardown_file_store);
	g_test_add("/data-file/file_read_private_without_login", int, NULL, setup_file_store, test_file_read_private_without_login, teardown_file_store);
	g_test_add("/data-file/test_file_write", int, NULL, setup_file_store, test_test_file_write, teardown_file_store);
	g_test_add("/data-file/cant_write_private_without_login", int, NULL, setup_file_store, test_cant_write_private_without_login, teardown_file_store);
	g_test_add("/data-file/write_private_with_login", int, NULL, setup_file_store, test_write_private_with_login, teardown_file_store);
	g_test_add("/data-file/read_private_with_login", int, NULL, setup_file_store, test_read_private_with_login, teardown_file_store);
	g_test_add("/data-file/destroy_entry", int, NULL, setup_file_store, test_destroy_entry, teardown_file_store);
	g_test_add("/data-file/destroy_entry_by_loading", int, NULL, setup_file_store, test_destroy_entry_by_loading, teardown_file_store);
	g_test_add("/data-file/destroy_private_without_login", int, NULL, setup_file_store, test_destroy_private_without_login, teardown_file_store);
	g_test_add("/data-file/entry_added_signal", int, NULL, setup_file_store, test_entry_added_signal, teardown_file_store);
	g_test_add("/data-file/entry_changed_signal", int, NULL, setup_file_store, test_entry_changed_signal, teardown_file_store);
	g_test_add("/data-file/entry_removed_signal", int, NULL, setup_file_store, test_entry_removed_signal, teardown_file_store);
	g_test_add("/data-file/data_file_foreach", int, NULL, setup_file_store, test_data_file_foreach, teardown_file_store);
	g_test_add("/data-file/unique_entry", int, NULL, setup_file_store, test_unique_entry, teardown_file_store);
	g_test_add("/data-file/have_sections", int, NULL, setup_file_store, test_have_sections, teardown_file_store);
	g_test_add("/file-tracker/file_watch", int, NULL, setup_tracker, test_file_watch, teardown_tracker);
	g_test_add("/file-tracker/watch_file", int, NULL, setup_tracker, test_watch_file, teardown_tracker);
	g_test_add("/file-tracker/nomatch", int, NULL, setup_tracker, test_nomatch, teardown_tracker);
}

#include "tests/gtest-helpers.c"
