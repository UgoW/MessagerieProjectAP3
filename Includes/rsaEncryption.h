//
// Created by Ugo WAREMBOURG on 03/12/2024.
//

#ifndef MESSAGERIEPROJECTAP3_RSAENCRYPTION_H
#define MESSAGERIEPROJECTAP3_RSAENCRYPTION_H

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define RSA_KEY_BITS 2048

RSA* generate_rsa_keys();
void rsa_encrypt(RSA *key, const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len);
void rsa_decrypt(RSA *key, const unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, int *plaintext_len);

#endif //MESSAGERIEPROJECTAP3_RSAENCRYPTION_H
