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
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#include "config.h"

#include "gp11/gp11.h"

#include "gcr-internal.h"
#include "gcr-marshal.h"
#include "gcr-parser.h"
#include "gcr-types.h"

#include "egg/egg-asn1.h"
#include "egg/egg-openssl.h"
#include "egg/egg-secure-memory.h"
#include "egg/egg-symkey.h"

#include <glib/gi18n-lib.h>

#include <stdlib.h>
#include <gcrypt.h>
#include <libtasn1.h>

enum {
	PROP_0,
	PROP_PARSED_LABEL,
	PROP_PARSED_ATTRIBUTES,
	PROP_PARSED_DESCRIPTION
};

enum {
	AUTHENTICATE,
	PARSED,
	LAST_SIGNAL
};

#define SUCCESS 0

static guint signals[LAST_SIGNAL] = { 0 };

struct _GcrParserPrivate {
	GTree *specific_formats;
	gboolean normal_formats;
	GPtrArray *passwords;
	
	GP11Attributes *parsed_attrs;
	const gchar *parsed_desc;
	gchar *parsed_label;
};

G_DEFINE_TYPE (GcrParser, gcr_parser, G_TYPE_OBJECT);

typedef struct {
	gint ask_state;
	gint seen;
} PasswordState;

#define PASSWORD_STATE_INIT { 0, 0 }

typedef struct _ParserFormat {
	gint format_id;
	gint (*function) (GcrParser *self, const guchar *data, gsize n_data);
} ParserFormat;

/* Forward declarations */
static const ParserFormat parser_normal[];
static const ParserFormat parser_formats[];
static ParserFormat* parser_format_lookup (gint format_id);

/* -----------------------------------------------------------------------------
 * QUARK DEFINITIONS
 */

/* 
 * PEM STRINGS 
 * The xxxxx in: ----- BEGIN xxxxx ------
 */ 
 
static GQuark PEM_CERTIFICATE;
static GQuark PEM_RSA_PRIVATE_KEY;
static GQuark PEM_DSA_PRIVATE_KEY;
static GQuark PEM_ANY_PRIVATE_KEY;
static GQuark PEM_ENCRYPTED_PRIVATE_KEY;
static GQuark PEM_PRIVATE_KEY;
static GQuark PEM_PKCS7;
static GQuark PEM_PKCS12;

/* 
 * OIDS
 */

static GQuark OID_PKIX1_RSA;
static GQuark OID_PKIX1_DSA;
static GQuark OID_PKCS7_DATA;
static GQuark OID_PKCS7_SIGNED_DATA;
static GQuark OID_PKCS7_ENCRYPTED_DATA;
static GQuark OID_PKCS12_BAG_PKCS8_KEY;
static GQuark OID_PKCS12_BAG_PKCS8_ENCRYPTED_KEY;
static GQuark OID_PKCS12_BAG_CERTIFICATE;
static GQuark OID_PKCS12_BAG_CRL;

static void
init_quarks (void)
{
	static volatile gsize quarks_inited = 0;

	if (g_once_init_enter (&quarks_inited)) {

		#define QUARK(name, value) \
			name = g_quark_from_static_string(value)
	 
		QUARK (OID_PKIX1_RSA, "1.2.840.113549.1.1.1");
		QUARK (OID_PKIX1_DSA, "1.2.840.10040.4.1");
		QUARK (OID_PKCS7_DATA, "1.2.840.113549.1.7.1");
		QUARK (OID_PKCS7_SIGNED_DATA, "1.2.840.113549.1.7.2");
		QUARK (OID_PKCS7_ENCRYPTED_DATA, "1.2.840.113549.1.7.6");
		QUARK (OID_PKCS12_BAG_PKCS8_KEY, "1.2.840.113549.1.12.10.1.1");
		QUARK (OID_PKCS12_BAG_PKCS8_ENCRYPTED_KEY, "1.2.840.113549.1.12.10.1.2");
		QUARK (OID_PKCS12_BAG_CERTIFICATE, "1.2.840.113549.1.12.10.1.3");
		QUARK (OID_PKCS12_BAG_CRL, "1.2.840.113549.1.12.10.1.4");
		
		QUARK (PEM_CERTIFICATE, "CERTIFICATE");
		QUARK (PEM_PRIVATE_KEY, "PRIVATE KEY");
		QUARK (PEM_RSA_PRIVATE_KEY, "RSA PRIVATE KEY");
		QUARK (PEM_DSA_PRIVATE_KEY, "DSA PRIVATE KEY");
		QUARK (PEM_ANY_PRIVATE_KEY, "ANY PRIVATE KEY");
		QUARK (PEM_ENCRYPTED_PRIVATE_KEY, "ENCRYPTED PRIVATE KEY");
		QUARK (PEM_PKCS7, "PKCS7");
		QUARK (PEM_PKCS12, "PKCS12");
		
		#undef QUARK
		
		g_once_init_leave (&quarks_inited, 1);
	}
}

/* -----------------------------------------------------------------------------
 * INTERNAL
 */

static gboolean
parsed_asn1_attribute (GcrParser *self, ASN1_TYPE asn, const guchar *data, gsize n_data, 
                       const gchar *part, CK_ATTRIBUTE_TYPE type)
{
	const guchar *value;
	gsize n_value;
	
	g_assert (GCR_IS_PARSER (self));
	g_assert (asn);
	g_assert (data);
	g_assert (part);
	g_assert (self->pv->parsed_attrs);
	
	value = egg_asn1_read_content (asn, data, n_data, part, &n_value);
	if (value == NULL)
		return FALSE;
	
	gp11_attributes_add_data (self->pv->parsed_attrs, type, value, n_value);
	return TRUE;
}

static void
parsed_clear (GcrParser *self, CK_OBJECT_CLASS klass)
{
	if (self->pv->parsed_attrs)
		gp11_attributes_unref (self->pv->parsed_attrs);
	if (klass == CKO_PRIVATE_KEY)
		self->pv->parsed_attrs = gp11_attributes_new_full ((GP11Allocator)egg_secure_realloc);
	else 
		self->pv->parsed_attrs = gp11_attributes_new ();
	gp11_attributes_add_ulong (self->pv->parsed_attrs, CKA_CLASS, klass);
	
	g_free (self->pv->parsed_label);
	self->pv->parsed_label = NULL;
	
	switch (klass) {
	case CKO_PRIVATE_KEY:
		self->pv->parsed_desc = _("Private Key");
		break;
	case CKO_CERTIFICATE:
		self->pv->parsed_desc = _("Certificate");
		break;
	case CKO_PUBLIC_KEY:
		self->pv->parsed_desc = _("Public Key");
		break;
	default:
		self->pv->parsed_desc = NULL;
		break;
	}
}

