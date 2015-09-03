#include "fsutil.h"

char* read_file(const char *filename) {
    FILE *f = fopen(filename, "rb");
    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *data = malloc(fsize + 1);
    if (fread(data, fsize, 1, f) < fsize && ferror(f)) {
        fprintf(stderr, "Error reading file: %s", filename);
    }
    data[fsize] = 0;
    fclose(f);

    return data;
}
