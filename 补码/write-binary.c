#include <stdio.h>

int main() {
    FILE *fp = fopen("./binary-file-1", "w");
    int n = 12345;
    fwrite(&n, sizeof(n), 1, fp);
    fclose(fp);
}
