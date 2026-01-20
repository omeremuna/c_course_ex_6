#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "org_tree.h"
#define FP_LEN 9


static void print_success(int mask, char *op, char* fingerprint, char* First_Name, char* Second_Name)
{
    printf("Successful Decrypt! The Mask used was mask_%d of type (%s) and The fingerprint was %.*s belonging to %s %s\n",
                       mask, op, FP_LEN, fingerprint, First_Name, Second_Name);
}

static void print_unsuccess()
{
    printf("Unsuccesful decrypt, Looks like he got away\n");
}

static unsigned char binary_string_to_byte(const char *s) {
    unsigned char result = 0;
    for (int i = 0; i < 8 && s[i] != '\0' && s[i] != '\n' && s[i] != '\r'; i++) {
        result = (result << 1) | (s[i] - '0');
    }
    return result;
}

static Node *find_fingerprint_in_org(const Org *org, const char *fp) {
    if (!org) return NULL;

    // Check boss
    if (org->boss && strncmp(org->boss->fingerprint, fp, FP_LEN) == 0) {
        return org->boss;
    }

    // Check left hand
    if (org->left_hand && strncmp(org->left_hand->fingerprint, fp, FP_LEN) == 0) {
        return org->left_hand;
    }

    // Check left hand's supports
    if (org->left_hand) {
        Node *curr = org->left_hand->supports_head;
        while (curr) {
            if (strncmp(curr->fingerprint, fp, FP_LEN) == 0) {
                return curr;
            }
            curr = curr->next;
        }
    }

    // Check right hand
    if (org->right_hand && strncmp(org->right_hand->fingerprint, fp, FP_LEN) == 0) {
        return org->right_hand;
    }

    // Check right hand's supports
    if (org->right_hand) {
        Node *curr = org->right_hand->supports_head;
        while (curr) {
            if (strncmp(curr->fingerprint, fp, FP_LEN) == 0) {
                return curr;
            }
            curr = curr->next;
        }
    }

    return NULL;
}

static int check_and_match(const char *fingerprint, const unsigned char *cipher, int mask) {
    for (int i = 0; i < FP_LEN; i++) {
        unsigned char expected = (unsigned char)fingerprint[i] & (unsigned char)mask;
        if (expected != cipher[i]) {
            return 0;
        }
    }
    return 1;
}

static Node *find_and_match_in_org(const Org *org, const unsigned char *cipher, int mask) {
    if (!org) return NULL;

    // Check boss
    if (org->boss && check_and_match(org->boss->fingerprint, cipher, mask)) {
        return org->boss;
    }

    // Check left hand
    if (org->left_hand && check_and_match(org->left_hand->fingerprint, cipher, mask)) {
        return org->left_hand;
    }

    // Check left hand's supports
    if (org->left_hand) {
        Node *curr = org->left_hand->supports_head;
        while (curr) {
            if (check_and_match(curr->fingerprint, cipher, mask)) {
                return curr;
            }
            curr = curr->next;
        }
    }

    // Check right hand
    if (org->right_hand && check_and_match(org->right_hand->fingerprint, cipher, mask)) {
        return org->right_hand;
    }

    // Check right hand's supports
    if (org->right_hand) {
        Node *curr = org->right_hand->supports_head;
        while (curr) {
            if (check_and_match(curr->fingerprint, cipher, mask)) {
                return curr;
            }
            curr = curr->next;
        }
    }

    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <clean_file.txt> <cipher_bits.txt> <mask_start_s>\n", argv[0]);
        return 0;
    }

    // Build the organization tree
    Org org = build_org_from_clean_file(argv[1]);

    // Read cipher bits from file
    FILE *cipher_file = fopen(argv[2], "r");
    if (!cipher_file) {
        free_org(&org);
        return 0;
    }

    unsigned char cipher[FP_LEN];
    char line[16];
    for (int i = 0; i < FP_LEN; i++) {
        if (fgets(line, sizeof(line), cipher_file)) {
            cipher[i] = binary_string_to_byte(line);
        }
    }
    fclose(cipher_file);

    // Parse mask start
    int s = atoi(argv[3]);

    // Brute force decrypt
    for (int mask = s; mask <= s + 10; mask++) {
        // Try XOR
        char decrypted[FP_LEN + 1];
        for (int i = 0; i < FP_LEN; i++) {
            decrypted[i] = cipher[i] ^ mask;
        }
        decrypted[FP_LEN] = '\0';

        Node *match = find_fingerprint_in_org(&org, decrypted);
        if (match) {
            print_success(mask, "XOR", match->fingerprint, match->first, match->second);
            free_org(&org);
            return 0;
        }

        // Try AND
        match = find_and_match_in_org(&org, cipher, mask);
        if (match) {
            print_success(mask, "AND", match->fingerprint, match->first, match->second);
            free_org(&org);
            return 0;
        }
    }

    // No match found
    print_unsuccess();
    free_org(&org);
    return 0;
}
