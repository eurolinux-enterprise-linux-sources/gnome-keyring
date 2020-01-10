/*
 * gnome-keyring
 *
 * Copyright (C) 2008 Stefan Walter
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include "gkd-secret-property.h"

#include "pkcs11/pkcs11i.h"

#include <string.h>


typedef enum _DataType {
	DATA_TYPE_INVALID = 0,

	/*
	 * The attribute is a CK_BBOOL.
	 * Property is DBUS_TYPE_BOOLEAN
	 */
	DATA_TYPE_BOOL,

	/*
	 * The attribute is in the format: "%Y%m%d%H%M%S00"
	 * Property is DBUS_TYPE_UINT64 since 1970 epoch.
	 */
	DATA_TYPE_TIME,

	/*
	 * The attribute is a CK_UTF8_CHAR string, not null-terminated
	 * Property is a DBUS_TYPE_STRING
	 */
	DATA_TYPE_STRING,

	/*
	 * The attribute is in the format: name\0value\0name2\0value2
	 * Property is dbus dictionary of strings: a{ss}
	 */
	DATA_TYPE_FIELDS
} DataType;

/* -----------------------------------------------------------------------------
 * INTERNAL
 */

static gboolean
property_to_attribute (const gchar *prop_name, const gchar *interface,
                       CK_ATTRIBUTE_TYPE *attr_type, DataType *data_type)
{
	g_return_val_if_fail (prop_name, FALSE);
	g_assert (attr_type);
	g_assert (data_type);

	/* If an interface is desired, check that it matches, and remove */
	if (interface) {
		if (!g_str_has_prefix (prop_name, interface))
			return FALSE;

		prop_name += strlen (interface);
		if (prop_name[0] != '.')
			return FALSE;
		++prop_name;
	}

	if (g_str_equal (prop_name, "Label")) {
		*attr_type = CKA_LABEL;
		*data_type = DATA_TYPE_STRING;

	/* Non-standard property for type schema */
	} else if (g_str_equal (prop_name, "Type")) {
		*attr_type = CKA_G_SCHEMA;
		*data_type = DATA_TYPE_STRING;

	} else if (g_str_equal (prop_name, "Locked")) {
		*attr_type = CKA_G_LOCKED;
		*data_type = DATA_TYPE_BOOL;

	} else if (g_str_equal (prop_name, "Created")) {
		*attr_type = CKA_G_CREATED;
		*data_type = DATA_TYPE_TIME;

	} else if (g_str_equal (prop_name, "Modified")) {
		*attr_type = CKA_G_MODIFIED;
		*data_type = DATA_TYPE_TIME;

	} else if (g_str_equal (prop_name, "Attributes")) {
		*attr_type = CKA_G_FIELDS;
		*data_type = DATA_TYPE_FIELDS;

	} else {
		return FALSE;
	}

	return TRUE;
}

static gboolean
attribute_to_property (CK_ATTRIBUTE_TYPE attr_type, const gchar **prop_name, DataType *data_type)
{
	g_assert (prop_name);
	g_assert (data_type);

	switch (attr_type) {
	case CKA_LABEL:
		*prop_name = "Label";
		*data_type = DATA_TYPE_STRING;
		break;
	/* Non-standard property for type schema */
	case CKA_G_SCHEMA:
		*prop_name = "Type";
		*data_type = DATA_TYPE_STRING;
		break;
	case CKA_G_LOCKED:
		*prop_name = "Locked";
		*data_type = DATA_TYPE_BOOL;
		break;
	case CKA_G_CREATED:
		*prop_name = "Created";
		*data_type = DATA_TYPE_TIME;
		break;
	case CKA_G_MODIFIED:
		*prop_name = "Modified";
		*data_type = DATA_TYPE_TIME;
		break;
	case CKA_G_FIELDS:
		*prop_name = "Attributes";
		*data_type = DATA_TYPE_FIELDS;
		break;
	default:
		return FALSE;
	};

	return TRUE;
}

typedef void (*IterAppendFunc) (DBusMessageIter*, const GckAttribute *);
typedef gboolean (*IterGetFunc) (DBusMessageIter*, gulong, GckBuilder *);

