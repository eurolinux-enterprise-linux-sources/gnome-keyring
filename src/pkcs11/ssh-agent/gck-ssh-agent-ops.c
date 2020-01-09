/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 8; tab-width: 8 -*- */
/* gkr-ssh-daemon-ops.h - SSH agent operations

   Copyright (C) 2007 Stefan Walter

   Gnome keyring is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.
  
   Gnome keyring is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   Author: Stef Walter <stef@memberwebs.com>
*/

#include "config.h"

#include "gck-ssh-agent-private.h"

#include "gp11/gp11.h"

#include "pkcs11/pkcs11.h"
#include "pkcs11/pkcs11g.h"

#include "egg/egg-secure-memory.h"

#include <glib.h>

#include <ctype.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


#define V1_LABEL "SSH1 RSA Key"

/* ---------------------------------------------------------------------------- */


static void
copy_attribute (GP11Attributes *original, CK_ATTRIBUTE_TYPE type, GP11Attributes *dest)
{
	GP11Attribute *attr;
	
	g_assert (original);
	g_assert (dest);
	
	attr = gp11_attributes_find (original, type);
	if (attr)
		gp11_attributes_add (dest, attr);
}

static gboolean
login_session (GP11Session *session)
{
	GP11SessionInfo *info;
	GError *error = NULL;
	gboolean ret = TRUE;
	
	/* TODO: We should have a way to just get the state */
	info = gp11_session_get_info (session);
	g_return_val_if_fail (info, FALSE);
	
	/* Log in the session if necessary */
	if (info->state == CKS_RO_PUBLIC_SESSION || info->state == CKS_RW_PUBLIC_SESSION) {
		if (!gp11_session_login (session, CKU_USER, NULL, 0, &error)) {
			g_message ("couldn't log into session: %s", error->message);
			ret = FALSE;
		}
	}
		
	gp11_session_info_free (info);
		
	return ret;
}

static GP11Attributes*
build_like_attributes (GP11Attributes *attrs, CK_OBJECT_CLASS klass)
{
	GP11Attributes *search;
	gulong key_type;
	
	g_assert (attrs);
	
	/* Determine the key type */
	if (!gp11_attributes_find_ulong (attrs, CKA_KEY_TYPE, &key_type))
		g_return_val_if_reached (NULL);
	
	search = gp11_attributes_new ();
	gp11_attributes_add_ulong (search, CKA_CLASS, klass);
	copy_attribute (attrs, CKA_KEY_TYPE, search);
	copy_attribute (attrs, CKA_TOKEN, search);
	
	switch (key_type) {
	case CKK_RSA:
		copy_attribute (attrs, CKA_MODULUS, search);
		copy_attribute (attrs, CKA_PUBLIC_EXPONENT, search);
		break;
		
	case CKK_DSA:
		copy_attribute (attrs, CKA_PRIME, search);
		copy_attribute (attrs, CKA_SUBPRIME, search);
		copy_attribute (attrs, CKA_BASE, search);
		copy_attribute (attrs, CKA_VALUE, search);
		break;
		
	default:
		g_return_val_if_reached (NULL);
		break;
	}
	
	return search;
}

static void
search_keys_like_attributes (gpointer session_or_module, GP11Attributes *attrs, CK_OBJECT_CLASS klass, 
                             GP11ObjectForeachFunc func, gpointer user_data)
{
	GP11Attributes *search;
	GError *error = NULL;
	GList *keys, *l;
	
	search = build_like_attributes (attrs, klass);
	
	/* In all slots */
	if (GP11_IS_MODULE (session_or_module)) {
		if (!gp11_module_enumerate_objects_full (session_or_module, search, NULL, 
		                                         func, user_data, &error)) {
			g_warning ("couldn't enumerate matching keys: %s", error->message);
			g_clear_error (&error);
		}
		
	/* Otherwise search in the session */
	} else if (GP11_IS_SESSION (session_or_module)){
		keys = gp11_session_find_objects_full (session_or_module, search, NULL, &error);
		
		if (error) {
			g_warning ("couldn't find matching keys: %s", error->message);
			g_clear_error (&error);
			
		} else {
			for (l = keys; l; l = g_list_next (l)) {
				if (!(func) (l->data, user_data))
					break;
			}
			
			gp11_list_unref_free (keys);
		}
		
	/* Bad object passed in */
	} else {
		g_assert_not_reached ();
	}
	
	gp11_attributes_unref (search);
}

static gboolean
list_all_matching (GP11Object *object, gpointer user_data)
{
	GList** list = (GList**)user_data;
	g_return_val_if_fail (GP11_IS_OBJECT (object), FALSE);
	*list = g_list_prepend (*list, g_object_ref (object));
	
	/* Keep going */
	return TRUE;
}

static gboolean
return_first_matching (GP11Object *object, gpointer user_data)
{
	GP11Object **result = (GP11Object**)user_data;
	
	g_return_val_if_fail (GP11_IS_OBJECT (object), FALSE);
	g_return_val_if_fail (result != NULL, FALSE);
	g_return_val_if_fail (*result == NULL, FALSE);
	*result = g_object_ref (object);
	
	/* We've seen enough */
	return FALSE;
}

