/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  TLS Certificate support
 * Author:   David Register
 *
 ***************************************************************************
 *   Copyright (C) 2022 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************
 *
 *
 *
 */

#include "config.h"

#include <cstdio>
#include <iostream>
#include <string.h>

#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>

#ifdef __MSVC__
#include "openssl/applink.c"
#endif

#ifdef HAVE_OCPN_LOG
#include "model/logger.h"
#define cerr ERROR_LOG
#endif

using namespace std;
/* Generates a 2048-bit RSA key. */
EVP_PKEY *generate_key() {
  /* Allocate memory for the EVP_PKEY structure. */
  EVP_PKEY *pkey = EVP_PKEY_new();
  if (!pkey) {
    cerr << "Unable to create EVP_PKEY structure." << endl;
    return NULL;
  }

  /* Generate the RSA key and assign it to pkey. */
  RSA *rsa = RSA_generate_key(2048, RSA_F4, NULL, NULL);
  if (!EVP_PKEY_assign_RSA(pkey, rsa)) {
    cerr << "Unable to generate 2048-bit RSA key." << endl;
    EVP_PKEY_free(pkey);
    return NULL;
  }

  /* The key has been generated, return it. */
  return pkey;
}

int cs_cert_set_subject_alt_name(X509 *x509_cert, string name) {
  const char *subject_alt_name = name.c_str();  //"IP: 192.168.1.1";
  X509_EXTENSION *extension_san = NULL;
  ASN1_OCTET_STRING *subject_alt_name_ASN1 = NULL;
  int ret = -1;

  subject_alt_name_ASN1 = ASN1_OCTET_STRING_new();
  if (!subject_alt_name_ASN1) {
    goto err;
  }
  ASN1_OCTET_STRING_set(subject_alt_name_ASN1,
                        (unsigned char *)subject_alt_name,
                        strlen(subject_alt_name));
  if (!X509_EXTENSION_create_by_NID(&extension_san, NID_subject_alt_name, 0,
                                    subject_alt_name_ASN1)) {
    goto err;
  }
  ASN1_OCTET_STRING_free(subject_alt_name_ASN1);
  ret = X509_add_ext(x509_cert, extension_san, -1);
  if (!ret) {
    goto err;
  }
  X509_EXTENSION_free(extension_san);
  return 0;

err:
  if (subject_alt_name_ASN1) ASN1_OCTET_STRING_free(subject_alt_name_ASN1);
  if (extension_san) X509_EXTENSION_free(extension_san);
  return -1;
}

/* Generates a self-signed x509 certificate. */
X509 *generate_x509(EVP_PKEY *pkey, string ip_v4) {
  /* Allocate memory for the X509 structure. */
  X509 *x509 = X509_new();
  if (!x509) {
    cerr << "Unable to create X509 structure." << endl;
    return NULL;
  }

  /* Set the serial number. */
  ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);

  /* This certificate is valid from now until exactly one year from now. */

  X509_gmtime_adj(X509_get_notBefore(x509), 0);
  X509_gmtime_adj(X509_get_notAfter(x509), 31536000L);

  /* Set the public key for our certificate. */
  X509_set_pubkey(x509, pkey);

  /* We want to copy the subject name to the issuer name. */
  X509_NAME *name = X509_get_subject_name(x509);

  /* Set the country code and common name. */
  X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char *)"CA", -1,
                             -1, 0);
  X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC,
                             (unsigned char *)"MyCompany", -1, -1, 0);
  X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC,
                             (unsigned char *)"localhost", -1, -1, 0);
  string ext_name("IP: ");
  ext_name += ip_v4;
  cs_cert_set_subject_alt_name(x509, ext_name);

  /* Now set the issuer name. */
  X509_set_issuer_name(x509, name);

  /* Actually sign the certificate with our key. */
  if (!X509_sign(x509, pkey, EVP_sha1())) {
    cerr << "Error signing certificate." << endl;
    X509_free(x509);
    return NULL;
  }

  return x509;
}

bool write_to_disk(EVP_PKEY *pkey, X509 *x509, string cert_directory) {
  /* Open the PEM file for writing the key to disk. */
  string key_file = cert_directory;
  key_file += "key.pem";
  FILE *pkey_file = fopen(key_file.c_str(), "wb");
  if (!pkey_file) {
    cerr << "Unable to open \"key.pem\" for writing." << endl;
    return false;
  }

  /* Write the key to disk. */
  bool ret = PEM_write_PrivateKey(pkey_file, pkey, NULL, NULL, 0, NULL, NULL);
  fclose(pkey_file);

  if (!ret) {
    cerr << "Unable to write private key to disk." << endl;
    return false;
  }

  /* Open the PEM file for writing the certificate to disk. */
  string cert_file = cert_directory;
  cert_file += "cert.pem";

  FILE *x509_file = fopen(cert_file.c_str(), "wb");
  if (!x509_file) {
    cerr << "Unable to open \"cert.pem\" for writing." << endl;
    return false;
  }

  /* Write the certificate to disk. */
  ret = PEM_write_X509(x509_file, x509);
  fclose(x509_file);

  if (!ret) {
    cerr << "Unable to write certificate to disk." << endl;
    return false;
  }

  return true;
}

int make_certificate(string ipv4, string destination_dir) {
  /* Generate the key. */
  if (getenv("OCPN_DEBUG_CERT")) cout << "Generating RSA key..." << endl;

  EVP_PKEY *pkey = generate_key();
  if (!pkey) return 1;

  /* Generate the certificate. */
  if (getenv("OCPN_DEBUG_CERT"))
    cout << "Generating x509 certificate..." << endl;

  X509 *x509 = generate_x509(pkey, ipv4);
  if (!x509) {
    EVP_PKEY_free(pkey);
    return 1;
  }

  /* Write the private key and certificate out to disk. */
  if (getenv("OCPN_DEBUG_CERT"))
    cout << "Writing key and certificate to disk..." << endl;

  bool ret = write_to_disk(pkey, x509, destination_dir);
  EVP_PKEY_free(pkey);
  X509_free(x509);

  if (ret) {
    if (getenv("OCPN_DEBUG_CERT")) cout << "Success!" << endl;
    return 0;
  } else
    return 1;
}
