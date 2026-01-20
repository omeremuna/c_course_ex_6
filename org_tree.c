#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "org_tree.h"

static Node *create_node(const char *first, const char *second,
                         const char *fingerprint, const char *position) {
    Node *node = (Node *)malloc(sizeof(Node));
    if (!node) return NULL;

    strncpy(node->first, first, MAX_FIELD - 1);
    node->first[MAX_FIELD - 1] = '\0';

    strncpy(node->second, second, MAX_FIELD - 1);
    node->second[MAX_FIELD - 1] = '\0';

    strncpy(node->fingerprint, fingerprint, MAX_FIELD - 1);
    node->fingerprint[MAX_FIELD - 1] = '\0';

    strncpy(node->position, position, MAX_POS - 1);
    node->position[MAX_POS - 1] = '\0';

    node->left = NULL;
    node->right = NULL;
    node->supports_head = NULL;
    node->next = NULL;

    return node;
}

static void trim_newline(char *str) {
    size_t len = strlen(str);
    while (len > 0 && (str[len-1] == '\n' || str[len-1] == '\r')) {
        str[--len] = '\0';
    }
}

Org build_org_from_clean_file(const char *path) {
    Org org = {NULL, NULL, NULL};

    FILE *file = fopen(path, "r");
    if (!file) {
        return org;
    }

    char line[512];
    char first[MAX_FIELD] = {0};
    char second[MAX_FIELD] = {0};
    char fingerprint[MAX_FIELD] = {0};
    char position[MAX_POS] = {0};

    while (fgets(line, sizeof(line), file)) {
        trim_newline(line);

        if (strncmp(line, "First Name: ", 12) == 0) {
            strncpy(first, line + 12, MAX_FIELD - 1);
            first[MAX_FIELD - 1] = '\0';
        } else if (strncmp(line, "Second Name: ", 13) == 0) {
            strncpy(second, line + 13, MAX_FIELD - 1);
            second[MAX_FIELD - 1] = '\0';
        } else if (strncmp(line, "Fingerprint: ", 13) == 0) {
            strncpy(fingerprint, line + 13, MAX_FIELD - 1);
            fingerprint[MAX_FIELD - 1] = '\0';
        } else if (strncmp(line, "Position: ", 10) == 0) {
            strncpy(position, line + 10, MAX_POS - 1);
            position[MAX_POS - 1] = '\0';

            // Create the node
            Node *node = create_node(first, second, fingerprint, position);
            if (!node) continue;

            // Place in tree based on position
            if (strcmp(position, "Boss") == 0) {
                org.boss = node;
            } else if (strcmp(position, "Left Hand") == 0) {
                org.left_hand = node;
                if (org.boss) {
                    org.boss->left = node;
                }
            } else if (strcmp(position, "Right Hand") == 0) {
                org.right_hand = node;
                if (org.boss) {
                    org.boss->right = node;
                }
            } else if (strcmp(position, "Support_Left") == 0) {
                if (org.left_hand) {
                    // Add to end of support list
                    if (org.left_hand->supports_head == NULL) {
                        org.left_hand->supports_head = node;
                    } else {
                        Node *curr = org.left_hand->supports_head;
                        while (curr->next) {
                            curr = curr->next;
                        }
                        curr->next = node;
                    }
                }
            } else if (strcmp(position, "Support_Right") == 0) {
                if (org.right_hand) {
                    // Add to end of support list
                    if (org.right_hand->supports_head == NULL) {
                        org.right_hand->supports_head = node;
                    } else {
                        Node *curr = org.right_hand->supports_head;
                        while (curr->next) {
                            curr = curr->next;
                        }
                        curr->next = node;
                    }
                }
            }

            // Reset for next entry
            first[0] = '\0';
            second[0] = '\0';
            fingerprint[0] = '\0';
            position[0] = '\0';
        }
    }

    fclose(file);
    return org;
}

static void print_node(const Node *node) {
    if (!node) return;
    printf("First Name: %s\n", node->first);
    printf("Second Name: %s\n", node->second);
    printf("Fingerprint: %s\n", node->fingerprint);
    printf("Position: %s\n", node->position);
    printf("\n");
}

void print_tree_order(const Org *org) {
    if (!org) return;

    // Print Boss
    if (org->boss) {
        print_node(org->boss);
    }

    // Print Left Hand
    if (org->left_hand) {
        print_node(org->left_hand);

        // Print Left Hand's supports
        Node *curr = org->left_hand->supports_head;
        while (curr) {
            print_node(curr);
            curr = curr->next;
        }
    }

    // Print Right Hand
    if (org->right_hand) {
        print_node(org->right_hand);

        // Print Right Hand's supports
        Node *curr = org->right_hand->supports_head;
        while (curr) {
            print_node(curr);
            curr = curr->next;
        }
    }
}

static void free_support_list(Node *head) {
    while (head) {
        Node *next = head->next;
        free(head);
        head = next;
    }
}

void free_org(Org *org) {
    if (!org) return;

    // Free support lists first
    if (org->left_hand) {
        free_support_list(org->left_hand->supports_head);
        free(org->left_hand);
    }

    if (org->right_hand) {
        free_support_list(org->right_hand->supports_head);
        free(org->right_hand);
    }

    // Free boss
    if (org->boss) {
        free(org->boss);
    }

    org->boss = NULL;
    org->left_hand = NULL;
    org->right_hand = NULL;
}