static gboolean
return_private_matching (GP11Object *object, gpointer user_data)
{
	GP11Object **result = (GP11Object**)user_data;
	GP11Session *session;
	GP11Attributes *attrs;
	GP11Attribute *attr;
	gboolean token;
	GList *objects;
	GError *error = NULL;
	
	g_return_val_if_fail (GP11_IS_OBJECT (object), FALSE);
	g_return_val_if_fail (result != NULL, FALSE);
	g_return_val_if_fail (*result == NULL, FALSE);
	
	/* Get the key identifier and token */
	attrs = gp11_object_get (object, &error, CKA_ID, CKA_TOKEN, GP11_INVALID);
	if (error) {
		g_warning ("error retrieving attributes for public key: %s", error->message);
		g_clear_error (&error);
		return TRUE;
	}

	/* Dig out the key identifier and token */
	attr = gp11_attributes_find (attrs, CKA_ID);
	g_return_val_if_fail (attr, FALSE);
	
	if (!gp11_attributes_find_boolean (attrs, CKA_TOKEN, &token))
		token = FALSE;
	
	session = gp11_object_get_session (object);
	g_return_val_if_fail (GP11_IS_SESSION (session), FALSE);

	if (!login_session (session))
		return FALSE;
	
	/* Search for the matching private key */
	objects = gp11_session_find_objects (session, NULL, 
	                                     CKA_ID, attr->length, attr->value,
	                                     CKA_CLASS, GP11_ULONG, CKO_PRIVATE_KEY,
	                                     CKA_TOKEN, GP11_BOOLEAN, token,
	                                     GP11_INVALID);
		
	gp11_attributes_unref (attrs);
		
	/* Keep searching, not found */
	if (objects) {
		*result = g_object_ref (objects->data);
		gp11_object_set_session (*result, session);
		gp11_list_unref_free (objects);
	}

	g_object_unref (session);

	/* Stop once we have a key */
	return (*result == NULL);
}

static gboolean 
load_identity_v1_attributes (GP11Object *object, gpointer user_data)
{
	GP11Attributes *attrs;
	GError *error = NULL;
	GList **all_attrs;
	
	g_return_val_if_fail (GP11_IS_OBJECT (object), FALSE);
	g_return_val_if_fail (user_data, FALSE);
	
	/* 
	 * The encompassing search should have limited to the right label.
	 * In addition V1 keys are only RSA.
	 */
	
	attrs = gp11_object_get (object, &error, CKA_ID, CKA_LABEL, CKA_KEY_TYPE, CKA_MODULUS, 
	                         CKA_PUBLIC_EXPONENT, CKA_CLASS, CKA_MODULUS_BITS, GP11_INVALID);
	if (error) {
		g_warning ("error retrieving attributes for public key: %s", error->message);
		g_clear_error (&error);
		return TRUE;
	}

	all_attrs = (GList**)user_data;
	*all_attrs = g_list_prepend (*all_attrs, attrs);
	
	/* Note that we haven't reffed the object or session */

	/* Keep going */
	return TRUE;
}

static gboolean 
load_identity_v2_attributes (GP11Object *object, gpointer user_data)
{
	GP11Attributes *attrs;
	GP11Attribute *attr;
	GError *error = NULL;
	gboolean valid = TRUE;
	gboolean token;
	GList **all_attrs;
	
	g_return_val_if_fail (GP11_IS_OBJECT (object), FALSE);
	g_return_val_if_fail (user_data, FALSE);
	
	attrs = gp11_object_get (object, &error, CKA_ID, CKA_LABEL, CKA_KEY_TYPE, CKA_MODULUS, 
	                         CKA_PUBLIC_EXPONENT, CKA_PRIME, CKA_SUBPRIME, CKA_BASE, 
	                         CKA_VALUE, CKA_CLASS, CKA_MODULUS_BITS, CKA_TOKEN, GP11_INVALID);
	if (error) {
		g_warning ("error retrieving attributes for public key: %s", error->message);
		g_clear_error (&error);
		return TRUE;
	}
	
	/* Dig out the label, and see if it's not v1, skip if so */
	attr = gp11_attributes_find (attrs, CKA_LABEL);
	if (attr != NULL) {
		if (attr->length == strlen (V1_LABEL) && 
		    strncmp ((gchar*)attr->value, V1_LABEL, attr->length) == 0)
			valid = FALSE;
	}
	
	/* Figure out if it's a token object or not */
	if (!gp11_attributes_find_boolean (attrs, CKA_TOKEN, &token))
		token = FALSE;

	all_attrs = (GList**)user_data;
	if (valid == TRUE)
		*all_attrs = g_list_prepend (*all_attrs, attrs);
	else 
		gp11_attributes_unref (attrs);
	
	/* Note that we haven't reffed the object or session */
	
	/* Keep going */
	return TRUE;
}

static void
remove_key_pair (GP11Session *session, GP11Object *priv, GP11Object *pub)
{
	GError *error = NULL;
	
	g_assert (GP11_IS_SESSION (session));
	
	if (!login_session (session))
		return;
	
	if (priv != NULL) {
		gp11_object_set_session (priv, session);
		gp11_object_destroy (priv, &error);
	
		if (error) {
			if (error->code != CKR_OBJECT_HANDLE_INVALID) 
				g_warning ("couldn't remove ssh private key: %s", error->message);
			g_clear_error (&error);
		}
	}
	
	if (pub != NULL) {
		gp11_object_set_session (pub, session);
		gp11_object_destroy (pub, &error);
	
		if (error) {
			if (error->code != CKR_OBJECT_HANDLE_INVALID) 
				g_warning ("couldn't remove ssh public key: %s", error->message);
			g_clear_error (&error);
		}
	}
}

