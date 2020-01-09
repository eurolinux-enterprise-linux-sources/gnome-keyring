/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/certificate/issuer_cn", int, NULL, setup_certificate, test_issuer_cn, teardown_certificate);
	g_test_add("/certificate/issuer_dn", int, NULL, setup_certificate, test_issuer_dn, teardown_certificate);
	g_test_add("/certificate/issuer_part", int, NULL, setup_certificate, test_issuer_part, teardown_certificate);
	g_test_add("/certificate/subject_cn", int, NULL, setup_certificate, test_subject_cn, teardown_certificate);
	g_test_add("/certificate/subject_dn", int, NULL, setup_certificate, test_subject_dn, teardown_certificate);
	g_test_add("/certificate/subject_part", int, NULL, setup_certificate, test_subject_part, teardown_certificate);
	g_test_add("/certificate/issued_date", int, NULL, setup_certificate, test_issued_date, teardown_certificate);
	g_test_add("/certificate/expiry_date", int, NULL, setup_certificate, test_expiry_date, teardown_certificate);
	g_test_add("/certificate/serial_number", int, NULL, setup_certificate, test_serial_number, teardown_certificate);
	g_test_add("/certificate/fingerprint", int, NULL, setup_certificate, test_fingerprint, teardown_certificate);
	g_test_add("/certificate/fingerprint_hex", int, NULL, setup_certificate, test_fingerprint_hex, teardown_certificate);
	g_test_add("/parser/parse_all", int, NULL, setup_parser, test_parse_all, teardown_parser);
}

#include "tests/gtest-helpers.c"