static void
iter_append_string (DBusMessageIter *iter,
                    const GckAttribute *attr)
{
	gchar *value;

	g_assert (iter);
	g_assert (attr);

	if (attr->length == 0) {
		value = "";
		dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &value);
	} else {
		value = g_strndup ((const gchar*)attr->value, attr->length);
		dbus_message_iter_append_basic (iter, DBUS_TYPE_STRING, &value);
		g_free (value);
	}
}

static gboolean
iter_get_string (DBusMessageIter *iter,
                 gulong attr_type,
                 GckBuilder *builder)
{
	const char *value;

	g_assert (iter != NULL);
	g_assert (builder != NULL);

	g_return_val_if_fail (dbus_message_iter_get_arg_type (iter) == DBUS_TYPE_STRING, FALSE);
	dbus_message_iter_get_basic (iter, &value);
	if (value == NULL)
		value = "";
	gck_builder_add_string (builder, attr_type, value);
	return TRUE;
}

static void
iter_append_bool (DBusMessageIter *iter,
                  const GckAttribute *attr)
{
	dbus_bool_t value;

	g_assert (iter);
	g_assert (attr);

	value = gck_attribute_get_boolean (attr) ? TRUE : FALSE;
	dbus_message_iter_append_basic (iter, DBUS_TYPE_BOOLEAN, &value);
}

static gboolean
iter_get_bool (DBusMessageIter *iter,
               gulong attr_type,
               GckBuilder *builder)
{
	dbus_bool_t value;

	g_assert (iter != NULL);
	g_assert (builder != NULL);

	g_return_val_if_fail (dbus_message_iter_get_arg_type (iter) == DBUS_TYPE_BOOLEAN, FALSE);
	dbus_message_iter_get_basic (iter, &value);

	gck_builder_add_boolean (builder, attr_type, value ? TRUE : FALSE);
	return TRUE;
}

static void
iter_append_time (DBusMessageIter *iter,
                  const GckAttribute *attr)
{
	guint64 value;
	struct tm tm;
	gchar buf[15];
	time_t time;

	g_assert (iter);
	g_assert (attr);

	if (attr->length == 0) {
		value = 0;

	} else if (!attr->value || attr->length != 16) {
		g_warning ("invalid length of time attribute");
		value = 0;

	} else {
		memset (&tm, 0, sizeof (tm));
		memcpy (buf, attr->value, 14);
		buf[14] = 0;

		if (!strptime(buf, "%Y%m%d%H%M%S", &tm)) {
			g_warning ("invalid format of time attribute");
			value = 0;
		} else {
			/* Convert to seconds since epoch */
			time = timegm (&tm);
			if (time < 0) {
				g_warning ("invalid time attribute");
				value = 0;
			} else {
				value = time;
			}
		}
	}

	dbus_message_iter_append_basic (iter, DBUS_TYPE_UINT64, &value);
}

static gboolean
iter_get_time (DBusMessageIter *iter,
               gulong attr_type,
               GckBuilder *builder)
{
	time_t time;
	struct tm tm;
	gchar buf[20];
	guint64 value;

	g_assert (iter != NULL);
	g_assert (builder != NULL);

	g_return_val_if_fail (dbus_message_iter_get_arg_type (iter) == DBUS_TYPE_UINT64, FALSE);
	dbus_message_iter_get_basic (iter, &value);
	if (value == 0) {
		gck_builder_add_empty (builder, attr_type);
		return TRUE;
	}

	time = value;
	if (!gmtime_r (&time, &tm))
		g_return_val_if_reached (FALSE);

	if (!strftime (buf, sizeof (buf), "%Y%m%d%H%M%S00", &tm))
		g_return_val_if_reached (FALSE);

	gck_builder_add_data (builder, attr_type, (const guchar *)buf, 16);
	return TRUE;
}

static void
iter_append_fields (DBusMessageIter *iter,
                    const GckAttribute *attr)
{
	DBusMessageIter array;
	DBusMessageIter dict;
	const gchar *ptr;
	const gchar *last;
	const gchar *name;
	gsize n_name;
	const gchar *value;
	gsize n_value;
	gchar *string;

	g_assert (iter);
	g_assert (attr);

	ptr = (gchar*)attr->value;
	last = ptr + attr->length;
	g_return_if_fail (ptr || last == ptr);

	dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, "{ss}", &array);

	while (ptr && ptr != last) {
		g_assert (ptr < last);

		name = ptr;
		ptr = memchr (ptr, 0, last - ptr);
		if (ptr == NULL) /* invalid */
			break;

		n_name = ptr - name;
		value = ++ptr;
		ptr = memchr (ptr, 0, last - ptr);
		if (ptr == NULL) /* invalid */
			break;

		n_value = ptr - value;
		++ptr;

		dbus_message_iter_open_container (&array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);

		string = g_strndup (name, n_name);
		dbus_message_iter_append_basic (&dict, DBUS_TYPE_STRING, &string);
		g_free (string);

		string = g_strndup (value, n_value);
		dbus_message_iter_append_basic (&dict, DBUS_TYPE_STRING, &string);
		g_free (string);

		dbus_message_iter_close_container (&array, &dict);
	}

	dbus_message_iter_close_container (iter, &array);
}