static void
lock_key_pair (GP11Session *session, GP11Object *priv, GP11Object *pub)
{
	GError *error = NULL;
	GList *objects, *l;

	g_assert (GP11_IS_SESSION (session));
	g_assert (GP11_IS_OBJECT (priv));
	g_assert (GP11_IS_OBJECT (pub));

	if (!login_session (session))
		return;

	/* Delete any authenticator objects */
	objects = gp11_session_find_objects (session, &error,
	                                     CKA_CLASS, GP11_ULONG, CKO_GNOME_AUTHENTICATOR,
	                                     CKA_GNOME_OBJECT, GP11_ULONG, gp11_object_get_handle (priv),
	                                     GP11_INVALID);

	if (error) {
		g_warning ("couldn't search for authenticator objects: %s", error->message);
		g_clear_error (&error);
		return;
	}

	/* Delete them all */
	for (l = objects; l; l = g_list_next (l)) {
		gp11_object_destroy (l->data, &error);
		if (error) {
			g_warning ("couldn't delete authenticator object: %s", error->message);
			g_clear_error (&error);
		}
	}
}

static void
remove_by_public_key (GP11Session *session, GP11Object *pub, gboolean exclude_v1)
{
	GP11Attributes *attrs;
	GError *error = NULL;
	GList *objects;
	gboolean token;
	gchar *label;

	g_assert (GP11_IS_SESSION (session));
	g_assert (GP11_IS_OBJECT (pub));
	
	if (!login_session (session))
		return;

	gp11_object_set_session (pub, session);
	attrs = gp11_object_get (pub, &error, 
	                         CKA_LABEL, CKA_ID, CKA_TOKEN, 
	                         GP11_INVALID);
	if (error) {
		g_warning ("couldn't lookup attributes for key: %s", error->message);
		g_clear_error (&error);
		return;
	}
	
	/* Skip over SSH V1 keys */
	if (exclude_v1 && gp11_attributes_find_string (attrs, CKA_LABEL, &label)) {
		if (label && strcmp (label, V1_LABEL) == 0) {
			gp11_attributes_unref (attrs);
			g_free (label);
			return;
		}
	}

	/* Lock token objects, remove session objects */
	if (!gp11_attributes_find_boolean (attrs, CKA_TOKEN, &token))
		token = FALSE;
	
	/* Search for exactly the same attributes but with a private key class */
	gp11_attributes_add_ulong (attrs, CKA_CLASS, CKO_PRIVATE_KEY);
	objects = gp11_session_find_objects_full (session, attrs, NULL, &error);
	gp11_attributes_unref (attrs);
	
	if (error) {
		g_warning ("couldn't search for related key: %s", error->message);
		g_clear_error (&error);
		return;
	}
	
	/* Lock the token objects */
	if (token && objects) {
		lock_key_pair (session, objects->data, pub);
	} else if (!token) {
		remove_key_pair (session, objects->data, pub);
	}

	gp11_list_unref_free (objects);
}

static gboolean
create_key_pair (GP11Session *session, GP11Attributes *priv, GP11Attributes *pub)
{
	GP11Object *priv_key, *pub_key;
	GError *error = NULL;
	
	g_assert (GP11_IS_SESSION (session));
	g_assert (priv);
	g_assert (pub);
	
	if (!login_session (session))
		return FALSE;
	
	priv_key = gp11_session_create_object_full (session, priv, NULL, &error);
	if (error) {
		g_warning ("couldn't create session private key: %s", error->message);
		g_clear_error (&error);
		return FALSE;
	}
	
	pub_key = gp11_session_create_object_full (session, pub, NULL, &error);
	if (error) {
		g_warning ("couldn't create session public key: %s", error->message);
		g_clear_error (&error);
		
		/* Failed, so remove private as well */
		gp11_object_set_session (priv_key, session);
		gp11_object_destroy (priv_key, NULL);
		g_object_unref (priv_key);
		
		return FALSE;
	}
	
	g_object_unref (pub_key);
	g_object_unref (priv_key);
	
	return TRUE;
}

static void
destroy_replaced_keys (GP11Session *session, GList *keys)
{
	GError *error = NULL;
	GList *l;

	g_assert (GP11_IS_SESSION (session));
	
	for (l = keys; l; l = g_list_next (l)) {
		gp11_object_set_session (l->data, session);
		if (!gp11_object_destroy (l->data, &error)) {
			if (error->code != CKR_OBJECT_HANDLE_INVALID)
				g_warning ("couldn't delete a SSH key we replaced: %s", error->message);
			g_clear_error (&error);
		}
	}
}