static void
parsed_label (GcrParser *self, const gchar *label)
{
	g_free (self->pv->parsed_label);
	self->pv->parsed_label = g_strdup (label);
}

static void
parsed_attribute (GcrParser *self, CK_ATTRIBUTE_TYPE type, gconstpointer data, gsize n_data)
{
	g_assert (GCR_IS_PARSER (self));
	g_assert (self->pv->parsed_attrs);
	gp11_attributes_add_data (self->pv->parsed_attrs, type, data, n_data);
}

static void
parsed_ulong (GcrParser *self, CK_ATTRIBUTE_TYPE type, gulong value)
{
	g_assert (GCR_IS_PARSER (self));
	g_assert (self->pv->parsed_attrs);
	gp11_attributes_add_ulong (self->pv->parsed_attrs, type, value);
}

static gint
enum_next_password (GcrParser *self, PasswordState *state, const gchar **password)
{
	gboolean result;

	/* 
	 * Next passes we look through all the passwords that the parser 
	 * has seen so far. This is because different parts of a encrypted
	 * container (such as PKCS#12) often use the same password even 
	 * if with different algorithms. 
	 * 
	 * If we didn't do this and the user chooses enters a password, 
	 * but doesn't save it, they would get prompted for the same thing
	 * over and over, dumb.  
	 */
	
	/* Look in our list of passwords */
	if (state->seen < self->pv->passwords->len) {
		g_assert (state->seen >= 0);
		*password = g_ptr_array_index (self->pv->passwords, state->seen);
		++state->seen;
		return SUCCESS;
	}
	
	/* Fire off all the parsed property signals so anyone watching can update their state */
	g_object_notify (G_OBJECT (self), "parsed-description");
	g_object_notify (G_OBJECT (self), "parsed-attributes");
	g_object_notify (G_OBJECT (self), "parsed-label");
	
	g_signal_emit (self, signals[AUTHENTICATE], 0, state->ask_state, &result);
	++state->ask_state;
	
	if (!result)
		return GCR_ERROR_CANCELLED;
	
	/* Return any passwords added */
	if (state->seen < self->pv->passwords->len) {
		g_assert (state->seen >= 0);
		*password = g_ptr_array_index (self->pv->passwords, state->seen);
		++state->seen;
		return SUCCESS;
	}
	
	return GCR_ERROR_LOCKED;
}

static void
parsed_fire (GcrParser *self)
{
	g_object_notify (G_OBJECT (self), "parsed-description");
	g_object_notify (G_OBJECT (self), "parsed-attributes");
	g_object_notify (G_OBJECT (self), "parsed-label");
	
	g_signal_emit (self, signals[PARSED], 0);
}

/* -----------------------------------------------------------------------------
 * RSA PRIVATE KEY
 */

static gint
parse_der_private_key_rsa (GcrParser *self, const guchar *data, gsize n_data)
{
	gint res = GCR_ERROR_UNRECOGNIZED;
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	guint version;
	
	asn = egg_asn1_decode ("PK.RSAPrivateKey", data, n_data);
	if (!asn)
		goto done;
	
	parsed_clear (self, CKO_PRIVATE_KEY);
	parsed_ulong (self, CKA_KEY_TYPE, CKK_RSA);
	res = GCR_ERROR_FAILURE;

	if (!egg_asn1_read_uint (asn, "version", &version))
		goto done;
	
	/* We only support simple version */
	if (version != 0) {
		res = GCR_ERROR_UNRECOGNIZED;
		g_message ("unsupported version of RSA key: %u", version);
		goto done;
	}
	
	if (!parsed_asn1_attribute (self, asn, data, n_data, "modulus", CKA_MODULUS) || 
	    !parsed_asn1_attribute (self, asn, data, n_data, "publicExponent", CKA_PUBLIC_EXPONENT) ||
	    !parsed_asn1_attribute (self, asn, data, n_data, "privateExponent", CKA_PRIVATE_EXPONENT) ||
            !parsed_asn1_attribute (self, asn, data, n_data, "prime1", CKA_PRIME_1) ||
            !parsed_asn1_attribute (self, asn, data, n_data, "prime2", CKA_PRIME_2) || 
            !parsed_asn1_attribute (self, asn, data, n_data, "coefficient", CKA_COEFFICIENT))
		goto done;
	
	parsed_fire (self);
	res = SUCCESS;

done:
	if (asn)
		asn1_delete_structure (&asn);

	if (res == GCR_ERROR_FAILURE)
		g_message ("invalid RSA key");
	
	return res;
}

/* -----------------------------------------------------------------------------
 * DSA PRIVATE KEY
 */

static gint
parse_der_private_key_dsa (GcrParser *self, const guchar *data, gsize n_data)
{
	gint ret = GCR_ERROR_UNRECOGNIZED;
	int res;
	ASN1_TYPE asn;

	asn = egg_asn1_decode ("PK.DSAPrivateKey", data, n_data);
	if (!asn)
		goto done;
	
	parsed_clear (self, CKO_PRIVATE_KEY);
	parsed_ulong (self, CKA_KEY_TYPE, CKK_DSA);
	res = GCR_ERROR_FAILURE;

	if (!parsed_asn1_attribute (self, asn, data, n_data, "p", CKA_PRIME) ||
	    !parsed_asn1_attribute (self, asn, data, n_data, "q", CKA_SUBPRIME) ||
	    !parsed_asn1_attribute (self, asn, data, n_data, "g", CKA_BASE) ||
	    !parsed_asn1_attribute (self, asn, data, n_data, "priv", CKA_VALUE))
		goto done;
		
	parsed_fire (self);
	ret = SUCCESS;

done:
	if (asn)
		asn1_delete_structure (&asn);
	
	if (ret == GCR_ERROR_FAILURE) 
		g_message ("invalid DSA key");
		
	return ret;
}

