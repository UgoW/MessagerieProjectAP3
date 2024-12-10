//
// Created by Ugo WAREMBOURG on 10/12/2024.
//

#include "../Includes/rsaEncryption.h"

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

EVP_PKEY *rsa_read_public_key(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    EVP_PKEY *key = PEM_read_PUBKEY(file, NULL, NULL, NULL);
    fclose(file);

    return key;
}