static gboolean
replace_key_pair (GP11Session *session, GP11Attributes *priv, GP11Attributes *pub)
{
	GList *priv_prev, *pub_prev;
	
	g_assert (GP11_IS_SESSION (session));
	g_assert (priv);
	g_assert (pub);
	
	if (!login_session (session))
		return FALSE;

	gp11_attributes_add_boolean (priv, CKA_TOKEN, FALSE);
	gp11_attributes_add_boolean (pub, CKA_TOKEN, FALSE);
	
	/* Find the previous keys that match the same description */
	priv_prev = pub_prev = NULL;
	search_keys_like_attributes (session, priv, CKO_PRIVATE_KEY, list_all_matching, &priv_prev);
	search_keys_like_attributes (session, priv, CKO_PUBLIC_KEY, list_all_matching, &pub_prev);
	
	/* Now try and create the new keys */
	if (create_key_pair (session, priv, pub)) {
		
		/* Delete the old keys */
		destroy_replaced_keys (session, priv_prev);
		destroy_replaced_keys (session, pub_prev);
	}
	
	gp11_list_unref_free (priv_prev);
	gp11_list_unref_free (pub_prev);
		
	return TRUE;
}

static gboolean
load_contraints (EggBuffer *buffer, gsize offset, gsize *next_offset,
                 GP11Attributes *priv, GP11Attributes *pub)
{
	GTimeVal tv;
	guchar constraint;
	guint32 lifetime;
	time_t time;
	struct tm tm;
	gchar buf[20];

	/*
	 * Constraints are a byte flag, and optional data depending
	 * on the constraint.
	 */

	while (offset < egg_buffer_length (buffer)) {
		if (!egg_buffer_get_byte (buffer, offset, &offset, &constraint))
			return FALSE;

		switch (constraint) {
		case GCK_SSH_FLAG_CONSTRAIN_LIFETIME:
			if (!egg_buffer_get_uint32 (buffer, offset, &offset, &lifetime))
				return FALSE;

			g_get_current_time (&tv);
			time = tv.tv_sec + lifetime;

			if (!gmtime_r (&time, &tm))
				g_return_val_if_reached (FALSE);

			if (!strftime(buf, sizeof (buf), "%Y%m%d%H%M%S00", &tm))
				g_return_val_if_reached (FALSE);

			gp11_attributes_add_data (pub, CKA_GNOME_AUTO_DESTRUCT, buf, 16);
			gp11_attributes_add_data (priv, CKA_GNOME_AUTO_DESTRUCT, buf, 16);
			break;

		case GCK_SSH_FLAG_CONSTRAIN_CONFIRM:
			/* We can't use prompting as access control on an insecure X desktop */
			g_message ("prompt constraints are not supported.");
			return FALSE;

		default:
			g_message ("unsupported constraint or other unsupported data");
			return FALSE;
		}
	}

	*next_offset = offset;
	return TRUE;
}

/* -----------------------------------------------------------------------------
 * OPERATIONS
 */

static gboolean
op_add_identity (GckSshAgentCall *call)
{
	GP11Attributes *pub;
	GP11Attributes *priv;
	GP11Session *session;
	gchar *stype = NULL;
	gchar *comment = NULL;
	gboolean ret;
	gulong algo;
	gsize offset;
	
	if (!egg_buffer_get_string (call->req, 5, &offset, &stype, (EggBufferAllocator)g_realloc))
		return FALSE;
		
	algo = gck_ssh_agent_proto_keytype_to_algo (stype);
	if (algo == G_MAXULONG) {
		g_warning ("unsupported algorithm from SSH: %s", stype);
		g_free (stype);
		return FALSE;
	}

	g_free (stype);
	priv = gp11_attributes_new_full ((GP11Allocator)egg_secure_realloc);
	pub = gp11_attributes_new_full (g_realloc);
	
	switch (algo) {
	case CKK_RSA:
		ret = gck_ssh_agent_proto_read_pair_rsa (call->req, &offset, priv, pub);
		break;
	case CKK_DSA:
		ret = gck_ssh_agent_proto_read_pair_dsa (call->req, &offset, priv, pub);
		break;
	default:
		g_assert_not_reached ();
		return FALSE;
	}
	
	if (!ret) {
		g_warning ("couldn't read incoming SSH private key");
		gp11_attributes_unref (pub);
		gp11_attributes_unref (priv);
		return FALSE;
	}
		
	/* Get the comment */
	if (!egg_buffer_get_string (call->req, offset, &offset, &comment, (EggBufferAllocator)g_realloc)) {
		gp11_attributes_unref (pub);
		gp11_attributes_unref (priv);
		return FALSE;
	}
	
	gp11_attributes_add_string (pub, CKA_LABEL, comment);
	gp11_attributes_add_string (priv, CKA_LABEL, comment);
	g_free (comment);

	/* Any constraints on loading the key */
	if (!load_contraints (call->req, offset, &offset, priv, pub)) {
		gp11_attributes_unref (pub);
		gp11_attributes_unref (priv);
		return FALSE;
	}

	/* 
	 * This is the session that owns these objects. Only 
	 * one thread can use it at a time. 
	 */
	
	session = gck_ssh_agent_checkout_main_session ();
	g_return_val_if_fail (session, FALSE);
	
	ret = replace_key_pair (session, priv, pub);
	
	gck_ssh_agent_checkin_main_session (session);
	
	gp11_attributes_unref (priv);
	gp11_attributes_unref (pub);
	
	egg_buffer_add_byte (call->resp, ret ? GCK_SSH_RES_SUCCESS : GCK_SSH_RES_FAILURE);
	return TRUE;	
}

