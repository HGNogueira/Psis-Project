#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "phototransfer.h"

#define BUFSIZE 1024

    /* transfers photo through socket to process calling photo_recv
     * on success returns 0
     * photo_size is calculated and passed through pointer */
int phototransfer_send(int s, char *photo_name){
    FILE * f;
    char buffer[BUFSIZE];
    ssize_t photo_size, remain_data;
    ssize_t len;
    ssize_t tosend;

    if((f = fopen(photo_name, "r") ) == NULL){
        perror("phototransfer_send (fopen):\n");
        return -1;
    }

    fseek(f, 0, SEEK_END);
    photo_size= (ssize_t) ftell(f);
    rewind(f);
    //send size
    send(s, &photo_size, sizeof(ssize_t), 0);

    remain_data = photo_size;
    tosend = fread(buffer, sizeof(char), BUFSIZE, f);
    while(((len = send(s, buffer, BUFSIZE, 0))  > 0) && (remain_data > 0)){
        tosend = fread(buffer, sizeof(char), BUFSIZE, f);
        remain_data = remain_data - len;
    }
    fclose(f);

    printf("Sent %u bytes\n", (unsigned)(photo_size - remain_data));

    return 0;
}


int phototransfer_recv(int s, char *photo_name){
    FILE *f;
    ssize_t photo_size, remain_data;
    char buffer[BUFSIZE];
    ssize_t len;

    if(recv(s, &photo_size, sizeof(ssize_t), 0) <= 0){
        printf("phototransfer_recv: socket disconnected while receiving photo_size\n");
        return -1;
    }

    printf("Will receive a photo with size %u\n", (unsigned)photo_size);

    if((f = fopen(photo_name, "w")) == NULL){
        perror("phototransfer_recv (fopen):");
        return -1;
    }

    remain_data = photo_size;
    while(((len = recv(s, buffer, BUFSIZE, 0)) > 0) && (remain_data > 0)){
        fwrite(buffer, sizeof(char), len, f);
        remain_data = remain_data - len;
    }
    printf("Received %u bytes\n", (unsigned)(photo_size - remain_data));

    fclose(f);

    return 0;
}