static gint
parse_der_private_key_dsa_parts (GcrParser *self, const guchar *keydata, gsize n_keydata,
                                 const guchar *params, gsize n_params)
{
	gint ret = GCR_ERROR_UNRECOGNIZED;
	int res;
	ASN1_TYPE asn_params = ASN1_TYPE_EMPTY;
	ASN1_TYPE asn_key = ASN1_TYPE_EMPTY;

	asn_params = egg_asn1_decode ("PK.DSAParameters", params, n_params);
	asn_key = egg_asn1_decode ("PK.DSAPrivatePart", keydata, n_keydata);
	if (!asn_params || !asn_key)
		goto done;
	
	parsed_clear (self, CKO_PRIVATE_KEY);
	parsed_ulong (self, CKA_KEY_TYPE, CKK_DSA);
	res = GCR_ERROR_FAILURE;
    
	if (!parsed_asn1_attribute (self, asn_params, params, n_params, "p", CKA_PRIME) ||
	    !parsed_asn1_attribute (self, asn_params, params, n_params, "q", CKA_SUBPRIME) ||
	    !parsed_asn1_attribute (self, asn_params, params, n_params, "g", CKA_BASE) ||
	    !parsed_asn1_attribute (self, asn_key, keydata, n_keydata, "", CKA_VALUE))
		goto done;

	parsed_fire (self);
	ret = SUCCESS;
	
done:
	if (asn_key)
		asn1_delete_structure (&asn_key);
	if (asn_params)
		asn1_delete_structure (&asn_params);
	
	if (ret == GCR_ERROR_FAILURE) 
		g_message ("invalid DSA key");
		
	return ret;	
}

/* -----------------------------------------------------------------------------
 * PRIVATE KEY
 */

static gint
parse_der_private_key (GcrParser *self, const guchar *data, gsize n_data)
{
	gint res;
	
	res = parse_der_private_key_rsa (self, data, n_data);
	if (res == GCR_ERROR_UNRECOGNIZED)
		res = parse_der_private_key_dsa (self, data, n_data);
		
	return res;
}

/* -----------------------------------------------------------------------------
 * PKCS8 
 */

static gint
parse_der_pkcs8_plain (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gint ret;
	CK_KEY_TYPE key_type;
	GQuark key_algo;
	const guchar *keydata;
	gsize n_keydata;
	const guchar *params;
	gsize n_params;
	
	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-8-PrivateKeyInfo", data, n_data);
	if (!asn)
		goto done;

	ret = GCR_ERROR_FAILURE;
	key_type = GP11_INVALID;
		
	key_algo = egg_asn1_read_oid (asn, "privateKeyAlgorithm.algorithm");
  	if (!key_algo)
  		goto done;
  	else if (key_algo == OID_PKIX1_RSA)
  		key_type = CKK_RSA;
  	else if (key_algo == OID_PKIX1_DSA)
  		key_type = CKK_DSA;
  		
  	if (key_type == GP11_INVALID) {
  		ret = GCR_ERROR_UNRECOGNIZED;
  		goto done;
  	}

	keydata = egg_asn1_read_content (asn, data, n_data, "privateKey", &n_keydata);
	if (!keydata)
		goto done;
		
	params = egg_asn1_read_element (asn, data, n_data, "privateKeyAlgorithm.parameters", 
	                                     &n_params);
		
	ret = SUCCESS;
	
done:
	if (ret == SUCCESS) {		
		switch (key_type) {
		case CKK_RSA:
			ret = parse_der_private_key_rsa (self, keydata, n_keydata);
			break;
		case CKK_DSA:
			/* Try the normal sane format */
			ret = parse_der_private_key_dsa (self, keydata, n_keydata);
			
			/* Otherwise try the two part format that everyone seems to like */
			if (ret == GCR_ERROR_UNRECOGNIZED && params && n_params)
				ret = parse_der_private_key_dsa_parts (self, keydata, n_keydata, 
				                                       params, n_params);
			break;
		default:
			g_message ("invalid or unsupported key type in PKCS#8 key");
			ret = GCR_ERROR_UNRECOGNIZED;
			break;
		};
		
	} else if (ret == GCR_ERROR_FAILURE) {
		g_message ("invalid PKCS#8 key");
	}
	
	if (asn)
		asn1_delete_structure (&asn);
	return ret;
}

static gint
parse_der_pkcs8_encrypted (GcrParser *self, const guchar *data, gsize n_data)
{
	PasswordState pstate = PASSWORD_STATE_INIT;
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gcry_cipher_hd_t cih = NULL;
	gcry_error_t gcry;
	gint ret, r;
	GQuark scheme;
	guchar *crypted = NULL;
	const guchar *params;
	gsize n_crypted, n_params;
	const gchar *password;
	gint l;

	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-8-EncryptedPrivateKeyInfo", data, n_data);
	if (!asn)
		goto done;

	ret = GCR_ERROR_FAILURE;

	/* Figure out the type of encryption */
	scheme = egg_asn1_read_oid (asn, "encryptionAlgorithm.algorithm");
	if (!scheme)
		goto done;
		
	params = egg_asn1_read_element (asn, data, n_data, "encryptionAlgorithm.parameters", &n_params);
	
	parsed_clear (self, CKO_PRIVATE_KEY);

	/* Loop to try different passwords */                       
	for (;;) {
		
		g_assert (cih == NULL);
		
		r = enum_next_password (self, &pstate, &password);
		if (r != SUCCESS) {
			ret = r;
			break;
		}
	        
		/* Parse the encryption stuff into a cipher. */
		if (!egg_symkey_read_cipher (scheme, password, -1, params, n_params, &cih))
			break;
			
		crypted = egg_asn1_read_value (asn, "encryptedData", &n_crypted, (EggAllocator)egg_secure_realloc);
		if (!crypted)
			break;
	
		gcry = gcry_cipher_decrypt (cih, crypted, n_crypted, NULL, 0);
		gcry_cipher_close (cih);
		cih = NULL;
		
		if (gcry != 0) {
			g_warning ("couldn't decrypt pkcs8 data: %s", gcry_strerror (gcry));
			break;
		}
		
		/* Unpad the DER data */
		l = egg_asn1_element_length (crypted, n_crypted);
		if (l > 0)
			n_crypted = l;
		
		/* Try to parse the resulting key */
		r = parse_der_pkcs8_plain (self, crypted, n_crypted);
		egg_secure_free (crypted);
		crypted = NULL;
		
		if (r != GCR_ERROR_UNRECOGNIZED) {
			ret = r;
			break;
		}
		
		/* We assume unrecognized data, is a bad encryption key */	
	}
		
done:
	if (cih)
		gcry_cipher_close (cih);
	if (asn)
		asn1_delete_structure (&asn);
	egg_secure_free (crypted);
		
	return ret;
}

static gint
parse_der_pkcs8 (GcrParser *self, const guchar *data, gsize n_data)
{
	gint ret;
	
	ret = parse_der_pkcs8_plain (self, data, n_data);
	if (ret == GCR_ERROR_UNRECOGNIZED)
		ret = parse_der_pkcs8_encrypted (self, data, n_data);
	
	return ret;
}