static gboolean
op_v1_add_identity (GckSshAgentCall *call)
{
	GP11Attributes *pub, *priv;
	GP11Session *session;
	gchar *comment = NULL;
	gboolean ret;
	gsize offset = 5;	
	guint32 unused;
	
	if (!egg_buffer_get_uint32 (call->req, offset, &offset, &unused))
		return FALSE;
	
	priv = gp11_attributes_new_full ((GP11Allocator)egg_secure_realloc);
	pub = gp11_attributes_new_full (g_realloc);

	if (!gck_ssh_agent_proto_read_pair_v1 (call->req, &offset, priv, pub)) {
		g_warning ("couldn't read incoming SSH private key");
		gp11_attributes_unref (pub);
		gp11_attributes_unref (priv);
		return FALSE;		
	}
	
	/* Get the comment */
	if (!egg_buffer_get_string (call->req, offset, &offset, &comment, (EggBufferAllocator)g_realloc)) {
		gp11_attributes_unref (pub);
		gp11_attributes_unref (priv);
		return FALSE;
	}

	g_free (comment);

	gp11_attributes_add_string (priv, CKA_LABEL, V1_LABEL);
	gp11_attributes_add_string (pub, CKA_LABEL, V1_LABEL);
	
	/* Any constraints on loading the key */
	if (!load_contraints (call->req, offset, &offset, priv, pub)) {
		gp11_attributes_unref (pub);
		gp11_attributes_unref (priv);
		return FALSE;
	}

	/* 
	 * This is the session that owns these objects. Only 
	 * one thread can use it at a time. 
	 */

	session = gck_ssh_agent_checkout_main_session ();
	g_return_val_if_fail (session, FALSE);
		
	ret = replace_key_pair (session, priv, pub);
	
	gck_ssh_agent_checkin_main_session (session);
	
	gp11_attributes_unref (priv);
	gp11_attributes_unref (pub);
	
	egg_buffer_add_byte (call->resp, ret ? GCK_SSH_RES_SUCCESS : GCK_SSH_RES_FAILURE);
	return TRUE;	
}

static gboolean
op_request_identities (GckSshAgentCall *call)
{
	GList *all_attrs, *l;
	GP11Attributes *attrs;
	gsize blobpos;
	gchar *comment;

	/* Find all the keys (we filter out v1 later) */
	/* TODO: Check SSH purpose */
	all_attrs = NULL;
	if (!gp11_module_enumerate_objects (call->module, 
	                                    load_identity_v2_attributes, &all_attrs,
	                                    CKA_CLASS, GP11_ULONG, CKO_PUBLIC_KEY,
	                                    GP11_INVALID)) {
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}
	
	egg_buffer_add_byte (call->resp, GCK_SSH_RES_IDENTITIES_ANSWER);
	egg_buffer_add_uint32 (call->resp, g_list_length (all_attrs));
	      
	for (l = all_attrs; l; l = g_list_next (l)) {
		
		attrs = l->data;
		
		/* Dig out the label */
		if (!gp11_attributes_find_string (attrs, CKA_LABEL, &comment))
			comment = NULL;
		
		/* Add a space for the key blob length */		
		blobpos = call->resp->len;
		egg_buffer_add_uint32 (call->resp, 0);

		/* Write out the key */
		gck_ssh_agent_proto_write_public (call->resp, attrs);
		
		/* Write back the blob length */
		egg_buffer_set_uint32 (call->resp, blobpos, (call->resp->len - blobpos) - 4);
		
		/* And now a per key comment */
		egg_buffer_add_string (call->resp, comment ? comment : "");
		
		g_free (comment);
		gp11_attributes_unref (attrs);
	}
	
	g_list_free (all_attrs);
	
	return TRUE;
}

static gboolean
op_v1_request_identities (GckSshAgentCall *call)
{
	GList *all_attrs, *l;
	GP11Attributes *attrs;
	
	/* Find all the keys not on token, and are V1 */
	/* TODO: Check SSH purpose */
	all_attrs = NULL;
	if (!gp11_module_enumerate_objects (call->module, 
	                                    load_identity_v1_attributes, &all_attrs,
	                                    CKA_CLASS, GP11_ULONG, CKO_PUBLIC_KEY,
	                                    CKA_TOKEN, GP11_BOOLEAN, FALSE,
	                                    CKA_LABEL, GP11_STRING, V1_LABEL,
	                                    GP11_INVALID)) {
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}
	
	egg_buffer_add_byte (call->resp, GCK_SSH_RES_RSA_IDENTITIES_ANSWER);
	egg_buffer_add_uint32 (call->resp, g_list_length (all_attrs));
	      
	for (l = all_attrs; l; l = g_list_next (l)) {
		
		attrs = l->data;
		
		/* Write out the key */
		gck_ssh_agent_proto_write_public_v1 (call->resp, attrs);
	
		/* And now a per key comment */
		egg_buffer_add_string (call->resp, "Public Key");
		
		gp11_attributes_unref (attrs);
	}
	
	g_list_free (all_attrs);
	
	return TRUE;
}

static const guchar SHA1_ASN[15] = /* Object ID is 1.3.14.3.2.26 */
	{ 0x30, 0x21, 0x30, 0x09, 0x06, 0x05, 0x2b, 0x0e, 0x03,
	  0x02, 0x1a, 0x05, 0x00, 0x04, 0x14 };

