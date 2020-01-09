/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* unit-test-daemon-setup.c: Start a gnome-keyring-daemon process for testing

   Copyright (C) 2007 Stefan Walter

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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>

#include "run-auto-test.h"

#include "library/gnome-keyring.h"

static GPid daemon_pid;

DEFINE_START(setup_daemon)
{
	GError *err = NULL;
	gchar *args[3];
	const gchar *outside, *path;
	gboolean start = FALSE;
	gchar *socket;

	/* If already setup somewhere else, then don't start daemon here */
	outside = g_getenv ("GNOME_KEYRING_OUTSIDE_TEST");
	if (!outside || !outside[0]) {
		start = TRUE;
	}

	path = g_getenv ("GNOME_KEYRING_TEST_PATH");
	g_assert (path && path[0]);

	socket = g_strdup_printf ("%s/socket", path);
	g_setenv ("GNOME_KEYRING_SOCKET", socket, TRUE);
	
	if (!start)
		return;
		
	g_printerr ("Starting gnome-keyring-daemon...\n");

	args[0] = "../../daemon/gnome-keyring-daemon";
	args[1] = "-f";
	args[2] = NULL;
	
	if (!g_spawn_async (NULL, args, NULL, G_SPAWN_LEAVE_DESCRIPTORS_OPEN | G_SPAWN_DO_NOT_REAP_CHILD, 
	                    NULL, NULL, &daemon_pid, &err)) {
		g_error ("couldn't start gnome-keyring-daemon for testing: %s", 
		         err && err->message ? err->message : "");
		g_assert_not_reached ();
	}
	
	/* Let it startup properly */
	sleep (2);
}

DEFINE_STOP(setup_daemon)
{
	if (daemon_pid)
		kill (daemon_pid, SIGTERM);
	/* We're exiting soon anyway, no need to wait */
}