/* -----------------------------------------------------------------------------
 * CERTIFICATE
 */

static gint
parse_der_certificate (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn;
	gchar *name;
	
	asn = egg_asn1_decode ("PKIX1.Certificate", data, n_data);
	if (asn == NULL)
		return GCR_ERROR_UNRECOGNIZED;

	parsed_clear (self, CKO_CERTIFICATE);
	parsed_ulong (self, CKA_CERTIFICATE_TYPE, CKC_X_509);

	name = egg_asn1_read_dn_part (asn, "tbsCertificate.subject.rdnSequence", "CN");
	asn1_delete_structure (&asn);
		
	if (name != NULL) {
		parsed_label (self, name);
		g_free (name);
	}

	parsed_attribute (self, CKA_VALUE, data, n_data);
	parsed_fire (self);
	
	return SUCCESS;
}

/* -----------------------------------------------------------------------------
 * PKCS7
 */

static gint
handle_pkcs7_signed_data (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gint ret;
	gchar *part;
	const guchar *certificate;
	gsize n_certificate;
	int i;
	
	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-7-SignedData", data, n_data);
	if (!asn)
		goto done;

	ret = GCR_ERROR_FAILURE;
	
	for (i = 0; TRUE; ++i) {
			
		part = g_strdup_printf ("certificates.?%u", i + 1);
		certificate = egg_asn1_read_element (asn, data, n_data, part, &n_certificate);
		g_free (part);
		
		/* No more certificates? */
		if (!certificate)
			break;
	
		ret = parse_der_certificate (self, certificate, n_certificate);
		if (ret != SUCCESS)
			goto done;
	}
	
	/* TODO: Parse out all the CRLs */
	
	ret = SUCCESS;
	
done:
	if (asn)
		asn1_delete_structure (&asn);
	
	return ret;
}

static gint
parse_der_pkcs7 (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gint ret;
	const guchar* content = NULL;
	gsize n_content;
	GQuark oid;
	
	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-7-ContentInfo", data, n_data);
	if (!asn)
		goto done;

	ret = GCR_ERROR_FAILURE;

	oid = egg_asn1_read_oid (asn, "contentType");
	if (!oid)
		goto done;

	/* Outer most one must just be plain data */
	if (oid != OID_PKCS7_SIGNED_DATA) {
		g_message ("unsupported outer content type in pkcs7: %s", g_quark_to_string (oid));
		goto done;
	}
	
	content = egg_asn1_read_content (asn, data, n_data, "content", &n_content);
	if (!content) 
		goto done;
		
	ret = handle_pkcs7_signed_data (self, content, n_content);
			
done:
	if (asn)
		asn1_delete_structure (&asn);
	return ret;
}

/* -----------------------------------------------------------------------------
 * PKCS12
 */

static gint
handle_pkcs12_cert_bag (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	const guchar *certificate;
	gsize n_certificate;
	gint ret;

	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-12-CertBag", data, n_data);
	if (!asn)
		goto done;
		
	ret = GCR_ERROR_FAILURE;
	
	certificate = egg_asn1_read_content (asn, data, n_data, "certValue", &n_certificate);
	if (!certificate)
		goto done;

	/* 
	 * Wrapped in an OCTET STRING, so unwrap here, rather than allocating 
	 * a whole bunch more memory for a full ASN.1 parsing context.
	 */ 
	certificate = egg_asn1_element_content (certificate, n_certificate, &n_certificate);
	if (!certificate)
		goto done;
	
	ret = parse_der_certificate (self, certificate, n_certificate);
		
done:
	if (asn)
		asn1_delete_structure (&asn);
		
	return ret;
}

static gint
handle_pkcs12_bag (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gint ret, r;
	int res, count = 0;
	GQuark oid;
	const guchar *element;
	gsize n_element;
	
	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-12-SafeContents", data, n_data);
	if (!asn)
		goto done;
		
	ret = GCR_ERROR_FAILURE;
	
	/* Get the number of elements in this bag */
	res = asn1_number_of_elements (asn, "", &count);
	if (res != ASN1_SUCCESS)
		goto done;	
	
	/* 
	 * Now inside each bag are multiple elements. Who comes up 
	 * with this stuff?
	 * 
	 * But this is where we draw the line. We only support one
	 * element per bag, not multiple elements, not strange
	 * nested bags, not fairy queens with magical wands in bags...
	 * 
	 * Just one element per bag.
	 */
	if (count >= 1) {

		oid = egg_asn1_read_oid (asn, "?1.bagId");
		if (!oid)
			goto done;
		
		element = egg_asn1_read_content (asn, data, n_data, "?1.bagValue", &n_element); 	
		if (!element)
			goto done;

		/* A normal unencrypted key */
		if (oid == OID_PKCS12_BAG_PKCS8_KEY) {
			r = parse_der_pkcs8_plain (self, element, n_element);
			
		/* A properly encrypted key */
		} else if (oid == OID_PKCS12_BAG_PKCS8_ENCRYPTED_KEY) {
			r = parse_der_pkcs8_encrypted (self, element, n_element);
			
		/* A certificate */
		} else if (oid == OID_PKCS12_BAG_CERTIFICATE) {
			r = handle_pkcs12_cert_bag (self, element, n_element);
								
		/* TODO: OID_PKCS12_BAG_CRL */
		} else {
			r = GCR_ERROR_UNRECOGNIZED;
		}
		 
		if (r == GCR_ERROR_FAILURE || r == GCR_ERROR_CANCELLED) {
			ret = r;
			goto done;
		}
	}

	ret = SUCCESS;	
		
done:
	if (asn)
		asn1_delete_structure (&asn);
		
	return ret;
}