static const guchar MD5_ASN[18] = /* Object ID is 1.2.840.113549.2.5 */
	{ 0x30, 0x20, 0x30, 0x0c, 0x06, 0x08, 0x2a, 0x86,0x48,
	  0x86, 0xf7, 0x0d, 0x02, 0x05, 0x05, 0x00, 0x04, 0x10 };

static guchar*
make_pkcs1_sign_hash (GChecksumType algo, const guchar *data, gsize n_data, 
                      gsize *n_result)
{
	gsize n_algo, n_asn, n_hash;
	GChecksum *checksum;
	const guchar *asn;
	guchar *hash;
	
	g_assert (data);
	g_assert (n_result);
	
	n_algo = g_checksum_type_get_length (algo);
	g_return_val_if_fail (n_algo > 0, FALSE);
	
	if (algo == G_CHECKSUM_SHA1) {
		asn = SHA1_ASN;
		n_asn = sizeof (SHA1_ASN);
	} else if (algo == G_CHECKSUM_MD5) {
		asn = MD5_ASN;
		n_asn = sizeof (MD5_ASN);
	}
	
	n_hash = n_algo + n_asn;
	hash = g_malloc0 (n_hash);
	memcpy (hash, asn, n_asn);
	
	checksum = g_checksum_new (algo);
	g_checksum_update (checksum, data, n_data);
	g_checksum_get_digest (checksum, hash + n_asn, &n_algo);
	g_checksum_free (checksum);

	*n_result = n_hash;
	return hash;
}

static guchar*
make_raw_sign_hash (GChecksumType algo, const guchar *data, gsize n_data, 
                    gsize *n_result)
{
	gsize n_hash;
	GChecksum *checksum;
	guchar *hash;
	
	g_assert (data);
	g_assert (n_result);
	
	n_hash = g_checksum_type_get_length (algo);
	g_return_val_if_fail (n_hash > 0, FALSE);
	
	hash = g_malloc0 (n_hash);
	
	checksum = g_checksum_new (algo);
	g_checksum_update (checksum, data, n_data);
	g_checksum_get_digest (checksum, hash, &n_hash);
	g_checksum_free (checksum);

	*n_result = n_hash;
	return hash;
}

static gboolean 
op_sign_request (GckSshAgentCall *call)
{
	GP11Attributes *attrs;
	GError *error = NULL;
	GP11Object *key = NULL;
	const guchar *data;
	const gchar *salgo;
	GP11Session *session;
	guchar *result;
	gsize n_data, n_result;
	guint32 flags;
	gsize offset;
	gboolean ret;
	guint blobpos, sz;
	guint8 *hash;
	gulong algo, mech;
	GChecksumType halgo;
	gsize n_hash = 0;
	
	offset = 5;
	
	/* The key packet size */
	if (!egg_buffer_get_uint32 (call->req, offset, &offset, &sz))
		return FALSE;

	/* The key itself */
	attrs = gp11_attributes_new ();
	if (!gck_ssh_agent_proto_read_public (call->req, &offset, attrs, &algo))
		return FALSE;
	
	/* Validate the key type / mechanism */
	if (algo == CKK_RSA)
		mech = CKM_RSA_PKCS;
	else if (algo == CKK_DSA)
		mech = CKM_DSA;
	else
		g_return_val_if_reached (FALSE);

	if (!egg_buffer_get_byte_array (call->req, offset, &offset, &data, &n_data) ||
	    !egg_buffer_get_uint32 (call->req, offset, &offset, &flags)) {
		gp11_attributes_unref (attrs);
	    	return FALSE;
	}

	/* Lookup the key */
	search_keys_like_attributes (call->module, attrs, CKO_PUBLIC_KEY, return_private_matching, &key);
	gp11_attributes_unref (attrs);
	
	if (!key) {
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}
	
	/* Usually we hash the data with SHA1 */
	if (flags & GCK_SSH_FLAG_OLD_SIGNATURE)
		halgo = G_CHECKSUM_MD5;
	else
		halgo = G_CHECKSUM_SHA1;
	
	/* Build the hash */
	if (mech == CKM_RSA_PKCS)
		hash = make_pkcs1_sign_hash (halgo, data, n_data, &n_hash);
	else
		hash = make_raw_sign_hash (halgo, data, n_data, &n_hash);
	
	session = gp11_object_get_session (key);
	g_return_val_if_fail (session, FALSE);
	
	/* Do the magic */
	result = gp11_session_sign (session, key, mech, hash, n_hash, &n_result, &error);
	
	g_object_unref (session);
	g_object_unref (key);
	g_free (hash);
	
	if (error) {
		if (error->code != CKR_FUNCTION_CANCELED)
			g_message ("signing of the data failed: %s", error->message);
		g_clear_error (&error);
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}
	
	egg_buffer_add_byte (call->resp, GCK_SSH_RES_SIGN_RESPONSE);
	
	/* Add a space for the sig blob length */		
	blobpos = call->resp->len;
	egg_buffer_add_uint32 (call->resp, 0);
	
	salgo = gck_ssh_agent_proto_algo_to_keytype (algo);
	g_assert (salgo);
	egg_buffer_add_string (call->resp, salgo);

	switch (algo) {
	case CKK_RSA:
		ret = gck_ssh_agent_proto_write_signature_rsa (call->resp, result, n_result);
		break;

	case CKK_DSA:
		ret = gck_ssh_agent_proto_write_signature_dsa (call->resp, result, n_result);
		break;

	default:
		g_assert_not_reached ();
	}

	g_free (result);
	g_return_val_if_fail (ret, FALSE);
	
	/* Write back the blob length */
	egg_buffer_set_uint32 (call->resp, blobpos, (call->resp->len - blobpos) - 4);
	
	return TRUE; 
}

