/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/asn1/asn1_types", int, NULL, setup_asn1_tree, test_asn1_types, teardown_asn1_tree);
	g_test_add("/asn1/asn1_integers", int, NULL, setup_asn1_tree, test_asn1_integers, teardown_asn1_tree);
	g_test_add("/asn1/boolean", int, NULL, setup_asn1_tree, test_boolean, teardown_asn1_tree);
	g_test_add("/asn1/write_value", int, NULL, setup_asn1_tree, test_write_value, teardown_asn1_tree);
	g_test_add("/asn1/element_length_content", int, NULL, setup_asn1_tree, test_element_length_content, teardown_asn1_tree);
	g_test_add("/asn1/read_element", int, NULL, setup_asn1_tree, test_read_element, teardown_asn1_tree);
	g_test_add("/asn1/oid", int, NULL, setup_asn1_tree, test_oid, teardown_asn1_tree);
	g_test_add("/asn1/general_time", int, NULL, setup_asn1_tree, test_general_time, teardown_asn1_tree);
	g_test_add("/asn1/utc_time", int, NULL, setup_asn1_tree, test_utc_time, teardown_asn1_tree);
	g_test_add("/asn1/read_time", int, NULL, setup_asn1_tree, test_read_time, teardown_asn1_tree);
	g_test_add("/asn1/read_date", int, NULL, setup_asn1_tree, test_read_date, teardown_asn1_tree);
	g_test_add("/asn1/read_dn", int, NULL, setup_asn1_tree, test_read_dn, teardown_asn1_tree);
	g_test_add("/asn1/dn_value", int, NULL, setup_asn1_tree, test_dn_value, teardown_asn1_tree);
	g_test_add("/asn1/parse_dn", int, NULL, setup_asn1_tree, test_parse_dn, teardown_asn1_tree);
	g_test_add("/asn1/read_dn_part", int, NULL, setup_asn1_tree, test_read_dn_part, teardown_asn1_tree);
	g_test_add("/cleanup/cleanup", int, NULL, NULL, test_cleanup, NULL);
	g_test_add("/cleanup/order", int, NULL, NULL, test_order, NULL);
	g_test_add("/cleanup/reregister", int, NULL, NULL, test_reregister, NULL);
	g_test_add("/cleanup/remove", int, NULL, NULL, test_remove, NULL);
	g_test_add("/hex/hex_encode", int, NULL, NULL, test_hex_encode, NULL);
	g_test_add("/hex/hex_encode_spaces", int, NULL, NULL, test_hex_encode_spaces, NULL);
	g_test_add("/hex/hex_decode", int, NULL, NULL, test_hex_decode, NULL);
	g_test_add("/oid/oid_tests", int, NULL, NULL, test_oid_tests, NULL);
	g_test_add("/secmem/secmem_alloc_free", int, NULL, NULL, test_secmem_alloc_free, NULL);
	g_test_add("/secmem/secmem_realloc_across", int, NULL, NULL, test_secmem_realloc_across, NULL);
	g_test_add("/secmem/secmem_alloc_two", int, NULL, NULL, test_secmem_alloc_two, NULL);
	g_test_add("/secmem/secmem_realloc", int, NULL, NULL, test_secmem_realloc, NULL);
	g_test_add("/secmem/secmem_multialloc", int, NULL, NULL, test_secmem_multialloc, NULL);
	g_test_add("/symkey/generate_key_simple", int, NULL, setup_crypto_setup, test_generate_key_simple, teardown_crypto_setup);
	g_test_add("/symkey/generate_key_pkcs12", int, NULL, setup_crypto_setup, test_generate_key_pkcs12, teardown_crypto_setup);
	g_test_add("/symkey/generate_key_pbkdf2", int, NULL, setup_crypto_setup, test_generate_key_pbkdf2, teardown_crypto_setup);
	g_test_add("/symkey/generate_key_pbe", int, NULL, setup_crypto_setup, test_generate_key_pbe, teardown_crypto_setup);
	g_test_add("/openssl/parse_reference", int, NULL, NULL, test_parse_reference, NULL);
	g_test_add("/openssl/write_reference", int, NULL, NULL, test_write_reference, NULL);
	g_test_add("/openssl/openssl_roundtrip", int, NULL, NULL, test_openssl_roundtrip, NULL);
}

#include "tests/gtest-helpers.c"
