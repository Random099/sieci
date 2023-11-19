#include <stdio.h>
#include <stdlib.h>

#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))

struct Packet{
    int id;
    unsigned char data[3];
};

int main(){
    FILE* fp = fopen("test.txt", "rb");
    struct Packet readPacket;
    
    //Calculate file size and return to beginning
    fseek(fp, 0L, SEEK_END);
    long fileSize = ftell(fp);
    rewind(fp);

    //Allocate buffer
    struct Packet* buffer = malloc((fileSize + 1) / 2);

    //Read file into buffer of desired form packets
    int bufferIdx = 0;
    for(; (fread(&readPacket.data, sizeof(readPacket.data) - 1, 1, fp) == 1); ++bufferIdx){
        readPacket.id = bufferIdx;
        buffer[bufferIdx] = readPacket; 
        buffer[bufferIdx].data[ARRAYSIZE(readPacket.data) - 1] = 0x00;
    }
    
    //Check for remaining bytes
    int bytesLeft = fileSize - (bufferIdx * 2);
    if(bytesLeft != 0){
        fread(&readPacket.data, bytesLeft, 1, fp);
        readPacket.id = bufferIdx;
        readPacket.data[bytesLeft] = 0x00;
        buffer[bufferIdx] = readPacket;
        bufferIdx++; 
    }

    for (int i = 0; i < bufferIdx; i++)
        printf("%d, %s\n", buffer[i].id, buffer[i].data);
    free(buffer);
    fclose(fp);
    return 0;
}
