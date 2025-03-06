#include <stdio.h>

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

int main(int argc, char **argv) {
    FILE *fp = fopen(argv[1], "rb");

    unsigned char buffer[1];
    while (1) {
        int size = fread(buffer, 1, 1, fp);
        if (size == 0) break;
        printf(BYTE_TO_BINARY_PATTERN, BYTE_TO_BINARY(buffer[0]));
        //printf("%c", buffer[0]);
        //printf(" ");
    }
    printf("\n");
    fclose(fp);
}
