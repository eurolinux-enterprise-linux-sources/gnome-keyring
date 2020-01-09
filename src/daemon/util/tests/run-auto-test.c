/* This is auto-generated code. Edit at your own peril. */
#include "tests/gtest-helpers.h"
#include "run-auto-test.h"

static void start_tests (void) {
}

static void stop_tests (void) {
}

static void initialize_tests (void) {
	g_test_add("/async/worker_simple", int, NULL, setup_async_init, test_worker_simple, NULL);
	g_test_add("/async/worker_cancel", int, NULL, setup_async_init, test_worker_cancel, NULL);
	g_test_add("/async/worker_five", int, NULL, setup_async_init, test_worker_five, NULL);
	g_test_add("/location/location_simple", int, NULL, NULL, test_location_simple, NULL);
	g_test_add("/location/location_parent", int, NULL, NULL, test_location_parent, NULL);
	g_test_add("/location/location_media", int, NULL, NULL, test_location_media, NULL);
	g_test_add("/location/location_fileops", int, NULL, NULL, test_location_fileops, NULL);
	g_test_add("/location-watch/location_watch", int, NULL, NULL, test_location_watch, NULL);
	g_test_add("/location-watch/location_file", int, NULL, NULL, test_location_file, NULL);
	g_test_add("/location-watch/location_nomatch", int, NULL, NULL, test_location_nomatch, NULL);
}

#include "tests/gtest-helpers.c"
