// See https://de.wikipedia.org/wiki/Lempel-Ziv-Welch-Algorithmus#Beispiel_zur_Dekompression for reference
#pragma once

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint16_t code;
    char     *str;
} lzw_dict_entry;

void lzw_encode(const char *src, uint16_t **dest, uint64_t *dest_len)
{
    // Since a 12-bit code is used, the size of the dict does not exceed 2^12=4096
    lzw_dict_entry dict[4096];
    uint64_t dict_len = 0;
    uint64_t src_size = strlen(src);

    // First, compute strings of individual characters
    for (uint64_t i = 0; i < src_size; i++) {
        int skip = 0;

        for (uint64_t j = 0; j < dict_len; j++) {
            if (dict[j].str[0] == src[i]) {
                skip = 1;
                break;
            }
        }

        if (!skip) {
            dict[dict_len].code   = (uint16_t) src[i];
            dict[dict_len].str    = malloc(2);
            dict[dict_len].str[0] = src[i];
            dict[dict_len].str[1] = '\0';
            dict_len++;
        }
    }

    uint64_t base_offset = 0;
    uint64_t dest_index = 0;
    char pattern[4097];
    memset(pattern, 0, 4097);
    for (uint64_t i = 0; i < src_size; i++) {
        char cur_str[2];
        cur_str[0] = src[i];
        cur_str[1] = '\0';

        char new_pattern[4097];
        memcpy(new_pattern, pattern, 4097);
        strncat(new_pattern, cur_str, 2);
        uint64_t new_pattern_len = strlen(new_pattern);

        lzw_dict_entry *entry = NULL;
        for (uint64_t j = 0; j < dict_len; j++) {
            if (strlen(dict[j].str) == new_pattern_len &&
                strncmp(dict[j].str, new_pattern, new_pattern_len) == 0) {
                entry = &dict[j];
                break;
            }
        }

        if (entry != NULL) {
            strncat(pattern, cur_str, 2);
        }
        else {
            // Add new code
            dict[dict_len].code = 0x100 + base_offset;
            dict[dict_len].str  = malloc(new_pattern_len + 1);
            memcpy(dict[dict_len].str, new_pattern, new_pattern_len + 1);
            dict_len++;
            base_offset++;

            // Output pattern
            for (uint64_t j = 0; j < dict_len; j++) {
                if (strlen(pattern) == strlen(dict[j].str) &&
                    strncmp(pattern, dict[j].str, strlen(pattern)) == 0) {
                    (*dest)[dest_index] = dict[j].code;
                    dest_index++;
                    break;
                }
            }

            // Set new character
            memset(pattern, 0, 4097);
            memcpy(pattern, cur_str, 2);
        }
    }

    if (strlen(pattern) != 0) {
        lzw_dict_entry *entry = NULL;

        for (uint64_t i = 0; i < dict_len; i++) {
            if (strlen(pattern) == strlen(dict[i].str) &&
                strncmp(pattern, dict[i].str, strlen(pattern)) == 0) {
                entry = &dict[i];
                break;
            }
        }

        if (entry != NULL) {
            (*dest)[dest_index] = entry->code;
            dest_index++;
        }
        else {
            (*dest)[dest_index] = 256 + base_offset;
            dest_index++;

            // Just to be consistent, we will add the entry
            dict[dict_len].code = 256 + base_offset;
            dict[dict_len].str = malloc(strlen(pattern));
            memcpy(dict[dict_len].str, pattern, strlen(pattern));
            base_offset++;
            dict_len++;
        }
    }

    *dest_len = dest_index;

    for (uint64_t i = 0; i < dict_len; i++) {
        free(dict[i].str);
    }
}

void lzw_decode(const uint16_t *src, uint64_t src_size, char **dest, uint64_t *dest_len) {
    lzw_dict_entry dict[4096];
    uint64_t dict_len = 0;

    // First, compute strings of individual characters
    for (uint64_t i = 0; i < src_size; i++) {
        if (src[i] > 0xFF) {
            continue;
        }

        int skip = 0;
        for (uint64_t j = 0; j < dict_len; j++) {
            if (dict[j].code == src[i]) {
                skip = 1;
            }
        }

        if (!skip) {
            dict[dict_len].code   = src[i];
            dict[dict_len].str    = malloc(2);
            dict[dict_len].str[0] = (char) src[i];
            dict[dict_len].str[1] = '\0';
            dict_len++;
        }
    }

    uint64_t base_offset = 0;
    uint64_t dest_index = 0;
    lzw_dict_entry *last_entry = &dict[0];
    uint64_t last_entry_len = 1;
    assert(last_entry != NULL);

    memcpy(*dest, last_entry->str, 1);
    dest_index++;

    for (uint64_t i = 1; i < src_size; i++) {
        uint16_t next_code = src[i];
        lzw_dict_entry *next_entry = NULL;

        for (uint64_t j = 0; j < dict_len; j++) {
            if (dict[j].code == next_code) {
                next_entry = &dict[j];
                break;
            }
        }

        dict[dict_len].code = 256 + base_offset;
        dict[dict_len].str = malloc(last_entry_len + 2);
        memset(dict[dict_len].str, 0, last_entry_len + 2);
        memcpy(dict[dict_len].str, last_entry->str, last_entry_len);
        if (next_entry != NULL) {
            dict[dict_len].str[last_entry_len] = next_entry->str[0];
        }
        else {
            dict[dict_len].str[last_entry_len] = last_entry->str[0];
            next_entry = &dict[dict_len];
        }

        dict_len++;
        base_offset++;

        memcpy(*dest + dest_index, next_entry->str, strlen(next_entry->str));
        dest_index += strlen(next_entry->str);

        last_entry = next_entry;
        last_entry_len = strlen(last_entry->str);
    }
    *dest_len = dest_index;

    for (uint64_t i = 0; i < dict_len; i++) {
        free(dict[i].str);
    }
}
