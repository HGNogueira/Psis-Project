#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "phototransfer.h"
#include <inttypes.h>
#include <string.h>

#define BUFSIZE 1024

    /* transfers photo through socket to process calling photo_recv
     * on success returns 0 */
int phototransfer_send(int s, char *photo_name, uint32_t photo_id){
    FILE * f;
    char buffer[BUFSIZE];
    ssize_t photo_size, remain_data;
    ssize_t len;
    ssize_t tosend;
    char filename[60];

    filename[0] = '\0';
    if(photo_id != 0){//-1 means client is sending
        sprintf(filename, "./%" PRIu32, photo_id);
    }
    strcat(filename, photo_name);

    if((f = fopen(filename, "r") ) == NULL){
        perror("phototransfer_send (fopen):\n");
        photo_size = 0;
        send(s, &photo_size, sizeof(ssize_t), 0);
        return -1;
    }

    fseek(f, 0, SEEK_END);
    photo_size= (ssize_t) ftell(f);
    rewind(f);
    //send size
    send(s, &photo_size, sizeof(ssize_t), 0);

    remain_data = photo_size;
    tosend = fread(buffer, sizeof(char), BUFSIZE, f);
    while( (remain_data > 0)  && ((len = send(s, buffer, tosend, 0))  > 0)){
        tosend = fread(buffer, sizeof(char), BUFSIZE, f);
        remain_data = remain_data - len;
    }
    fclose(f);

    printf("Sent %u bytes\n", (unsigned)(photo_size - remain_data));

    return 0;
}


int phototransfer_recv(int s, char *photo_name, uint32_t photo_id){
    FILE *f;
    ssize_t photo_size, remain_data;
    char buffer[BUFSIZE];
    ssize_t len;
    char filename[60];

    filename[0] = '\0';
    if(photo_id != 0){
        sprintf(filename, "./%" PRIu32, photo_id);
    }
    strcat(filename, photo_name);

    if(recv(s, &photo_size, sizeof(ssize_t), 0) <= 0){
        printf("phototransfer_recv: socket disconnected while receiving photo_size\n");
        return -1;
    }
    if(photo_size == 0){
        printf("Counterpart couldn't find photo\n");
        return 1;
    }

    printf("Will receive a photo with size %u\n", (unsigned)photo_size);

    if((f = fopen(filename, "w")) == NULL){
        perror("phototransfer_recv (fopen):");
        printf("Server shutdown required\n");
        exit(EXIT_FAILURE);
    }

    remain_data = photo_size;
    while( (remain_data > 0) && ((len = recv(s, buffer, BUFSIZE, 0)) > 0)){
        fwrite(buffer, sizeof(char), len, f);
        remain_data = remain_data - len;
    }
    printf("Received %u bytes\n", (unsigned)(photo_size - remain_data));
    if(remain_data != 0){
        printf("Didn't receive whole file\n");
    }

    fclose(f);

    return 0;
}
