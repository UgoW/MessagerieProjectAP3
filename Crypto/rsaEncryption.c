//
// Created by Ugo WAREMBOURG on 28/11/2024.
//

#include "../Includes/rsaEncryption.h"
#include <stdio.h>
#include <stdlib.h>

RSA* generate_rsa_keys() {
    RSA *key = RSA_new();
    BIGNUM *e = BN_new();
    BN_set_word(e, RSA_F4);

    RSA_generate_key_ex(key, RSA_KEY_BITS, e, NULL);
    BN_free(e);

    return key;
}

void rsa_encrypt(RSA *key, const unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext, int *ciphertext_len) {
    *ciphertext_len = RSA_public_encrypt(plaintext_len, plaintext, ciphertext, key, RSA_PKCS1_OAEP_PADDING);
    if (*ciphertext_len == -1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

void rsa_decrypt(RSA *key, const unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext, int *plaintext_len) {
    *plaintext_len = RSA_private_decrypt(ciphertext_len, ciphertext, plaintext, key, RSA_PKCS1_OAEP_PADDING);
    if (*plaintext_len == -1) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}
