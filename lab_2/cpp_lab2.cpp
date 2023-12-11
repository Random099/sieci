#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <iomanip>

constexpr uint32_t MAX_PACKET_SIZE = 3;

struct Packet
{
    Packet(uint32_t id, uint32_t size) : id(id), size(size){}
    uint32_t id;
    uint32_t size;
};

int main()
{
    //std::vector<unsigned char> data(5, 0xFF);
    std::vector<unsigned char> data{0xF1, 0xF2, 0xF3, 0xF4, 0xF5};

    if(FILE* f = fopen("data.bin", "wb"))
    {
        fwrite(data.data(), sizeof(unsigned char), data.size(), f);
        fclose(f);
    }
    std::vector<Packet*> packets;
    std::vector<unsigned char*> buffer;
    if(FILE* f = fopen("data.bin", "rb"))
    {
        std::fseek(f, 0, SEEK_END);
        long fileSize = ftell(f);
        std::cout << "FILE SIZE: " << fileSize << '\n';
        std::rewind(f);
        long packetId = 1;
        for(;;)
        {
            unsigned char* incomingPacket = new unsigned char[2 * sizeof(uint32_t) + (MAX_PACKET_SIZE + 1) * sizeof(unsigned char)];
            Packet* packet = new (incomingPacket) Packet(packetId, MAX_PACKET_SIZE);
            packets.push_back(packet);
            unsigned char* readData = new (incomingPacket + 2 * sizeof(uint32_t)) unsigned char[MAX_PACKET_SIZE + 1];
            if(fread(readData, sizeof(unsigned char) * MAX_PACKET_SIZE, 1, f) != 1)
            {

                delete[] incomingPacket;
                packets.erase(packets.begin() + packets.size() - 1);
                break;
            }
            readData[MAX_PACKET_SIZE] = '\0';
            buffer.push_back(readData);
            readData = nullptr;
            ++packetId;
        }
        uint32_t bytesLeft = fileSize % MAX_PACKET_SIZE;
        std::fseek(f, -bytesLeft, SEEK_END);
        //std::cout << "BYTES LEFT" << bytesLeft << '\n';
        if(bytesLeft != 0)
        {
            unsigned char* incomingPacket = new unsigned char[2 * sizeof(uint32_t) + (bytesLeft + 1) * sizeof(unsigned char)];
            Packet* packet = new (incomingPacket) Packet(packetId, bytesLeft);
            packets.push_back(packet);
            unsigned char* readData = new (incomingPacket + 2 * sizeof(uint32_t)) unsigned char[bytesLeft + 1];
            fread(readData, sizeof(unsigned char) * bytesLeft, 1, f);
            readData[bytesLeft] = '\0';
            buffer.push_back(readData);
            readData = nullptr;
        }
    }

    std::cout << "BUFFER SIZE: " << buffer.size() << '\n';
    for(int j = 0; j < buffer.size(); ++j)
    {
        std::cout << "PACKET" << j+1 << " ";
        //std::cout << std::setfill('0') << std::setw(8) << &packets[j]->id << " ";
        //std::cout << std::setfill('0') << std::setw(8) << &packets[j]->size << " ";
        std::cout << &packets[j]->id << " ";
        std::cout << &packets[j]->size << " ";
        for(int i = 0; buffer.data()[j][i] != '\0'; ++i)
        {
            std::cout << static_cast<void*>(&buffer.data()[j][i])  << " ";
        }
        std::cout << '\n';
    }


    return 0;
}