static gint
handle_pkcs12_encrypted_bag (GcrParser *self, const guchar *data, gsize n_data)
{
	PasswordState pstate = PASSWORD_STATE_INIT;
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gcry_cipher_hd_t cih = NULL;
	gcry_error_t gcry;
	guchar *crypted = NULL;
	const guchar *params;
	gsize n_params, n_crypted;
	const gchar *password;
	GQuark scheme;
	gint ret, r;
	gint l;
	
	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-7-EncryptedData", data, n_data);
	if (!asn)
		goto done;
	
	ret = GCR_ERROR_FAILURE;
		
	/* Check the encryption schema OID */
	scheme = egg_asn1_read_oid (asn, "encryptedContentInfo.contentEncryptionAlgorithm.algorithm");
	if (!scheme) 
		goto done;	

	params = egg_asn1_read_element (asn, data, n_data, "encryptedContentInfo.contentEncryptionAlgorithm.parameters", &n_params);
	if (!params)
		goto done;
	
	parsed_clear (self, 0);

	/* Loop to try different passwords */
	for (;;) {
		
		g_assert (cih == NULL);
		
		r = enum_next_password (self, &pstate, &password);
		if (r != SUCCESS) {
			ret = r;
			goto done;
		}
	        
		/* Parse the encryption stuff into a cipher. */
		if (!egg_symkey_read_cipher (scheme, password, -1, params, n_params, &cih)) {
			ret = GCR_ERROR_FAILURE;
			goto done;
		}
			
		crypted = egg_asn1_read_value (asn, "encryptedContentInfo.encryptedContent", 
		                               &n_crypted, (EggAllocator)egg_secure_realloc);
		if (!crypted)
			goto done;
	
		gcry = gcry_cipher_decrypt (cih, crypted, n_crypted, NULL, 0);
		gcry_cipher_close (cih);
		cih = NULL;
		
		if (gcry != 0) {
			g_warning ("couldn't decrypt pkcs7 data: %s", gcry_strerror (gcry));
			goto done;
		}
		
		/* Unpad the DER data */
		l = egg_asn1_element_length (crypted, n_crypted);
		if (l > 0)
			n_crypted = l;

		/* Try to parse the resulting key */
		r = handle_pkcs12_bag (self, crypted, n_crypted);
		egg_secure_free (crypted);
		crypted = NULL;
		
		if (r != GCR_ERROR_UNRECOGNIZED) {
			ret = r;
			break;
		}
		
		/* We assume unrecognized data is a bad encryption key */	
	}
		
done:
	if (cih)
		gcry_cipher_close (cih);
	if (asn)
		asn1_delete_structure (&asn);
	egg_secure_free (crypted);
	
	return ret;
}

static gint
handle_pkcs12_safe (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gint ret, r;
	const guchar *bag;
	gsize n_bag;
	gchar *part;
	GQuark oid;
	guint i;
	
	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-12-AuthenticatedSafe", data, n_data);
	if (!asn)
		goto done;
		
	ret = GCR_ERROR_FAILURE;
	
	/*
	 * Inside each PKCS12 safe there are multiple bags. 
	 */
	for (i = 0; TRUE; ++i) {
		
		part = g_strdup_printf ("?%u.contentType", i + 1);
		oid = egg_asn1_read_oid (asn, part);
		g_free (part);
		
		/* All done? no more bags */
		if (!oid) 
			break;
		
		part = g_strdup_printf ("?%u.content", i + 1);
		bag = egg_asn1_read_content (asn, data, n_data, part, &n_bag);
		g_free (part);
		
		if (!bag) /* A parse error */
			goto done;
			
		/* A non encrypted bag, just parse */
		if (oid == OID_PKCS7_DATA) {
			
			/* 
		 	 * Wrapped in an OCTET STRING, so unwrap here, rather than allocating 
		 	 * a whole bunch more memory for a full ASN.1 parsing context.
		 	 */ 
			bag = egg_asn1_element_content (bag, n_bag, &n_bag);
			if (!bag)
				goto done;	
			
			r = handle_pkcs12_bag (self, bag, n_bag);

		/* Encrypted data first needs decryption */
		} else if (oid == OID_PKCS7_ENCRYPTED_DATA) {
			r = handle_pkcs12_encrypted_bag (self, bag, n_bag);
		
		/* Hmmmm, not sure what this is */
		} else {
			g_warning ("unrecognized type of safe content in pkcs12: %s", g_quark_to_string (oid));
			r = GCR_ERROR_UNRECOGNIZED;
		}
		
		if (r == GCR_ERROR_FAILURE || r == GCR_ERROR_CANCELLED) {
			ret = r;
			goto done;
		}
	}
	
	ret = SUCCESS;
	
done:
	if (asn)
		asn1_delete_structure (&asn);
		
	return ret;
}

static gint
parse_der_pkcs12 (GcrParser *self, const guchar *data, gsize n_data)
{
	ASN1_TYPE asn = ASN1_TYPE_EMPTY;
	gint ret;
	const guchar* content = NULL;
	gsize n_content;
	GQuark oid;
	
	ret = GCR_ERROR_UNRECOGNIZED;
	
	asn = egg_asn1_decode ("PKIX1.pkcs-12-PFX", data, n_data);
	if (!asn)
		goto done;

	oid = egg_asn1_read_oid (asn, "authSafe.contentType");
	if (!oid)
		goto done;
		
	/* Outer most one must just be plain data */
	if (oid != OID_PKCS7_DATA) {
		g_message ("unsupported safe content type in pkcs12: %s", g_quark_to_string (oid));
		goto done;
	}
	
	content = egg_asn1_read_content (asn, data, n_data, "authSafe.content", &n_content);
	if (!content)
		goto done;
		
	/* 
	 * Wrapped in an OCTET STRING, so unwrap here, rather than allocating 
	 * a whole bunch more memory for a full ASN.1 parsing context.
	 */ 
	content = egg_asn1_element_content (content, n_content, &n_content);
	if (!content)
		goto done;
				
	ret = handle_pkcs12_safe (self, content, n_content);
	
done:
	if (asn)
		asn1_delete_structure (&asn);
	return ret;
}

/* -----------------------------------------------------------------------------
 * PEM PARSING 
 */

static gint
handle_plain_pem (GcrParser *self, GQuark type, gint subformat, 
                  const guchar *data, gsize n_data)
{
	ParserFormat *format;
	gint format_id;
	
	if (type == PEM_RSA_PRIVATE_KEY)
		format_id = GCR_FORMAT_DER_PRIVATE_KEY_RSA;

	else if (type == PEM_DSA_PRIVATE_KEY)
		format_id = GCR_FORMAT_DER_PRIVATE_KEY_DSA;

	else if (type == PEM_ANY_PRIVATE_KEY)
		format_id = GCR_FORMAT_DER_PRIVATE_KEY;
	
	else if (type == PEM_PRIVATE_KEY)
		format_id = GCR_FORMAT_DER_PKCS8_PLAIN;
		
	else if (type == PEM_ENCRYPTED_PRIVATE_KEY)
		format_id = GCR_FORMAT_DER_PKCS8_ENCRYPTED;

	else if (type == PEM_CERTIFICATE)
		format_id = GCR_FORMAT_DER_CERTIFICATE_X509;

	else if (type == PEM_PKCS7)
		format_id = GCR_FORMAT_DER_PKCS7;
		
	else if (type == PEM_PKCS12)
		format_id = GCR_FORMAT_DER_PKCS12;
		
	else
		return GCR_ERROR_UNRECOGNIZED;

	if (subformat != 0 && subformat != format_id)
		return GCR_ERROR_UNRECOGNIZED;
	
	format = parser_format_lookup (format_id);
	if (format == NULL)
		return GCR_ERROR_UNRECOGNIZED;
	
	return (format->function) (self, data, n_data);
}

