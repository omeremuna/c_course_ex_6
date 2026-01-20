#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_FIELD 256
#define INITIAL_CAPACITY 16

typedef struct {
    char first_name[MAX_FIELD];
    char second_name[MAX_FIELD];
    char fingerprint[16];
    char position[32];
    int order;
} Entry;

int is_corruption_char(char c) {
    return (c == '#' || c == '?' || c == '!' || c == '@' || c == '&' || c == '$');
}

char *clean_input(const char *input, size_t len) {
    char *clean = malloc(len + 1);
    if (!clean) return NULL;

    size_t j = 0;
    for (size_t i = 0; i < len; i++) {
        char c = input[i];
        if (!is_corruption_char(c) && c != '\n' && c != '\r') {
            clean[j++] = c;
        }
    }
    clean[j] = '\0';
    return clean;
}

int get_position_priority(const char *pos) {
    if (strcmp(pos, "Boss") == 0) return 0;
    if (strcmp(pos, "Right Hand") == 0) return 1;
    if (strcmp(pos, "Left Hand") == 0) return 2;
    if (strcmp(pos, "Support_Right") == 0) return 3;
    if (strcmp(pos, "Support_Left") == 0) return 4;
    return 5;
}

int is_valid_position(const char *pos) {
    return get_position_priority(pos) < 5;
}

int is_duplicate_fingerprint(Entry *entries, int count, const char *fp) {
    for (int i = 0; i < count; i++) {
        if (strcmp(entries[i].fingerprint, fp) == 0) {
            return 1;
        }
    }
    return 0;
}

int compare_entries(const void *a, const void *b) {
    const Entry *ea = (const Entry *)a;
    const Entry *eb = (const Entry *)b;

    int pa = get_position_priority(ea->position);
    int pb = get_position_priority(eb->position);

    if (pa != pb) return pa - pb;
    return ea->order - eb->order;
}

Entry *parse_entries(const char *clean, int *count_out) {
    int capacity = INITIAL_CAPACITY;
    int count = 0;
    int order = 0;

    Entry *entries = malloc(capacity * sizeof(Entry));
    if (!entries) {
        *count_out = 0;
        return NULL;
    }

    const char *ptr = clean;

    while ((ptr = strstr(ptr, "First Name:")) != NULL) {
        const char *fn_start = ptr + 11;
        const char *sn_label = strstr(fn_start, "Second Name:");
        if (!sn_label) break;

        const char *sn_start = sn_label + 12;
        const char *fp_label = strstr(sn_start, "Fingerprint:");
        if (!fp_label) break;

        const char *fp_start = fp_label + 12;
        const char *pos_label = strstr(fp_start, "Position:");
        if (!pos_label) break;

        const char *pos_start = pos_label + 9;

        // Extract first name
        size_t fn_len = sn_label - fn_start;
        while (fn_len > 0 && fn_start[fn_len-1] == ' ') fn_len--;
        while (fn_len > 0 && *fn_start == ' ') { fn_start++; fn_len--; }

        // Extract second name
        size_t sn_len = fp_label - sn_start;
        while (sn_len > 0 && sn_start[sn_len-1] == ' ') sn_len--;
        while (sn_len > 0 && *sn_start == ' ') { sn_start++; sn_len--; }

        // Skip leading spaces for fingerprint
        while (*fp_start == ' ') fp_start++;

        // Extract exactly 9 alphanumeric characters for fingerprint
        char fp_buf[16];
        int fp_idx = 0;
        const char *fp_ptr = fp_start;
        while (fp_idx < 9 && fp_ptr < pos_label) {
            if (isalnum((unsigned char)*fp_ptr)) {
                fp_buf[fp_idx++] = *fp_ptr;
            }
            fp_ptr++;
        }
        fp_buf[fp_idx] = '\0';

        if (fp_idx != 9) {
            ptr = sn_label;
            continue;
        }

        // Skip leading spaces for position
        while (*pos_start == ' ') pos_start++;

        // Find position value
        char pos_buf[32] = {0};
        if (strncmp(pos_start, "Boss", 4) == 0) {
            strcpy(pos_buf, "Boss");
        } else if (strncmp(pos_start, "Right Hand", 10) == 0) {
            strcpy(pos_buf, "Right Hand");
        } else if (strncmp(pos_start, "Left Hand", 9) == 0) {
            strcpy(pos_buf, "Left Hand");
        } else if (strncmp(pos_start, "Support_Right", 13) == 0) {
            strcpy(pos_buf, "Support_Right");
        } else if (strncmp(pos_start, "Support_Left", 12) == 0) {
            strcpy(pos_buf, "Support_Left");
        } else {
            ptr = sn_label;
            continue;
        }

        // Check for duplicate fingerprint
        if (is_duplicate_fingerprint(entries, count, fp_buf)) {
            ptr = sn_label;
            continue;
        }

        // Grow array if needed
        if (count >= capacity) {
            capacity *= 2;
            Entry *new_entries = realloc(entries, capacity * sizeof(Entry));
            if (!new_entries) {
                *count_out = count;
                return entries;
            }
            entries = new_entries;
        }

        // Store entry
        if (fn_len >= MAX_FIELD) fn_len = MAX_FIELD - 1;
        if (sn_len >= MAX_FIELD) sn_len = MAX_FIELD - 1;

        strncpy(entries[count].first_name, fn_start, fn_len);
        entries[count].first_name[fn_len] = '\0';

        strncpy(entries[count].second_name, sn_start, sn_len);
        entries[count].second_name[sn_len] = '\0';

        strcpy(entries[count].fingerprint, fp_buf);
        strcpy(entries[count].position, pos_buf);
        entries[count].order = order++;

        count++;
        ptr = sn_label;
    }

    *count_out = count;
    return entries;
}

//TODO create functions that you can use to clean up the file
int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <input_corrupted.txt> <output_clean.txt>\n", argv[0]);
        return 0;
    }
    // TODO: implement

    FILE *fin = fopen(argv[1], "rb");
    if (!fin) {
        printf("Error opening file: %s\n", argv[1]);
        return 0;
    }

    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);

    char *raw = malloc(file_size + 1);
    if (!raw) {
        fclose(fin);
        return 0;
    }

    size_t bytes_read = fread(raw, 1, file_size, fin);
    raw[bytes_read] = '\0';
    fclose(fin);

    char *clean = clean_input(raw, bytes_read);
    free(raw);

    if (!clean) {
        return 0;
    }

    int count = 0;
    Entry *entries = parse_entries(clean, &count);
    free(clean);

    if (!entries) {
        return 0;
    }

    qsort(entries, count, sizeof(Entry), compare_entries);

    FILE *fout = fopen(argv[2], "w");
    if (!fout) {
        printf("Error opening file: %s\n", argv[2]);
        free(entries);
        return 0;
    }

    for (int i = 0; i < count; i++) {
        fprintf(fout, "First Name: %s\n", entries[i].first_name);
        fprintf(fout, "Second Name: %s\n", entries[i].second_name);
        fprintf(fout, "Fingerprint: %s\n", entries[i].fingerprint);
        fprintf(fout, "Position: %s\n", entries[i].position);
        fprintf(fout, "\n");
    }

    fclose(fout);
    free(entries);

    return 0;
}
