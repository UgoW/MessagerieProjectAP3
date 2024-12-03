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

void generate_aes_key_and_iv(unsigned char *key, unsigned char *iv);
void aes_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, unsigned char *iv, unsigned char *ciphertext);
void aes_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned char *iv, unsigned char *plaintext);

#endif //MESSAGERIEPROJECTAP3_AESENCRYPTION_H