static gboolean 
op_v1_challenge (GckSshAgentCall *call)
{
	gsize offset, n_data, n_result, n_hash;
	GP11Session *session;
	GP11Attributes *attrs;
	guchar session_id[16];
	guint8 hash[16];
	const guchar *data;
	guchar *result = NULL;
	GChecksum *checksum;
	GP11Object *key = NULL;
	guint32 resp_type;
	GError *error = NULL;
	gboolean ret;
	guint i;
	guchar b;
	
	ret = FALSE;
	offset = 5;
	
	attrs = gp11_attributes_new ();
	if (!gck_ssh_agent_proto_read_public_v1 (call->req, &offset, attrs)) {
		gp11_attributes_unref (attrs);
		return FALSE;
	}
	
	/* Read the entire challenge */
	data = gck_ssh_agent_proto_read_challenge_v1 (call->req, &offset, &n_data);
	
	/* Only protocol 1.1 is supported */
	if (call->req->len <= offset) {
		gp11_attributes_unref (attrs);
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}
		
	/* Read out the session id, raw, unbounded */
	for (i = 0; i < 16; ++i) {
		egg_buffer_get_byte (call->req, offset, &offset, &b);
		session_id[i] = b;
	}
		
	/* And the response type */
	egg_buffer_get_uint32 (call->req, offset, &offset, &resp_type);
	
	/* Did parsing fail? */
	if (egg_buffer_has_error (call->req) || data == NULL) {
		gp11_attributes_unref (attrs);
		return FALSE;
	}
	
	/* Not supported request type */
	if (resp_type != 1) {
		gp11_attributes_unref (attrs);
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}
	
	/* Lookup the key */
	search_keys_like_attributes (call->module, attrs, CKO_PUBLIC_KEY, return_private_matching, &key);
	gp11_attributes_unref (attrs);
	
	/* Didn't find a key? */
	if (key == NULL) {
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}

	session = gp11_object_get_session (key);
	g_return_val_if_fail (session, FALSE);
	
	result = gp11_session_decrypt (session, key, CKM_RSA_PKCS, data, n_data, &n_result, &error);
	
	g_object_unref (session);
	g_object_unref (key);
	
	if (error) {
		if (error->code != CKR_FUNCTION_CANCELED)
			g_message ("decryption of the data failed: %s", error->message);
		g_clear_error (&error);
		egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
		return TRUE;
	}
	
	/* Now build up a hash of this and the session_id */
	checksum = g_checksum_new (G_CHECKSUM_MD5);
	g_checksum_update (checksum, result, n_result);
	g_checksum_update (checksum, session_id, sizeof (session_id));
	n_hash = sizeof (hash);
	g_checksum_get_digest (checksum, hash, &n_hash);
	
	egg_buffer_add_byte (call->resp, GCK_SSH_RES_RSA_RESPONSE);
	egg_buffer_append (call->resp, hash, n_hash);
	
	g_free (result);
	return TRUE;
}

static gboolean 
op_remove_identity (GckSshAgentCall *call)
{
	GP11Attributes *attrs;
	GP11Session *session;
	GP11Object *key = NULL;
	gsize offset;
	guint sz;
	
	offset = 5;
	
	/* The key packet size */
	if (!egg_buffer_get_uint32 (call->req, offset, &offset, &sz))
		return FALSE;

	/* The public key itself */
	attrs = gp11_attributes_new ();
	if (!gck_ssh_agent_proto_read_public (call->req, &offset, attrs, NULL)) {
		gp11_attributes_unref (attrs);
		return FALSE;
	}

	/* 
	 * This is the session that owns these objects. Only 
	 * one thread can use it at a time. 
	 */
	
	session = gck_ssh_agent_checkout_main_session ();
	g_return_val_if_fail (session, FALSE);

	search_keys_like_attributes (session, attrs, CKO_PUBLIC_KEY, return_first_matching, &key);
	gp11_attributes_unref (attrs);
	
	if (key != NULL) { 
		remove_by_public_key (session, key, TRUE);
		g_object_unref (key);
	}

	gck_ssh_agent_checkin_main_session (session);

	egg_buffer_add_byte (call->resp, GCK_SSH_RES_SUCCESS);

	return TRUE;	
}

static gboolean 
op_v1_remove_identity (GckSshAgentCall *call)
{
	GP11Session *session;
	GP11Attributes *attrs;
	GP11Object *key = NULL;
	gsize offset;
	
	offset = 5;
	
	attrs = gp11_attributes_new ();
	if (!gck_ssh_agent_proto_read_public_v1 (call->req, &offset, attrs)) {
		gp11_attributes_unref (attrs);		
		return FALSE;
	}

	/* 
	 * This is the session that owns these objects. Only 
	 * one thread can use it at a time. 
	 */

	session = gck_ssh_agent_checkout_main_session ();
	g_return_val_if_fail (session, FALSE);

	search_keys_like_attributes (session, attrs, CKO_PUBLIC_KEY, return_first_matching, &key);
	gp11_attributes_unref (attrs);
	
	if (key != NULL) { 
		remove_by_public_key (session, key, FALSE);
		g_object_unref (key);
	}

	gck_ssh_agent_checkin_main_session (session);

	egg_buffer_add_byte (call->resp, GCK_SSH_RES_SUCCESS);
	return TRUE;	
}

