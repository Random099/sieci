#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

struct Packet{
    uint32_t id;
    uint32_t byteCount;
    unsigned char data[3];
};

unsigned char* strToDecStr(unsigned char* string, unsigned char* dst){
    for(int i = 0; i < strlen(string); i++){
        unsigned char* temp = malloc(sizeof(unsigned char)*2);
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
        unsigned char data[] = {0xFF, 0xFF, 0xFF};
        fwrite(data, sizeof(data), 1, dataFile);
        fclose(dataFile);
    }
    FILE* fp = fopen("test.bin", "rb");
    struct Packet readPacket;
    
    //Calculate file size and return to beginning
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    //Allocate buffer
    struct Packet* buffer = malloc((fileSize + 1) / 2);

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
        readPacket.id = bufferIdx+1;
        readPacket.byteCount = bytesLeft;
        readPacket.data[bytesLeft] = 0x00;
        buffer[bufferIdx] = readPacket;
        bufferIdx++; 
    }

    unsigned char* printPtr = malloc(sizeof(readPacket.data));
    printPtr[0] = '\0';
    for (int i = 0; i < bufferIdx; i++){
        printf("id: %08d byte count: %08d data: %s\n", buffer[i].id, buffer[i].byteCount, strToDecStr(buffer[i].data, printPtr));
        memset(printPtr, 0, strlen(printPtr));
    }

    free(printPtr);
    free(buffer);
    fclose(fp);
    return 0;
}