static CK_OBJECT_CLASS
pem_type_to_class (gint type)
{
	if (type == PEM_RSA_PRIVATE_KEY ||
	    type == PEM_DSA_PRIVATE_KEY ||
	    type == PEM_ANY_PRIVATE_KEY ||
	    type == PEM_PRIVATE_KEY ||
	    type == PEM_ENCRYPTED_PRIVATE_KEY)
		return CKO_PRIVATE_KEY;

	else if (type == PEM_CERTIFICATE)
		return CKO_CERTIFICATE;
		
	else if (type == PEM_PKCS7 ||
	         type == PEM_PKCS12)
		return 0;

	return 0;
}

static gint
handle_encrypted_pem (GcrParser *self, GQuark type, gint subformat, 
                      GHashTable *headers, const guchar *data, gsize n_data)
{
	PasswordState pstate = PASSWORD_STATE_INIT;
	const gchar *password;
	guchar *decrypted;
	gsize n_decrypted;
	const gchar *val;
	gboolean ret;
	gint res;
	gint l;
	
	g_assert (GCR_IS_PARSER (self));
	g_assert (headers);
	g_assert (type);
	
	val = g_hash_table_lookup (headers, "DEK-Info");
	if (!val) {
		g_message ("missing encryption header");
		return GCR_ERROR_FAILURE;
	}
	
	/* Fill in information necessary for prompting */
	parsed_clear (self, pem_type_to_class (type));
	
	for (;;) {

		res = enum_next_password (self, &pstate, &password);
		if (res != SUCCESS)
			return res;
		
		decrypted = NULL;
		n_decrypted = 0;
		
		/* Decrypt, this will result in garble if invalid password */	
		ret = egg_openssl_decrypt_block (val, password, -1, data, n_data, 
		                                 &decrypted, &n_decrypted);
		if (!ret)
			return GCR_ERROR_FAILURE;
			
		g_assert (decrypted);
		
		/* Unpad the DER data */
		l = egg_asn1_element_length (decrypted, n_decrypted);
		if (l > 0)
			n_decrypted = l;
	
		/* Try to parse */
		res = handle_plain_pem (self, type, subformat, decrypted, n_decrypted);
		egg_secure_free (decrypted);

		/* Unrecognized is a bad password */
		if (res != GCR_ERROR_UNRECOGNIZED)
			return res;		
	}
	
	return GCR_ERROR_FAILURE;
}

typedef struct {
	GcrParser *parser;
	gint result;
	gint subformat;
} HandlePemArgs;

static void
handle_pem_data (GQuark type, const guchar *data, gsize n_data,
                 GHashTable *headers, gpointer user_data)
{
	HandlePemArgs *args = (HandlePemArgs*)user_data;
	gint res = GCR_ERROR_FAILURE;
	gboolean encrypted = FALSE;
	const gchar *val;
	
	/* Something already failed to parse */
	if (args->result == GCR_ERROR_FAILURE)
		return;
	
	/* See if it's encrypted PEM all openssl like*/
	if (headers) {
		val = g_hash_table_lookup (headers, "Proc-Type");
		if (val && strcmp (val, "4,ENCRYPTED") == 0) 
			encrypted = TRUE;
	}
	
	if (encrypted)
		res = handle_encrypted_pem (args->parser, type, args->subformat,  
		                            headers, data, n_data); 
	else
		res = handle_plain_pem (args->parser, type, args->subformat, 
		                        data, n_data);
	
	if (res != GCR_ERROR_UNRECOGNIZED) {
		if (args->result == GCR_ERROR_UNRECOGNIZED)
			args->result = res;
		else if (res > args->result)
			args->result = res;
	}
}

static gint
handle_pem_format (GcrParser *self, gint subformat, const guchar *data, gsize n_data)
{
	HandlePemArgs ctx = { self, GCR_ERROR_UNRECOGNIZED, subformat };
	guint found;
	
	if (n_data == 0)
		return GCR_ERROR_UNRECOGNIZED;
	
	found = egg_openssl_pem_parse (data, n_data, handle_pem_data, &ctx);
	
	if (found == 0)
		return GCR_ERROR_UNRECOGNIZED;
		
	return ctx.result;
}


static gint
parse_pem (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, 0, data, n_data);
}

static gint 
parse_pem_private_key_rsa (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, GCR_FORMAT_DER_PRIVATE_KEY_RSA, data, n_data);
}

static gint 
parse_pem_private_key_dsa (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, GCR_FORMAT_DER_PRIVATE_KEY_DSA, data, n_data);
}

static gint 
parse_pem_certificate (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, GCR_FORMAT_PEM_CERTIFICATE_X509, data, n_data);
}

static gint 
parse_pem_pkcs8_plain (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, GCR_FORMAT_PEM_PKCS8_PLAIN, data, n_data);
}

static gint 
parse_pem_pkcs8_encrypted (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, GCR_FORMAT_PEM_PKCS8_ENCRYPTED, data, n_data);
}

static gint 
parse_pem_pkcs7 (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, GCR_FORMAT_PEM_PKCS7, data, n_data);
}

static gint 
parse_pem_pkcs12 (GcrParser *self, const guchar *data, gsize n_data)
{
	return handle_pem_format (self, GCR_FORMAT_PEM_PKCS12, data, n_data);
}

/* -----------------------------------------------------------------------------
 * FORMATS
 */

/* In order of parsing when no formats specified */
static const ParserFormat parser_normal[] = {
	{ GCR_FORMAT_PEM, parse_pem },
	{ GCR_FORMAT_DER_PRIVATE_KEY_RSA, parse_der_private_key_rsa },
	{ GCR_FORMAT_DER_PRIVATE_KEY_DSA, parse_der_private_key_dsa },
	{ GCR_FORMAT_DER_CERTIFICATE_X509, parse_der_certificate },
	{ GCR_FORMAT_DER_PKCS7, parse_der_pkcs7 },
	{ GCR_FORMAT_DER_PKCS8_PLAIN, parse_der_pkcs8_plain },
	{ GCR_FORMAT_DER_PKCS8_ENCRYPTED, parse_der_pkcs8_encrypted },
	{ GCR_FORMAT_DER_PKCS12, parse_der_pkcs12 }
};

