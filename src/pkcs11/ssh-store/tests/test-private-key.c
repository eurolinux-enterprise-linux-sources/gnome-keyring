/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* unit-test-private-key.c: Test SSH Key Private key functionality

   Copyright (C) 2009 Stefan Walter

   The Gnome Keyring Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Gnome Keyring Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the Gnome Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

   Author: Stef Walter <stef@memberwebs.com>
*/

#include "config.h"

#include "mock-ssh-module.h"

#include "gkm/gkm-credential.h"
#include "gkm/gkm-session.h"
#include "gkm/gkm-module.h"

#include "ssh-store/gkm-ssh-private-key.h"

#include "egg/egg-testing.h"

#include "pkcs11i.h"

typedef struct {
	GkmModule *module;
	GkmSession *session;
	GkmSshPrivateKey *key;
} Test;

static void
setup_basic (Test *test,
             gconstpointer unused)
{
	test->module = test_ssh_module_initialize_and_enter ();
	test->session = test_ssh_module_open_session (TRUE);
}

static void
teardown_basic (Test *test,
                gconstpointer unused)
{
	test_ssh_module_leave_and_finalize ();
}

static void
setup (Test *test,
       gconstpointer unused)
{
	gboolean ret;

	setup_basic (test, unused);

	test->key = gkm_ssh_private_key_new (test->module, "my-unique");
	g_assert (GKM_IS_SSH_PRIVATE_KEY (test->key));

	ret = gkm_ssh_private_key_parse (test->key, SRCDIR "/files/id_dsa_encrypted.pub",
	                                 SRCDIR "/files/id_dsa_encrypted", NULL);
	g_assert (ret == TRUE);
}

static void
teardown (Test *test,
          gconstpointer unused)
{
	g_object_unref (test->key);
	teardown_basic (test, unused);
}

static void
test_parse_plain (Test *test, gconstpointer unused)
{
	GkmSshPrivateKey *key;
	gboolean ret;

	key = gkm_ssh_private_key_new (test->module, "my-unique");
	g_assert (GKM_IS_SSH_PRIVATE_KEY (key));

	ret = gkm_ssh_private_key_parse (key, SRCDIR "/files/id_dsa_plain.pub",
	                                 SRCDIR "/files/id_dsa_plain", NULL);
	g_assert (ret == TRUE);

	g_object_unref (key);
}

static void
test_unlock (Test *test,
             gconstpointer unused)
{
	GkmCredential *cred;
	CK_RV rv;

	rv = gkm_credential_create (test->module, NULL, GKM_OBJECT (test->key),
	                            (guchar*)"password", 8, &cred);
	g_assert (rv == CKR_OK);

	g_object_unref (cred);
}

static void
test_internal_sha1_compat (Test *test,
                           gconstpointer unused)
{
	gpointer data;
	gsize n_data;

	data = gkm_object_get_attribute_data (GKM_OBJECT (test->key), test->session,
	                                      CKA_GNOME_INTERNAL_SHA1, &n_data);

	egg_assert_cmpmem (data, n_data, ==, "\x33\x37\x31\x31\x64\x33\x33\x65\x61\x34\x31\x31\x33\x61\x35\x64\x32\x35\x38\x37\x63\x36\x66\x32\x35\x66\x39\x35\x35\x36\x39\x66\x65\x65\x38\x31\x38\x35\x39\x34", 40);
	g_free (data);
}

int
main (int argc, char **argv)
{
#if !GLIB_CHECK_VERSION(2,35,0)
	g_type_init ();
#endif
	g_test_init (&argc, &argv, NULL);

	g_test_add ("/ssh-store/private-key/parse_plain", Test, NULL, setup_basic, test_parse_plain, teardown_basic);
	g_test_add ("/ssh-store/private-key/unlock", Test, NULL, setup, test_unlock, teardown);
	g_test_add ("/ssh-store/private-key/internal-sha1-compat", Test, NULL, setup, test_internal_sha1_compat, teardown);

	return g_test_run ();
}
