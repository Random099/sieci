#include <iostream>
#include <fstream>
#include <cstdint>
#include <cstdio>
#include <vector>
#include <iomanip>
#include <utility>
#include <algorithm>
#include <random>
#include <string>
#include <new>

constexpr uint32_t MAX_PACKET_SIZE{ 3 };
constexpr uint32_t PACKET_COUNT{ 5 };

struct Packet
{
    Packet(uint32_t id, uint32_t size) : id(id), size(size){}
    uint32_t id;
    uint32_t size;
};

int main()
{
    std::vector<unsigned char> data(PACKET_COUNT, 0xFF);
    //std::vector<unsigned char> data{0xF1, 0xF2, 0xF3, 0xF4, 0xF5};

    if(FILE* f = std::fopen("data.bin", "wb"))
    {
        std::fwrite(data.data(), sizeof(unsigned char), data.size(), f);
        std::fclose(f);
    }

    std::vector< std::pair<Packet*, unsigned char*> > buffer;
    if(FILE* f = std::fopen("data.bin", "rb"))
    {
        std::fseek(f, 0, SEEK_END);
        long fileSize = ftell(f);
        std::rewind(f);
        std::cout << "FILE SIZE: " << fileSize << '\n';
        uint32_t packetId = 1;

        for(;;)
        {
            unsigned char* incomingPacket = new unsigned char[2 * sizeof(uint32_t) + (MAX_PACKET_SIZE + 1) * sizeof(unsigned char)];
            Packet* packet = new (incomingPacket) Packet(packetId, MAX_PACKET_SIZE);
            unsigned char* readData = new (incomingPacket + 2 * sizeof(uint32_t)) unsigned char[MAX_PACKET_SIZE + 1];
            if(fread(readData, sizeof(unsigned char) * MAX_PACKET_SIZE, 1, f) != 1)
            {

                delete[] incomingPacket;
                break;
            }
            readData[MAX_PACKET_SIZE] = '\0';
            buffer.push_back(std::make_pair(packet, readData));
            readData = nullptr;
            packet = nullptr;
            ++packetId;
        }

        uint32_t bytesLeft = fileSize % MAX_PACKET_SIZE;
        std::fseek(f, -bytesLeft, SEEK_END);
        //std::cout << "BYTES LEFT" << bytesLeft << '\n';
        if(bytesLeft != 0)
        {
            unsigned char* incomingPacket = new unsigned char[2 * sizeof(uint32_t) + (bytesLeft + 1) * sizeof(unsigned char)];
            Packet* packet = new (incomingPacket) Packet(packetId, bytesLeft);
            unsigned char* readData = new (incomingPacket + 2 * sizeof(uint32_t)) unsigned char[bytesLeft + 1];
            fread(readData, sizeof(unsigned char) * bytesLeft, 1, f);
            readData[bytesLeft] = '\0';
            buffer.push_back(std::make_pair(packet, readData));
            readData = nullptr;
            packet = nullptr;
        }
        std::fclose(f);
    }

    std::cout << "BUFFER SIZE: " << buffer.size() << '\n';
    auto gen = std::default_random_engine {};

    std::shuffle(buffer.begin(), buffer.end(), gen);

    if (FILE* f = std::fopen("reconstructed.bin", "wb"))
    {
        uint32_t currentPacketId = 1;

        for (uint32_t i = 0; i < PACKET_COUNT; ++i)
        {
            auto it = std::find_if(buffer.begin(), buffer.end(), [currentPacketId](const std::pair<Packet*, unsigned char*>& packet) {
                return packet.first->id == currentPacketId;
            });

            if (it != buffer.end())
            {
                std::fwrite(it->second, sizeof(unsigned char), it->first->size, f);
                ++currentPacketId;
            }
        }
        std::fclose(f);
    }

    for(int j = 0; j < buffer.size(); ++j)
    {
        std::cout << "PACKET" << std::setw(std::to_string(PACKET_COUNT).length()) << std::left << buffer.data()[j].first->id << " ";
        std::cout << std::setw(11) << std::left << &buffer.data()[j].first->id << " ";
        std::cout << std::setw(11) << std::left << &buffer.data()[j].first->size << " ";
        for(int i = 0; buffer.data()[j].second[i] != '\0'; ++i)
        {
                std::cout << std::setw(10) << std::left <<  static_cast<void*>(&buffer.data()[j].second[i]) << " ";
        }
        std::cout << '\n';
    }

    for(uint32_t i = 0; i < buffer.size(); ++i){
        buffer[i].first->~Packet();
        ::operator delete[] (buffer[i].first);
    }
    buffer.clear();

    return 0;
}
