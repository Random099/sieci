#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>

#pragma pack 1
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#define CHUNK_SIZE 2 //Bytes of data per packet

struct Packet{
    uint32_t id;
    uint32_t byteCount;
    unsigned char data[CHUNK_SIZE + 1];
};

void shuffle(struct Packet* array, size_t n) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int usec = tv.tv_usec;
    srand(usec);

    if (n > 1) {
        for (size_t i = n - 1; i > 0; i--) {
            size_t j = (unsigned int) (rand()%i);
            struct Packet t = array[j];
            array[j] = array[i];
            array[i] = t;
        }
    }
}

void reconstructFile(struct Packet* buffer, size_t bufferSize){
    FILE* writeFile = fopen("transmitted_data.bin", "wb");
    struct Packet* readPackets = (struct Packet*)malloc(sizeof(struct Packet) * bufferSize);
    int readPacketCount = 0;
    uint32_t currentPacketIdx = 1;
    size_t j = 0;
    
    while(currentPacketIdx <= bufferSize){
        if(j < bufferSize){
            readPackets[j] = buffer[j];
            readPackets[j].id = buffer[j].id;
            readPackets[j].byteCount = buffer[j].byteCount;
            strncpy(readPackets[j].data, buffer[j].data, buffer[j].byteCount);
            readPackets[j].data[2] = 0x00;
            ++j;
        }
        for(int i = 0; i < j; ++i){
            if(readPackets[i].id == currentPacketIdx){
                fwrite(readPackets[i].data, sizeof(readPackets[i].data)-1, 1, writeFile);
                ++currentPacketIdx;
            }
        }
    }
    free(readPackets);
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
        unsigned char data[] = {0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        printf("%i", sizeof(data));
        fwrite(data, sizeof(data), 1, dataFile);
        fclose(dataFile);
    }
    FILE* fp = fopen("test.bin", "rb");
    struct Packet readPacket;

    //Calculate file size and move pointer to beginning
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);
    printf("Filesize: %i\n", fileSize);

    //Allocate buffer
    if(CHUNK_SIZE > 1 && (fileSize % CHUNK_SIZE != 0))
        fileSize = fileSize + CHUNK_SIZE;
    struct Packet* buffer = (struct Packet*)malloc(sizeof(struct Packet) * (fileSize / CHUNK_SIZE));

    //Read file into buffer of desired form packets
    size_t bufferIdx = 0;
    for(; (fread(&readPacket.data, CHUNK_SIZE, 1, fp) == 1); ++bufferIdx){
        readPacket.id = bufferIdx+1;
        readPacket.byteCount = CHUNK_SIZE;
        buffer[bufferIdx] = readPacket;
        buffer[bufferIdx].data[CHUNK_SIZE] = 0x00;
    }

    //Check for remaining bytes
    if(CHUNK_SIZE > 1 && (fileSize % CHUNK_SIZE != 0)){
        size_t bytesLeft = (fileSize - CHUNK_SIZE) - (bufferIdx * CHUNK_SIZE);
        if(bytesLeft > 0){
            fread(&readPacket.data, bytesLeft, 1, fp);
            readPacket.data[bytesLeft] = 0x00;
            readPacket.id = bufferIdx+1;
            readPacket.byteCount = bytesLeft;
            buffer[bufferIdx] = readPacket;
            bufferIdx++;
        }
    }

    printf("[Original]:\n");
    for (size_t i = 0; i < bufferIdx; i++){
        unsigned char* printPtr = (unsigned char*)malloc( ((4 * CHUNK_SIZE) + 1) * sizeof(unsigned char));
        printPtr[0] = 0x00;
        printf("id: %p byte count: %p\ndata: ", &buffer[i].id, &buffer[i].byteCount);
        for(size_t j = 0; j < CHUNK_SIZE; ++j)
            printf("%p ", &buffer[i].data[j]);
        free(printPtr);
        printf("\n");
    }

    shuffle(buffer, fileSize / CHUNK_SIZE);
    printf("[Shuffled]:\n");
    for (size_t i = 0; i < bufferIdx; i++){
        unsigned char* printPtr = (unsigned char*)malloc( ((4 * CHUNK_SIZE) + 1) * sizeof(unsigned char));
        printPtr[0] = 0x00;
        printf("id: %08d byte count: %08d data: %s\n", buffer[i].id, buffer[i].byteCount, strToDecStr(buffer[i].data, printPtr));
        free(printPtr);
    }

    reconstructFile(buffer, fileSize / CHUNK_SIZE);
    free(buffer);
    fclose(fp);
    return 0;
}