static gboolean
iter_get_fields (DBusMessageIter *iter,
                 gulong attr_type,
                 GckBuilder *builder)
{
	DBusMessageIter array;
	DBusMessageIter dict;
	GString *result;
	const gchar *string;

	g_assert (iter != NULL);
	g_assert (builder != NULL);

	result = g_string_new ("");

	g_return_val_if_fail (dbus_message_iter_get_arg_type (iter) == DBUS_TYPE_ARRAY, FALSE);
	dbus_message_iter_recurse (iter, &array);

	while (dbus_message_iter_get_arg_type (&array) == DBUS_TYPE_DICT_ENTRY) {
		dbus_message_iter_recurse (&array, &dict);

		/* Key */
		g_return_val_if_fail (dbus_message_iter_get_arg_type (&dict) == DBUS_TYPE_STRING, FALSE);
		dbus_message_iter_get_basic (&dict, &string);
		g_string_append (result, string);
		g_string_append_c (result, '\0');

		dbus_message_iter_next (&dict);

		/* Value */
		g_return_val_if_fail (dbus_message_iter_get_arg_type (&dict) == DBUS_TYPE_STRING, FALSE);
		dbus_message_iter_get_basic (&dict, &string);
		g_string_append (result, string);
		g_string_append_c (result, '\0');

		dbus_message_iter_next (&array);
	}

	gck_builder_add_data (builder, attr_type, (const guchar *)result->str, result->len);
	g_string_free (result, TRUE);
	return TRUE;
}

static void
iter_append_variant (DBusMessageIter *iter,
                     DataType data_type,
                     const GckAttribute *attr)
{
	DBusMessageIter sub;
	IterAppendFunc func = NULL;
	const gchar *sig = NULL;

	g_assert (iter);
	g_assert (attr);

	switch (data_type) {
	case DATA_TYPE_STRING:
		func = iter_append_string;
		sig = DBUS_TYPE_STRING_AS_STRING;
		break;
	case DATA_TYPE_BOOL:
		func = iter_append_bool;
		sig = DBUS_TYPE_BOOLEAN_AS_STRING;
		break;
	case DATA_TYPE_TIME:
		func = iter_append_time;
		sig = DBUS_TYPE_UINT64_AS_STRING;
		break;
	case DATA_TYPE_FIELDS:
		func = iter_append_fields;
		sig = "a{ss}";
		break;
	default:
		g_assert (FALSE);
		break;
	}

	dbus_message_iter_open_container (iter, DBUS_TYPE_VARIANT, sig, &sub);
	(func) (&sub, attr);
	dbus_message_iter_close_container (iter, &sub);
}

static gboolean
iter_get_variant (DBusMessageIter *iter,
                  DataType data_type,
                  gulong attr_type,
                  GckBuilder *builder)
{
	DBusMessageIter variant;
	IterGetFunc func = NULL;
	gboolean ret;
	const gchar *sig = NULL;
	char *signature;

	g_assert (iter != NULL);
	g_assert (builder != NULL);

	g_return_val_if_fail (dbus_message_iter_get_arg_type (iter) == DBUS_TYPE_VARIANT, FALSE);
	dbus_message_iter_recurse (iter, &variant);

	switch (data_type) {
	case DATA_TYPE_STRING:
		func = iter_get_string;
		sig = DBUS_TYPE_STRING_AS_STRING;
		break;
	case DATA_TYPE_BOOL:
		func = iter_get_bool;
		sig = DBUS_TYPE_BOOLEAN_AS_STRING;
		break;
	case DATA_TYPE_TIME:
		func = iter_get_time;
		sig = DBUS_TYPE_UINT64_AS_STRING;
		break;
	case DATA_TYPE_FIELDS:
		func = iter_get_fields;
		sig = "a{ss}";
		break;
	default:
		g_assert (FALSE);
		break;
	}

	signature = dbus_message_iter_get_signature (&variant);
	g_return_val_if_fail (signature, FALSE);
	ret = g_str_equal (sig, signature);
	dbus_free (signature);

	if (ret == FALSE)
		return FALSE;

	return (func) (&variant, attr_type, builder);
}

