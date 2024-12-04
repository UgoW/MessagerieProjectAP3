#include <openssl/evp.h>
#include <openssl/rand.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AES_KEY_SIZE 16      // Taille clé pour AES-256
#define AES_BLOCK_SIZE 16     // Taille de bloc AES (en bytes)

void generate_aes_key_and_iv(unsigned char *key, unsigned char *iv) {
    if (RAND_bytes(key, AES_KEY_SIZE) != 1) {
        fprintf(stderr, "AESERROR : Generation key.\n");
        exit(EXIT_FAILURE);
    }

    if (RAND_bytes(iv, AES_BLOCK_SIZE) != 1) {
        fprintf(stderr, "AESERROR : Generation iv.\n");
        exit(EXIT_FAILURE);
    }
}

void aes_encrypt(const unsigned char *plaintext, int plaintext_len, const unsigned char *key, unsigned char *iv, unsigned char *ciphertext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "AESERROR : EVP_CIPHER_CTX_new failed.\n");
        exit(EXIT_FAILURE);
    }

    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "AESERROR : EVP_EncryptInit_ex failed.\n");
        exit(EXIT_FAILURE);
    }

    int len;
    int ciphertext_len = 0;

    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        fprintf(stderr, "AESERROR : EVP_EncryptUpdate failed.\n");
        exit(EXIT_FAILURE);
    }
    ciphertext_len = len;

    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        fprintf(stderr, "AESERROR : EVP_EncryptFinal_ex failed.\n");
        exit(EXIT_FAILURE);
    }
    ciphertext_len += len;

    EVP_CIPHER_CTX_free(ctx);
}

void aes_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        fprintf(stderr, "AESERROR : EVP_CIPHER_CTX_new failed.\n");
        exit(EXIT_FAILURE);
    }

    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        fprintf(stderr, "AESERROR : EVP_DecryptInit_ex failed.\n");
        exit(EXIT_FAILURE);
    }

    int len;
    int plaintext_len = 0;

    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        fprintf(stderr, "AESERROR : EVP_DecryptUpdate failed.\n");
        exit(EXIT_FAILURE);
    }
    plaintext_len = len;

    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        fprintf(stderr, "AESERROR : EVP_DecryptFinal_ex failed.\n");
        exit(EXIT_FAILURE);
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
}
