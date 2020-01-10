/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gkr-pkix-asn1.c - ASN.1 helper routines

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
   <http://www.gnu.org/licenses/>.

   Author: Stef Walter <stef@memberwebs.com>
*/

#include "config.h"

#include "gkm-data-asn1.h"

#include "egg/egg-asn1x.h"

gboolean
gkm_data_asn1_read_mpi (GNode *asn, gcry_mpi_t *mpi)
{
	gcry_error_t gcry;
	GBytes *buf;
	gsize sz;

	g_return_val_if_fail (asn, FALSE);
	g_return_val_if_fail (mpi, FALSE);

	buf = egg_asn1x_get_integer_as_raw (asn);
	if (!buf)
		return FALSE;

	/* Automatically stores in secure memory if DER data is secure */
	sz = g_bytes_get_size (buf);
	gcry = gcry_mpi_scan (mpi, GCRYMPI_FMT_STD, g_bytes_get_data (buf, NULL), sz, &sz);
	if (gcry != 0)
		return FALSE;

	return TRUE;
}

gboolean
gkm_data_asn1_write_mpi (GNode *asn, gcry_mpi_t mpi)
{
	gcry_error_t gcry;
	GBytes *bytes;
	gsize len;
	guchar *buf;

	g_return_val_if_fail (asn, FALSE);
	g_return_val_if_fail (mpi, FALSE);

	/* Get the size */
	gcry = gcry_mpi_print (GCRYMPI_FMT_STD, NULL, 0, &len, mpi);
	g_return_val_if_fail (gcry == 0, FALSE);
	g_return_val_if_fail (len > 0, FALSE);

	buf = gcry_calloc_secure (len, 1);

	gcry = gcry_mpi_print (GCRYMPI_FMT_STD, buf, len, &len, mpi);
	g_return_val_if_fail (gcry == 0, FALSE);

	bytes = g_bytes_new_with_free_func (buf, len, gcry_free, buf);
	egg_asn1x_set_integer_as_raw (asn, bytes);
	g_bytes_unref (bytes);

	return TRUE;
}
