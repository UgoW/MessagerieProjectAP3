//
// Created by Ugo WAREMBOURG on 03/12/2024.
//

#ifndef MESSAGERIEPROJECTAP3_AESENCRYPTION_H
#define MESSAGERIEPROJECTAP3_AESENCRYPTION_H

#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define AES_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

/**
 * \brief Generate AES key and IV.
 *
 * \param key Pointer to the buffer where the generated key will be stored.
 * \param iv Pointer to the buffer where the generated IV will be stored.
 */
void generate_aes_key_and_iv(unsigned char *key, unsigned char *iv);

/**
 * \brief Encrypt plaintext using AES.
 *
 * \param plaintext Pointer to the plaintext to be encrypted.
 * \param plaintext_len Length of the plaintext.
 * \param key Pointer to the AES key.
 * \param iv Pointer to the AES IV.
 * \param ciphertext Pointer to the buffer where the ciphertext will be stored.
 */
void aes_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, unsigned char *iv, unsigned char *ciphertext);

/**
 * \brief Decrypt ciphertext using AES.
 *
 * \param ciphertext Pointer to the ciphertext to be decrypted.
 * \param ciphertext_len Length of the ciphertext.
 * \param key Pointer to the AES key.
 * \param iv Pointer to the AES IV.
 * \param plaintext Pointer to the buffer where the decrypted plaintext will be stored.
 */
void aes_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned char *iv, unsigned char *plaintext);

#endif //MESSAGERIEPROJECTAP3_AESENCRYPTION_H
