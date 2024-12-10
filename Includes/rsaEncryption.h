//
// Created by Ugo WAREMBOURG on 03/12/2024.
//

#ifndef MESSAGERIEPROJECTAP3_RSAENCRYPTION_H
#define MESSAGERIEPROJECTAP3_RSAENCRYPTION_H

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>

#define RSA_KEY_BITS 2048

/**
 * \brief Encrypt plaintext using RSA.
 *
 * \param key Pointer to the RSA public key.
 * \param plaintext Pointer to the plaintext to be encrypted.
 * \param plaintext_len Length of the plaintext.
 * \param ciphertext Pointer to the buffer where the ciphertext will be stored.
 * \param ciphertext_len Pointer to the length of the ciphertext.
 */
void rsa_encrypt(EVP_PKEY *key, const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len);

/**
 * \brief Decrypt ciphertext using RSA.
 *
 * \param key Pointer to the RSA private key.
 * \param ciphertext Pointer to the ciphertext to be decrypted.
 * \param ciphertext_len Length of the ciphertext.
 * \param plaintext Pointer to the buffer where the decrypted plaintext will be stored.
 * \param plaintext_len Pointer to the length of the decrypted plaintext.
 */
void rsa_decrypt(EVP_PKEY *key, const unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, int *plaintext_len);

/**
 * \brief Read an RSA public key from a file.
 *
 * \param filename Path to the file containing the RSA public key.
 * \return Pointer to the EVP_PKEY structure containing the RSA public key.
 */
EVP_PKEY *rsa_read_public_key(const char *filename);

#endif //MESSAGERIEPROJECTAP3_RSAENCRYPTION_H
