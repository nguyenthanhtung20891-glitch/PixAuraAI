#include "pixaura/document.h"
#include <stdio.h>
#include <string.h>

#define CHECK(e) do { if (!(e)) { fprintf(stderr, "C document line %d\n", __LINE__); return 1; } } while (0)
int main(int argc, char** argv) {
    uint8_t input[8192], output[8192];
    const uint8_t context_id[] = "000000000000000000000000000000a1";
    const uint8_t session_id[] = "00000000000000000000000000000500";
    pixaura_document_context context = {0};
    pixaura_document_handle handle = {0};
    pixaura_document_error error = {0};
    uint64_t required = 0;
    FILE* file;
    size_t size;
    CHECK((argc == 2 || argc == 3) && sizeof(error) == 184 && offsetof(pixaura_document_error, message) == 24);
    file = fopen(argv[1], "rb"); CHECK(file != NULL);
    size = fread(input, 1, sizeof(input), file); CHECK(fclose(file) == 0 && size > 0 && size < sizeof(input));
    error.api_version = 1; error.struct_size = sizeof(error);
    CHECK(pixaura_document_context_init(1, &context, sizeof(context), context_id, 32, &error) == 0);
    CHECK(pixaura_document_open(1, &context, input, size, session_id, 32, &handle, &error) == 0);
    memset(input, 0, sizeof(input)); /* library does not borrow the input */
    CHECK(pixaura_document_serialize(&context, &handle, NULL, 0, &required, &error) == 0 && required == size);
    memset(output, 0x5a, sizeof(output));
    CHECK(pixaura_document_serialize(&context, &handle, output, 1, &required, &error) == PIXAURA_DOCUMENT_BUFFER_TOO_SMALL && output[0] == 0x5a);
    CHECK(pixaura_document_serialize(&context, &handle, output, sizeof(output), &required, &error) == 0);
    CHECK(output[size - 1] == '\n' && output[size] == 0x5a);
    CHECK(pixaura_document_context_destroy(&context) == 0);
    CHECK(pixaura_document_release(&context, &handle) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    CHECK(pixaura_document_context_destroy(&context) == PIXAURA_DOCUMENT_INVALID_HANDLE);
    file = fopen(argv[1], "rb"); CHECK(file != NULL);
    CHECK(fread(input, 1, sizeof(input), file) == size && fclose(file) == 0);
    CHECK(memcmp(input, output, size) == 0);
    if (argc == 3) {
        file = fopen(argv[2], "wb"); CHECK(file != NULL);
        CHECK(fwrite(output, 1, size, file) == size && fclose(file) == 0);
    }
    puts("Independent C document fixture and ownership PASS");
    return 0;
}
