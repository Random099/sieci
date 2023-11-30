#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

struct Packet{
    uint32_t id;
    uint32_t byteCount;
    unsigned char data[3];
};

void shuffle(struct Packet* array, size_t n) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int usec = tv.tv_usec;
    srand(usec);


    if (n > 1) {
        size_t i;
        for (i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (rand()%i);
            printf("sz: %i\n", j);
            struct Packet t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

unsigned char* strToDecStr(unsigned char* string, unsigned char* dst){
    for(int i = 0; i < strlen(string); i++){
        unsigned char* temp = (unsigned char*)malloc(sizeof(unsigned char)*4);
        temp[0] = 0x00;
        sprintf(temp, "%d", string[i]);
        strcat(dst, temp);
        strcat(dst, " ");
        free(temp);
    }
    return dst;
}

int main(){
    FILE* dataFile = fopen("test.bin", "wb");
    if (dataFile != NULL){
        unsigned char data[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        fwrite(data, sizeof(data), 1, dataFile);
        fclose(dataFile);
    }
    FILE* fp = fopen("test.bin", "rb");
    struct Packet readPacket;

    //Calculate file size and return to beginning
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);
    printf("Filesize: %i\n", fileSize);

    //Allocate buffer
    struct Packet* buffer = (struct Packet*)malloc(sizeof(struct Packet) * ((fileSize + 1) / 2));

    //Read file into buffer of desired form packets
    int bufferIdx = 0;
    size_t chunkSize = sizeof(readPacket.data) - 1;
    for(; (fread(&readPacket.data, chunkSize, 1, fp) == 1); ++bufferIdx){
        readPacket.id = bufferIdx+1;
        readPacket.byteCount = chunkSize;
        buffer[bufferIdx] = readPacket;
        buffer[bufferIdx].data[ARRAYSIZE(readPacket.data) - 1] = 0x00;
    }

    //Check for remaining bytes
    int bytesLeft = fileSize - (bufferIdx * 2);
    if(bytesLeft != 0){
        fread(&readPacket.data, bytesLeft, 1, fp);
        readPacket.data[bytesLeft] = 0x00;
        readPacket.id = bufferIdx+1;
        readPacket.byteCount = bytesLeft;
        buffer[bufferIdx] = readPacket;
        bufferIdx++;
    }

    printf("[Original]:\n");
    for (int i = 0; i < bufferIdx; i++){
        unsigned char* printPtr = (unsigned char*)malloc(9 * sizeof(unsigned char));
        printPtr[0] = 0x00;
        printf("id: %08d byte count: %08d data: %s\n", buffer[i].id, buffer[i].byteCount, strToDecStr(buffer[i].data, printPtr));
        free(printPtr);
    }



    shuffle(buffer, (fileSize + 1) / 2);
    printf("[Shuffled]:\n");
    for (int i = 0; i < bufferIdx; i++){
        unsigned char* printPtr = (unsigned char*)malloc(9 * sizeof(unsigned char));
        printPtr[0] = 0x00;
        printf("id: %08d byte count: %08d data: %s\n", buffer[i].id, buffer[i].byteCount, strToDecStr(buffer[i].data, printPtr));
        free(printPtr);
    }

    free(buffer);
    fclose(fp);
    return 0;
}
