#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(){
    FILE *fp = fopen("111.png", "rb");
    if (fp == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }
    FILE *fpr = fopen("222.png", "wb");
    if (fpr  == NULL) {
        perror("Error creating file");
        return EXIT_FAILURE;
    }

    char buff[1024];
    while(fread(buff, 1, sizeof(buff), fp) > 0) {
        fwrite(buff, 1, sizeof(buff), fpr);
    }
    fclose(fp);
    fclose(fpr);
    getchar();
    return 0;


}