/* Must be in format_id numeric order */
static const ParserFormat parser_formats[] = {
	{ GCR_FORMAT_DER_PRIVATE_KEY, parse_der_private_key },
	{ GCR_FORMAT_DER_PRIVATE_KEY_RSA, parse_der_private_key_rsa },
	{ GCR_FORMAT_DER_PRIVATE_KEY_DSA, parse_der_private_key_dsa },
	{ GCR_FORMAT_DER_CERTIFICATE_X509, parse_der_certificate },
	{ GCR_FORMAT_DER_PKCS7, parse_der_pkcs7 },
	{ GCR_FORMAT_DER_PKCS8, parse_der_pkcs8 },
	{ GCR_FORMAT_DER_PKCS8_PLAIN, parse_der_pkcs8_plain },
	{ GCR_FORMAT_DER_PKCS8_ENCRYPTED, parse_der_pkcs8_encrypted },
	{ GCR_FORMAT_DER_PKCS12, parse_der_pkcs12 },
	{ GCR_FORMAT_PEM, parse_pem },
	{ GCR_FORMAT_PEM_PRIVATE_KEY_RSA, parse_pem_private_key_rsa },
	{ GCR_FORMAT_PEM_PRIVATE_KEY_DSA, parse_pem_private_key_dsa },
	{ GCR_FORMAT_PEM_CERTIFICATE_X509, parse_pem_certificate },
	{ GCR_FORMAT_PEM_PKCS7, parse_pem_pkcs7 },
	{ GCR_FORMAT_PEM_PKCS8_PLAIN, parse_pem_pkcs8_plain },
	{ GCR_FORMAT_PEM_PKCS8_ENCRYPTED, parse_pem_pkcs8_encrypted },
	{ GCR_FORMAT_PEM_PKCS12, parse_pem_pkcs12 },
};

static int
compar_id_to_parser_format (const void *a, const void *b)
{
	const gint *format_id = a;
	const ParserFormat *format = b;
	
	g_assert (format_id);
	g_assert (format);
	
	if (format->format_id == *format_id)
		return 0;
	return (*format_id < format->format_id) ? -1 : 1;
}

static ParserFormat*
parser_format_lookup (gint format_id)
{
	return bsearch (&format_id, parser_formats, G_N_ELEMENTS (parser_formats),
	                sizeof (parser_formats[0]), compar_id_to_parser_format);
}

static gint
compare_pointers (gconstpointer a, gconstpointer b)
{
	if (a == b)
		return 0;
	return a < b ? -1 : 1;
}

typedef struct _ForeachArgs {
	GcrParser *parser;
	const guchar *data;
	gsize n_data;
	gint result;
} ForeachArgs;

static gboolean
parser_format_foreach (gpointer key, gpointer value, gpointer data)
{
	ForeachArgs *args = data;
	ParserFormat *format = key;
	gint result;
	
	g_assert (format);
	g_assert (format->function);
	g_assert (GCR_IS_PARSER (args->parser));
	
	result = (format->function) (args->parser, args->data, args->n_data);
	if (result != GCR_ERROR_UNRECOGNIZED) {
		args->result = result;
		return TRUE;
	}
	
	/* Keep going */
	return FALSE;
}

/* -----------------------------------------------------------------------------
 * OBJECT 
 */


static GObject* 
gcr_parser_constructor (GType type, guint n_props, GObjectConstructParam *props) 
{
	GcrParser *self = GCR_PARSER (G_OBJECT_CLASS (gcr_parser_parent_class)->constructor(type, n_props, props));
	g_return_val_if_fail (self, NULL);	

	/* Always try to parse with NULL and empty passwords first */
	gcr_parser_add_password (self, NULL);
	gcr_parser_add_password (self, "");
	
	return G_OBJECT (self);
}

static void
gcr_parser_init (GcrParser *self)
{
	self->pv = G_TYPE_INSTANCE_GET_PRIVATE (self, GCR_TYPE_PARSER, GcrParserPrivate);
	self->pv->passwords = g_ptr_array_new ();
	self->pv->normal_formats = TRUE;
}

static void
gcr_parser_dispose (GObject *obj)
{
	GcrParser *self = GCR_PARSER (obj);
	gsize i;
	
	if (self->pv->parsed_attrs)
		gp11_attributes_unref (self->pv->parsed_attrs);
	self->pv->parsed_attrs = NULL;
	
	g_free (self->pv->parsed_label);
	self->pv->parsed_label = NULL;
	
	for (i = 0; i < self->pv->passwords->len; ++i)
		egg_secure_strfree (g_ptr_array_index (self->pv->passwords, i));
	g_ptr_array_set_size (self->pv->passwords, 0);
	
	G_OBJECT_CLASS (gcr_parser_parent_class)->dispose (obj);
}

static void
gcr_parser_finalize (GObject *obj)
{
	GcrParser *self = GCR_PARSER (obj);
	
	g_assert (!self->pv->parsed_attrs);
	g_assert (!self->pv->parsed_label);

	g_ptr_array_free (self->pv->passwords, TRUE);
	self->pv->passwords = NULL;

	G_OBJECT_CLASS (gcr_parser_parent_class)->finalize (obj);
}

static void
gcr_parser_set_property (GObject *obj, guint prop_id, const GValue *value, 
                           GParamSpec *pspec)
{
#if 0
	GcrParser *self = GCR_PARSER (obj);
#endif
	switch (prop_id) {
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
		break;
	}
}

static void
gcr_parser_get_property (GObject *obj, guint prop_id, GValue *value, 
                         GParamSpec *pspec)
{
	GcrParser *self = GCR_PARSER (obj);
	
	switch (prop_id) {
	case PROP_PARSED_ATTRIBUTES:
		g_value_set_boxed (value, gcr_parser_get_parsed_attributes (self));
		break;
	case PROP_PARSED_LABEL:
		g_value_set_string (value, gcr_parser_get_parsed_label (self));
		break;
	case PROP_PARSED_DESCRIPTION:
		g_value_set_string (value, gcr_parser_get_parsed_description (self));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (obj, prop_id, pspec);
		break;
	}
}

