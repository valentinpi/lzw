#include "lzw.h"

int main(int argc, char *argv[])
{
    if (argc < 2 || strncmp(argv[1], "--help", 6) == 0) {
        printf("Usage: lzw <string>\n");
        return EXIT_FAILURE;
    }

    printf("String: %s\n", argv[1]);

    // Encoding
    uint16_t *example_encoded = malloc(4096 * sizeof(uint16_t));
    uint64_t example_encoded_len = 0;
    lzw_encode(argv[1], &example_encoded, &example_encoded_len);

    printf("LZW-encoded: ");
    for (uint64_t i = 0; i < example_encoded_len; i++) {
        uint16_t cur = example_encoded[i];
        
        if (cur > 0xFF) {
            printf("<%"PRIu16"> ", cur);
        }
        else {
            printf("%c ", (char) cur);
        }
    }
    printf("\n");

    double compression_ratio = (double) example_encoded_len / (double) strlen(argv[1]) * 100.0;
    printf("Compression ratio: %"PRIu64" -> %"PRIu64" (%f%%)\n", strlen(argv[1]), example_encoded_len, compression_ratio);

    // Decoding
    char *example_decoded = malloc(4096);
    memset(example_decoded, 0, 4096);
    uint64_t example_decoded_len = 0;
    lzw_decode(example_encoded, example_encoded_len, &example_decoded, &example_decoded_len);

    printf("LZW-decoded: %s\n", example_decoded);

    free(example_decoded);
    free(example_encoded);

    return EXIT_SUCCESS;
}