/* -----------------------------------------------------------------------------
 * PUBLIC
 */

gboolean
gkd_secret_property_get_type (const gchar *property, CK_ATTRIBUTE_TYPE *type)
{
	DataType data_type;

	g_return_val_if_fail (property, FALSE);
	g_return_val_if_fail (type, FALSE);

	return property_to_attribute (property, NULL, type, &data_type);
}

gboolean
gkd_secret_property_parse_all (DBusMessageIter *array,
                               const gchar *interface,
                               GckBuilder *builder)
{
	DBusMessageIter dict;
	CK_ATTRIBUTE_TYPE attr_type;
	const char *name;
	DataType data_type;

	g_return_val_if_fail (array != NULL, FALSE);
	g_return_val_if_fail (builder != NULL, FALSE);

	while (dbus_message_iter_get_arg_type (array) == DBUS_TYPE_DICT_ENTRY) {
		dbus_message_iter_recurse (array, &dict);

		/* Property interface.name */
		g_return_val_if_fail (dbus_message_iter_get_arg_type (&dict) == DBUS_TYPE_STRING, FALSE);
		dbus_message_iter_get_basic (&dict, &name);
		dbus_message_iter_next (&dict);

		if (!property_to_attribute (name, interface, &attr_type, &data_type))
			return FALSE;

		/* Property value */
		g_return_val_if_fail (dbus_message_iter_get_arg_type (&dict) == DBUS_TYPE_VARIANT, FALSE);
		if (!iter_get_variant (&dict, data_type, attr_type, builder))
			return FALSE;

		dbus_message_iter_next (array);
	}

	return TRUE;
}

gboolean
gkd_secret_property_append_all (DBusMessageIter *array, GckAttributes *attrs)
{
	DBusMessageIter dict;
	const GckAttribute *attr;
	DataType data_type;
	const gchar *name;
	gulong num, i;

	g_return_val_if_fail (array, FALSE);
	g_return_val_if_fail (attrs, FALSE);

	num = gck_attributes_count (attrs);
	for (i = 0; i < num; ++i) {
		attr = gck_attributes_at (attrs, i);
		if (!attribute_to_property (attr->type, &name, &data_type))
			g_return_val_if_reached (FALSE);

		dbus_message_iter_open_container (array, DBUS_TYPE_DICT_ENTRY, NULL, &dict);
		dbus_message_iter_append_basic (&dict, DBUS_TYPE_STRING, &name);
		iter_append_variant (&dict, data_type, attr);
		dbus_message_iter_close_container (array, &dict);
	}

	return TRUE;
}

gboolean
gkd_secret_property_append_variant (DBusMessageIter *iter,
                                    const GckAttribute *attr)
{
	const gchar *property;
	DataType data_type;

	g_return_val_if_fail (attr, FALSE);
	g_return_val_if_fail (iter, FALSE);

	if (!attribute_to_property (attr->type, &property, &data_type))
		return FALSE;
	iter_append_variant (iter, data_type, attr);
	return TRUE;
}

gboolean
gkd_secret_property_parse_variant (DBusMessageIter *iter,
                                   const gchar *property,
                                   GckBuilder *builder)
{
	CK_ATTRIBUTE_TYPE attr_type;
	DataType data_type;

	g_return_val_if_fail (iter, FALSE);
	g_return_val_if_fail (property, FALSE);
	g_return_val_if_fail (builder != NULL, FALSE);

	if (!property_to_attribute (property, NULL, &attr_type, &data_type))
		return FALSE;

	return iter_get_variant (iter, data_type, attr_type, builder);
}

gboolean
gkd_secret_property_parse_fields (DBusMessageIter *iter,
                                  GckBuilder *builder)
{
	g_return_val_if_fail (iter != NULL, FALSE);
	g_return_val_if_fail (builder != NULL, FALSE);

	return iter_get_fields (iter, CKA_G_FIELDS, builder);
}
