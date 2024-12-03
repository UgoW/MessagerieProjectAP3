#include "../Includes/aesEncryption.h"

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
    AES_KEY enc_key;
    AES_set_encrypt_key(key, AES_KEY_SIZE * 8, &enc_key);

    int num_blocks = plaintext_len / AES_BLOCK_SIZE;
    for (int i = 0; i < num_blocks; i++) {
        AES_cbc_encrypt(plaintext + (i * AES_BLOCK_SIZE), ciphertext + (i * AES_BLOCK_SIZE), AES_BLOCK_SIZE, &enc_key, iv, AES_ENCRYPT);
    }
}

void aes_decrypt(const unsigned char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned char *iv, unsigned char *plaintext) {
    AES_KEY dec_key;
    AES_set_decrypt_key(key, AES_KEY_SIZE * 8, &dec_key);

    int num_blocks = ciphertext_len / AES_BLOCK_SIZE;
    for (int i = 0; i < num_blocks; i++) {
        AES_cbc_encrypt(ciphertext + (i * AES_BLOCK_SIZE), plaintext + (i * AES_BLOCK_SIZE), AES_BLOCK_SIZE, &dec_key, iv, AES_DECRYPT);
    }
}
