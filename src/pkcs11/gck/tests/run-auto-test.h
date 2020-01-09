/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"

DECLARE_TEST(attribute_equal_zero_len_null_ptr);
DECLARE_TEST(attribute_consume);
DECLARE_TEST(attribute_get_bool);
DECLARE_TEST(attribute_get_bool_invalid);
DECLARE_TEST(attribute_set_time);
DECLARE_TEST(attribute_set_time_empty);
DECLARE_TEST(attribute_set_time_length);
DECLARE_TEST(attribute_get_time);
DECLARE_TEST(attribute_get_time_empty);
DECLARE_TEST(attribute_get_time_invalid);
DECLARE_TEST(attribute_get_time_invalid_length);
DECLARE_SETUP(crypto_setup);
DECLARE_TEARDOWN(crypto_setup);
DECLARE_TEST(parse_key);
DECLARE_TEST(sexp_key_to_public);
DECLARE_SETUP(asn1_tree);
DECLARE_TEARDOWN(asn1_tree);
DECLARE_TEST(asn1_integers);
DECLARE_SETUP(preload);
DECLARE_TEARDOWN(preload);
DECLARE_TEST(der_rsa_public);
DECLARE_TEST(der_dsa_public);
DECLARE_TEST(der_rsa_private);
DECLARE_TEST(der_dsa_private);
DECLARE_TEST(der_dsa_private_parts);
DECLARE_TEST(read_public_key_info);
DECLARE_TEST(read_certificate);
DECLARE_TEST(write_certificate);
DECLARE_TEST(read_ca_certificates_public_key_info);
DECLARE_TEST(read_basic_constraints);
DECLARE_TEST(read_key_usage);
DECLARE_TEST(read_enhanced_usage);
DECLARE_TEST(read_all_pkcs8);
DECLARE_TEST(read_pkcs8_bad_password);
DECLARE_TEST(write_pkcs8_plain);
DECLARE_TEST(write_pkcs8_encrypted);
DECLARE_SETUP(object_setup);
DECLARE_TEARDOWN(object_teardown);
DECLARE_TEST(object_create_destroy_transient);
DECLARE_TEST(object_transient_transacted_fail);
DECLARE_TEST(object_create_transient_bad_value);
DECLARE_TEST(object_create_auto_destruct);
DECLARE_TEST(object_create_auto_destruct_not_transient);
DECLARE_TEST(object_create_auto_destruct_bad_value);
DECLARE_SETUP(authenticator_setup);
DECLARE_TEARDOWN(authenticator_teardown);
DECLARE_TEST(authenticator_create);
DECLARE_TEST(authenticator_create_missing_pin);
DECLARE_TEST(authenticator_create_no_object);
DECLARE_TEST(authenticator_create_invalid_object);
DECLARE_TEST(authenticator_get_attributes);
DECLARE_TEST(authenticator_uses_property);
DECLARE_TEST(authenticator_object_property);
DECLARE_TEST(authenticator_login_property);
DECLARE_SETUP(timer_setup);
DECLARE_TEARDOWN(timer_teardown);
DECLARE_TEST(timer_extra_initialize);
DECLARE_TEST(timer_simple);
DECLARE_TEST(timer_cancel);
DECLARE_TEST(timer_immediate);
DECLARE_TEST(timer_multiple);
DECLARE_TEST(timer_outstanding);
DECLARE_TEST(transaction_empty);
DECLARE_TEST(transaction_fail);
DECLARE_TEST(transaction_signals_success);
DECLARE_TEST(transaction_signals_failure);
DECLARE_TEST(transaction_order_is_reverse);
DECLARE_TEST(transaction_dispose_completes);
DECLARE_TEST(remove_file_success);
DECLARE_TEST(remove_file_abort);
DECLARE_TEST(remove_file_non_exist);
DECLARE_TEST(write_file);
DECLARE_TEST(write_file_abort_gone);
DECLARE_TEST(write_file_abort_revert);
DECLARE_SETUP(store);
DECLARE_TEARDOWN(store);
DECLARE_TEST(store_schema);
DECLARE_TEST(store_schema_flags);
DECLARE_SETUP(memory_store);
DECLARE_TEARDOWN(memory_store);
DECLARE_TEST(get_attribute_default);
DECLARE_TEST(read_value_default);
DECLARE_TEST(read_string);
DECLARE_TEST(get_invalid);
DECLARE_TEST(get_sensitive);
DECLARE_TEST(get_internal);
DECLARE_TEST(set_invalid);
DECLARE_TEST(set_internal);
DECLARE_TEST(set_get_attribute);
DECLARE_TEST(write_read_value);
DECLARE_TEST(set_no_validate);
DECLARE_TEST(set_transaction_default);
DECLARE_TEST(set_transaction_revert_first);
DECLARE_TEST(set_notifies);
DECLARE_TEST(set_object_gone_first);
DECLARE_TEST(test_login);
DECLARE_TEST(test_null_terminated);
DECLARE_TEST(test_null);
DECLARE_TEST(test_empty);
DECLARE_SETUP(file_store);
DECLARE_TEARDOWN(file_store);
DECLARE_TEST(test_file_create);
DECLARE_TEST(test_file_write_value);
DECLARE_TEST(test_file_read_value);
DECLARE_TEST(test_file_read);
DECLARE_TEST(test_file_lookup);
DECLARE_TEST(file_read_private_without_login);
DECLARE_TEST(test_file_write);
DECLARE_TEST(cant_write_private_without_login);
DECLARE_TEST(write_private_with_login);
DECLARE_TEST(read_private_with_login);
DECLARE_TEST(destroy_entry);
DECLARE_TEST(destroy_entry_by_loading);
DECLARE_TEST(destroy_private_without_login);
DECLARE_TEST(entry_added_signal);
DECLARE_TEST(entry_changed_signal);
DECLARE_TEST(entry_removed_signal);
DECLARE_TEST(data_file_foreach);
DECLARE_TEST(unique_entry);
DECLARE_TEST(have_sections);
DECLARE_SETUP(tracker);
DECLARE_TEARDOWN(tracker);
DECLARE_TEST(file_watch);
DECLARE_TEST(watch_file);
DECLARE_TEST(nomatch);

