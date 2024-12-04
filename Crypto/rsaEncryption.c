#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bn.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <stdio.h>
#include <stdlib.h>

#define RSA_KEY_BITS 2048    // Taille de la clé RSA (bits)

void rsa_encrypt(EVP_PKEY *key, const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (EVP_PKEY_encrypt(ctx, ciphertext, (size_t *)ciphertext_len, plaintext, plaintext_len) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    EVP_PKEY_CTX_free(ctx);
}

void rsa_decrypt(EVP_PKEY *key, const unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, int *plaintext_len) {
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(key, NULL);
    if (!ctx) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (EVP_PKEY_decrypt(ctx, plaintext, (size_t *)plaintext_len, ciphertext, ciphertext_len) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    EVP_PKEY_CTX_free(ctx);
}
