#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <bitset>
#include <limits>
#include <cstring>
#include <random>
#include <tuple>
#include <array>

std::tuple<uint64_t, std::string> parityBit(const std::string READ_FILENAME, const std::vector<uint8_t>& input){
    std::string writeFileName {"ParityOutput_"+READ_FILENAME};
    std::ofstream writeFile(writeFileName, std::ios::binary | std::ios::out);
    uint64_t bitCount{0};
    if(writeFile.is_open()){
        for(const auto& byte : input){
            std::bitset<8> bin(byte);
            if(bin.count() % 2 != 0) //even parity
                bin[7] = true;
            bitCount += bin.count();
            writeFile << byte;
        }
        writeFile << std::bitset<64>(bitCount%8);
        writeFile.close();
    }
    return {bitCount % 8, writeFileName};
}

std::tuple<uint64_t, std::string> moduloSum(const std::string READ_FILENAME, const uint64_t modDividend, const std::vector<uint8_t>& input){
    std::string writeFileName {"ModuloSum_"+READ_FILENAME};
    std::ofstream writeFile(writeFileName, std::ios::binary | std::ios::out);
    uint64_t sum{0};
    if(writeFile.is_open()){
        for(const auto& byte : input){
            writeFile << byte;
            sum += byte;
        }
        writeFile << std::bitset<64>(sum % modDividend);
        writeFile.close();
    }
    return {sum % modDividend, writeFileName};
}

template<const int polyLength>
std::tuple<uint64_t, std::string> crcUniversal(const std::string READ_FILENAME, const char POLYNOMIAL[], const std::vector<uint8_t>& input){
    std::string writeFileName {"CRC_"+std::to_string(polyLength)+"_"+READ_FILENAME};
    std::ofstream writeFile("CRC_"+std::to_string(polyLength)+"_"+READ_FILENAME, std::ios::binary | std::ios::out);
    std::bitset<polyLength> gen(POLYNOMIAL);
    std::bitset<polyLength> buffer;
    std::vector<bool> msg;
    if(writeFile.is_open()){
        for(const auto& byte : input){
            std::bitset<8> bin(byte);
            writeFile << byte;
            for(int i = 7; i >= 0; i--)
                msg.push_back(bin[i]);
        }
        for(int i = 0; i < polyLength - 1; ++i)
                msg.push_back(false);
        for(auto i = 0; i < msg.size(); ++i){
            bool msbState = buffer[polyLength - 1];
            buffer <<= 1;
            buffer[0] = msg[i];
            if(msbState)
                buffer ^= gen;
            msbState = false;
        }
        writeFile << std::bitset<64>(buffer.to_string());
    }
    return {buffer.to_ullong(), writeFileName};
}

void readFile(const std::string FILENAME, std::vector<uint8_t>& v){
    std::ifstream readFile(FILENAME, std::ios::binary | std::ios::in);
    uint8_t byte;
    if(readFile.is_open()){
        readFile >> std::noskipws;
        while(readFile >> byte)
            v.push_back(byte);
        readFile.close();
    }
}

void genErrors(const std::string READ_FILENAME, const double PROBABILITY){
    std::ifstream readFile(READ_FILENAME, std::ios::binary | std::ios::in);
    std::ofstream writeFile("DAMAGED_"+READ_FILENAME, std::ios::binary | std::ios::out);
    uint8_t inByte;
    std::mt19937 gen;
    const double RANGE {100/PROBABILITY};
    std::uniform_int_distribution<> dist(0, RANGE);
    if(readFile.is_open() && writeFile.is_open()){
        readFile >> std::noskipws;
        while(readFile >> inByte){
            std::bitset<8> bin(inByte);
            for(int i = 0; i < 8; i++){
                if(dist(gen) == RANGE/2)
                    bin[i] = !bin[i];
            }
            writeFile << (uint8_t)bin.to_ullong();
        }
        writeFile.close();
        readFile.close();
    }
}

void genErrorsR(const std::string READ_FILENAME, const double PROBABILITY){
    std::ifstream readFile(READ_FILENAME, std::ios::binary | std::ios::in);
    std::ofstream writeFile("DAMAGED_"+READ_FILENAME, std::ios::binary | std::ios::out);
    uint8_t inByte;
    std::vector<std::bitset<8>> msg;
    std::mt19937 gen;
    if(readFile.is_open() && writeFile.is_open()){
        readFile >> std::noskipws;
        while(readFile >> inByte)
            msg.push_back(inByte);
        std::uniform_int_distribution<> distId(0, msg.size()-1);
        std::uniform_int_distribution<> distBit(0, 7);
        for(int i = 0; i < (msg.size()*8)*(PROBABILITY/100); ++i)
            msg[distId(gen)][distBit(gen)] = !msg[distId(gen)][distBit(gen)];
        for(const auto& byte : msg)
            writeFile << (uint8_t)byte.to_ullong();
        writeFile.close();
        readFile.close();
    }
}

int main(){
    std::vector<uint8_t> input;
    const std::string READ_FILENAME{"test.txt"};
    constexpr char CRC_POLYNOMIAL[] {"10011"};
    constexpr int CRC_POLYNOMIAL_LENGTH{std::char_traits<char>::length(CRC_POLYNOMIAL)};
    constexpr double ERROR_PROBABILITY{0.1}; //Percentage
    readFile(READ_FILENAME, input);

    std::vector<std::tuple<uint64_t, std::string>> output; //Initial checksum results
    //output.push_back(parityBit(READ_FILENAME, input));
    //output.push_back(moduloSum(READ_FILENAME, 128ULL, input));
    output.push_back(crcUniversal<CRC_POLYNOMIAL_LENGTH>(READ_FILENAME, CRC_POLYNOMIAL, input));
    //std::cout << "Bit count: " << std::get<0>(output[0]) << '\n';
    //std::cout << "Checksum: " << std::get<0>(output[1]) << '\n';
    std::cout << "Crc remainder: " << std::get<0>(output[0]) << '\n';

    for(const auto& tup : output)
        genErrorsR(std::get<1>(tup), ERROR_PROBABILITY);
    //std::array<std::vector<uint8_t>, 1> errIn;
    std::vector<uint8_t> errIn;
    readFile("DAMAGED_"+std::get<1>(output[0]), errIn);
    errIn.erase(errIn.end()-64, errIn.end()); //Truncate checksum
    /*
    for(int i = 0; i < output.size(); ++i){
        std::cout << "TEST" << '\n';
        readFile("DAMAGED_"+std::get<1>(output[i]), errIn[i]);
        errIn[i].erase(errIn[i].end()-64, errIn[i].end()); //Truncate checksum
    }
    */
    std::vector<std::tuple<uint64_t, std::string>> errOutput; //Checksum results with errors
    //errOutput.push_back(parityBit("DAMAGED_"+std::get<1>(output[0]), errIn[0]));
    //errOutput.push_back(moduloSum("DAMAGED_"+std::get<1>(output[1]), 128ULL, errIn[1]));
    errOutput.push_back(crcUniversal<CRC_POLYNOMIAL_LENGTH>("DAMAGED_"+std::get<1>(output[0]), CRC_POLYNOMIAL, errIn));
    std::cout << "After generating errors: " << '\n';
    //std::cout << "Bit count: " << std::get<0>(errOutput[0]) << '\n';
    //std::cout << "Checksum: " << std::get<0>(errOutput[1]) << '\n';
    std::cout << "Crc remainder: " << std::get<0>(errOutput[0]) << '\n';
}