static void
gcr_parser_class_init (GcrParserClass *klass)
{
	GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
	gint i;
    
	gobject_class->constructor = gcr_parser_constructor;
	gobject_class->dispose = gcr_parser_dispose;
	gobject_class->finalize = gcr_parser_finalize;
	gobject_class->set_property = gcr_parser_set_property;
	gobject_class->get_property = gcr_parser_get_property;
    
	g_type_class_add_private (gobject_class, sizeof (GcrParserPrivate));
	
	g_object_class_install_property (gobject_class, PROP_PARSED_ATTRIBUTES,
	           g_param_spec_boxed ("parsed-attributes", "Parsed Attributes", "Parsed PKCS#11 attributes", 
	                               GP11_TYPE_ATTRIBUTES, G_PARAM_READABLE));

	g_object_class_install_property (gobject_class, PROP_PARSED_LABEL,
	           g_param_spec_string ("parsed-label", "Parsed Label", "Parsed item label", 
	                                "", G_PARAM_READABLE));

	g_object_class_install_property (gobject_class, PROP_PARSED_DESCRIPTION,
	           g_param_spec_string ("parsed-description", "Parsed Description", "Parsed item description", 
	                                "", G_PARAM_READABLE));
    
	signals[AUTHENTICATE] = g_signal_new ("authenticate", GCR_TYPE_PARSER, 
	                                G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (GcrParserClass, authenticate),
	                                g_signal_accumulator_true_handled, NULL, _gcr_marshal_BOOLEAN__INT, 
	                                G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);

	signals[PARSED] = g_signal_new ("parsed", GCR_TYPE_PARSER, 
	                                G_SIGNAL_RUN_FIRST, G_STRUCT_OFFSET (GcrParserClass, parsed),
	                                NULL, NULL, g_cclosure_marshal_VOID__VOID, 
	                                G_TYPE_NONE, 0);
	
	init_quarks ();
	_gcr_initialize ();
	
	/* Check that the format tables are in order */
	for (i = 1; i < G_N_ELEMENTS (parser_formats); ++i)
		g_assert (parser_formats[i].format_id >= parser_formats[i - 1].format_id);
}

/* -----------------------------------------------------------------------------
 * PUBLIC 
 */

GcrParser*
gcr_parser_new (void)
{
	return g_object_new (GCR_TYPE_PARSER, NULL);
}

void
gcr_parser_add_password (GcrParser *self, const gchar *password)
{
	g_return_if_fail (GCR_IS_PARSER (self));
	g_ptr_array_add (self->pv->passwords, egg_secure_strdup (password));
}

gboolean
gcr_parser_parse_data (GcrParser *self, const guchar *data, 
                       gsize n_data, GError **err)
{
	ForeachArgs args = { self, data, n_data, GCR_ERROR_UNRECOGNIZED };
	const gchar *message;
	gint i;
	
	g_return_val_if_fail (GCR_IS_PARSER (self), FALSE);
	g_return_val_if_fail (data || !n_data, FALSE);
	g_return_val_if_fail (!err || !*err, FALSE);

	/* Just the specific formats requested */
	if (self->pv->specific_formats) { 
		g_tree_foreach (self->pv->specific_formats, parser_format_foreach, &args);
		
	/* All the 'normal' formats */
	} else if (self->pv->normal_formats) {
		for (i = 0; i < G_N_ELEMENTS (parser_normal); ++i) {
			if (parser_format_foreach ((gpointer)(parser_normal + i), 
			                           (gpointer)(parser_normal + i), &args))
				break;
		}
	}
	
	switch (args.result) {
	case SUCCESS:
		return TRUE;
	case GCR_ERROR_CANCELLED:
		message = _("The operation was cancelled");
		break;
	case GCR_ERROR_UNRECOGNIZED:
		message = _("Unrecognized or unsupported data.");
		break;
	case GCR_ERROR_FAILURE:
		message = _("Could not parse invalid or corrupted data.");
		break;
	case GCR_ERROR_LOCKED:
		message = _("The data is locked");
		break;
	default:
		g_assert_not_reached ();
		break;
	};
	
	g_set_error_literal (err, GCR_DATA_ERROR, args.result, message);
	return FALSE;
}

gboolean
gcr_parser_format_enable (GcrParser *self, gint format_id)
{
	ParserFormat *format;
	
	g_return_val_if_fail (GCR_IS_PARSER (self), FALSE);
	
	if (format_id == -1) {
		if (self->pv->specific_formats)
			g_tree_destroy (self->pv->specific_formats);
		self->pv->specific_formats = NULL;
		self->pv->normal_formats = TRUE;
		return TRUE;
	}
	
	format = parser_format_lookup (format_id);
	if (format == NULL)
		return FALSE;
	
	if (!self->pv->specific_formats) {
		if (self->pv->normal_formats)
			return TRUE;
		self->pv->specific_formats = g_tree_new (compare_pointers);
	}
	
	g_tree_insert (self->pv->specific_formats, format, format);
	return TRUE;
}

gboolean
gcr_parser_format_disable (GcrParser *self, gint format_id)
{
	ParserFormat *format;
	
	g_return_val_if_fail (GCR_IS_PARSER (self), FALSE);
	
	if (format_id == -1) {
		if (self->pv->specific_formats)
			g_tree_destroy (self->pv->specific_formats);
		self->pv->specific_formats = NULL;
		self->pv->normal_formats = FALSE;
		return TRUE;
	}
	
	if (!self->pv->specific_formats)
		return TRUE;
	
	format = parser_format_lookup (format_id);
	if (format == NULL)
		return FALSE;
	
	g_tree_remove (self->pv->specific_formats, format);
	return TRUE;
}

gboolean
gcr_parser_format_supported (GcrParser *self, gint format_id)
{
	g_return_val_if_fail (GCR_IS_PARSER (self), FALSE);
	g_return_val_if_fail (format_id != -1, FALSE);
	return parser_format_lookup (format_id) ? TRUE : FALSE;	
}

const gchar*
gcr_parser_get_parsed_description (GcrParser *self)
{
	g_return_val_if_fail (GCR_IS_PARSER (self), NULL);
	return self->pv->parsed_desc;
}

GP11Attributes*
gcr_parser_get_parsed_attributes (GcrParser *self)
{
	g_return_val_if_fail (GCR_IS_PARSER (self), NULL);
	return self->pv->parsed_attrs;	
}

const gchar*
gcr_parser_get_parsed_label (GcrParser *self)
{
	g_return_val_if_fail (GCR_IS_PARSER (self), NULL);
	return self->pv->parsed_label;		
}