static gboolean 
op_remove_all_identities (GckSshAgentCall *call)
{
	GP11Session *session;
	GList *objects, *l;
	GError *error = NULL;
	
	/* 
	 * This is the session that owns these objects. Only 
	 * one thread can use it at a time. 
	 */
	
	session = gck_ssh_agent_checkout_main_session ();
	g_return_val_if_fail (session, FALSE);
	
	/* Find all session SSH public keys */
	objects = gp11_session_find_objects (session, &error,
	                                     CKA_CLASS, GP11_ULONG, CKO_PUBLIC_KEY,
	                                     GP11_INVALID);
	

	for (l = objects; l; l = g_list_next (l)) 
		remove_by_public_key (session, l->data, TRUE);

	gp11_list_unref_free (objects);

	gck_ssh_agent_checkin_main_session (session);

	egg_buffer_add_byte (call->resp, GCK_SSH_RES_SUCCESS);
	return TRUE;
}

static gboolean 	
op_v1_remove_all_identities (GckSshAgentCall *call)
{
	GP11Session *session;
	GList *objects, *l;
	GError *error = NULL;

	/* 
	 * This is the session that owns these objects. Only 
	 * one thread can use it at a time. 
	 */
	
	session = gck_ssh_agent_checkout_main_session ();
	g_return_val_if_fail (session, FALSE);

	/* Find all session SSH v1 public keys */
	objects = gp11_session_find_objects (session, &error,
	                                     CKA_TOKEN, GP11_BOOLEAN, FALSE,
	                                     CKA_CLASS, GP11_ULONG, CKO_PUBLIC_KEY,
	                                     CKA_LABEL, GP11_STRING, V1_LABEL,
	                                     GP11_INVALID);

	for (l = objects; l; l = g_list_next (l)) 
		remove_by_public_key (session, l->data, FALSE);

	gp11_list_unref_free (objects);

	gck_ssh_agent_checkin_main_session (session);
		
	egg_buffer_add_byte (call->resp, GCK_SSH_RES_SUCCESS);
	return TRUE;
}

static gboolean 
op_not_implemented_success (GckSshAgentCall *call)
{
	egg_buffer_add_byte (call->resp, GCK_SSH_RES_SUCCESS);
	return TRUE;
}
	
static gboolean
op_not_implemented_failure (GckSshAgentCall *call)
{
	egg_buffer_add_byte (call->resp, GCK_SSH_RES_FAILURE);
	return TRUE;
}

static gboolean
op_invalid (GckSshAgentCall *call)
{
	/* Invalid request, disconnect immediately */
	return FALSE;
}

const GckSshAgentOperation gck_ssh_agent_operations[GCK_SSH_OP_MAX] = {
     op_invalid,                                 /* 0 */
     op_v1_request_identities,                   /* GKR_SSH_OP_REQUEST_RSA_IDENTITIES */
     op_invalid,                                 /* 2 */
     op_v1_challenge,                            /* GKR_SSH_OP_RSA_CHALLENGE */
     op_invalid,                                 /* 4 */
     op_invalid,                                 /* 5 */
     op_invalid,                                 /* 6 */
     op_v1_add_identity,                         /* GKR_SSH_OP_ADD_RSA_IDENTITY */
     op_v1_remove_identity,                      /* GKR_SSH_OP_REMOVE_RSA_IDENTITY */
     op_v1_remove_all_identities,                /* GKR_SSH_OP_REMOVE_ALL_RSA_IDENTITIES */
     op_invalid,                                 /* 10 */     
     op_request_identities,                      /* GKR_SSH_OP_REQUEST_IDENTITIES */
     op_invalid,                                 /* 12 */
     op_sign_request,                            /* GKR_SSH_OP_SIGN_REQUEST */
     op_invalid,                                 /* 14 */     
     op_invalid,                                 /* 15 */     
     op_invalid,                                 /* 16 */     
     op_add_identity,                            /* GKR_SSH_OP_ADD_IDENTITY */
     op_remove_identity,                         /* GKR_SSH_OP_REMOVE_IDENTITY */
     op_remove_all_identities,                   /* GKR_SSH_OP_REMOVE_ALL_IDENTITIES */
     op_not_implemented_failure,                 /* GKR_SSH_OP_ADD_SMARTCARD_KEY */
     op_not_implemented_failure,                 /* GKR_SSH_OP_REMOVE_SMARTCARD_KEY */
     op_not_implemented_success,                 /* GKR_SSH_OP_LOCK */
     op_not_implemented_success,                 /* GKR_SSH_OP_UNLOCK */
     op_v1_add_identity,                         /* GKR_SSH_OP_ADD_RSA_ID_CONSTRAINED */
     op_add_identity,                            /* GKR_SSH_OP_ADD_ID_CONSTRAINED */
     op_not_implemented_failure,                 /* GKR_SSH_OP_ADD_SMARTCARD_KEY_CONSTRAINED */
};
