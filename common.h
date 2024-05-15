#ifndef _COMMON_H_
#define _COMMON_H_

#include <vector>
#include <stdint.h>
#include <string>
#include <stdio.h>

bool toInt(const char *str, int &result);
bool toUint16(const char *str, uint16_t &result);
bool toIntRange(const char *str, int data_size, int &start, int &end);

bool readLine(std::vector<char> &line);

bool signalIgnoring();

enum class IOStatus {
    OK,
    ERR,
    EOD,  // End Of Data
};

IOStatus myRead(int fd, void *_buf, size_t nbyte);
IOStatus myWrite(int fd, const void *_buf, size_t nbyte);

#endif 
