#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(){

    // OPEN FILE TO READ
    FILE * f = fopen("image.png", "r");

    // GET FILE SIZE AND ALLOC SUFFICIENT MEMORY
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek (f, 0, SEEK_SET);
    unsigned char *img = malloc(length * sizeof(char));

    // STORE DATA INTO BUFFER
    fread(img, sizeof(char), length, f);

    // DISPLAY DATA
    //for(int i = 0; i < length; i++)
    //    printf("k[%d] = %d\n", i, img[i]);

    // CLOSE FILE
    fclose(f);

    // CREATE PNG FROM DATA
    f = fopen("cenas.png", "w");
    fwrite(img, sizeof(char), length, f);
    fclose(f);
}
