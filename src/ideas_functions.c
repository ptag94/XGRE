#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <time.h>

#include "headers/IDEAS_packetEncode.h"
#include "headers/server.h"

#define WRITE_SYS 0
#define READ_SYS 1
#define WRITE_ASIC 2
#define READ_ASIC 3
#define WRITE_READ_ASIC 4

void define_buffer(int choice, uint8_t **encBuffer, uint16_t *bufferLen, uint16_t regLen)
{
    switch (choice)
    {
    case WRITE_SYS:
        *bufferLen = IDEAS_calcLength_writeSystemRegister(regLen);
        break;
    case READ_SYS:
        *bufferLen = IDEAS_calcLength_readSystemRegister(regLen);
        break;
    case WRITE_ASIC:
        *bufferLen = IDEAS_calcLength_asicSpiRegisterWrite(regLen);
        break;
    case READ_ASIC:
        *bufferLen = IDEAS_calcLength_asicSpiRegisterRead(regLen);
        break;
    case WRITE_READ_ASIC:
        *bufferLen = IDEAS_calcLength_asicConfig(regLen);
        break;
    default:
        printf("Wrong choice input\n");
    }

    *encBuffer = (uint8_t *)malloc(sizeof(uint8_t) * *bufferLen);
}

void writeSys(TcpServer *tcp, uint16_t regAddr, uint8_t *regData, uint16_t regLen)
{
    uint8_t *encBuffer;
    uint16_t bufferLen;

    define_buffer(WRITE_SYS, &encBuffer, &bufferLen, regLen);
    IDEAS_encode_writeSystemRegister(
        0,
        0,
        0,

        regAddr,
        regLen,
        regData,
        encBuffer);

    sendBuffer(tcp, encBuffer, bufferLen);
}

void readSys(TcpServer *tcp, uint16_t regAddr)
{
    uint8_t *encBuffer;
    uint16_t bufferLen;
    uint16_t regLen = 0x2;

    define_buffer(READ_SYS, &encBuffer, &bufferLen, regLen);
    IDEAS_encode_readSystemRegister(
        0,
        0,
        0,

        regAddr,
        encBuffer);
    sendBuffer(tcp, encBuffer, bufferLen);
}

void writeAsicSpi(TcpServer *tcp, uint16_t regAddr, uint8_t *regData)
{
    int i;
    uint8_t *encBuffer;
    uint16_t bufferLen;
    uint16_t regLen = 0x14;

    define_buffer(WRITE_ASIC, &encBuffer, &bufferLen, regLen);
    IDEAS_encode_asicSpiRegisterWrite(
        0,
        0,
        0,

        0,
        0x01,
        regAddr,
        regLen,
        regData,
        encBuffer);

    printf("Configuration envoyee: \b{");
    for (i = 0; i < 19; i++)
    {
        printf("%02X ", encBuffer[i]);
    }
    printf("\b}\n");
    sendBuffer(tcp, encBuffer, bufferLen);
}

int readbackAsicSpi(TcpServer *tcp, uint8_t *serverReply)
{
    int i, recvSize;
    uint16_t regLen = 0x14;
    uint8_t respBufferLen = IDEAS_calcLength_asicSpiRegisterReadback(regLen);

    if ((recvSize = recv((*tcp).s, serverReply, 19, 0)) == SOCKET_ERROR)
    {
        puts("recv failed");
    }
    printf("Donnees recues:  %i\n", recvSize);
    printf("\b{");
    for (i = 0; i < respBufferLen; i++)
    {
        printf("%02X ", serverReply[i]);
    }
    printf("\b}\n");

    return recvSize;
}

void readAsicSpi(TcpServer *tcp, uint16_t regAddr)
{
    uint8_t *encBuffer;
    uint16_t bufferLen;
    uint16_t regLen = 0x14;
    
    define_buffer(READ_ASIC, &encBuffer, &bufferLen, regLen);
    IDEAS_encode_asicSpiRegisterRead(
        0,
        0,
        0,

        0,
        0x01,
        regAddr,
        regLen,
        encBuffer);
    int i;
    printf("Lecture de l'adresse 0x%02X : \b{", regAddr);
    for (i = 0; i < bufferLen; i++)
    {
        printf("%02X ", encBuffer[i]);
    }
    printf("\b}\n");
    sendBuffer(tcp, encBuffer, bufferLen);
}

int readbackSys(TcpServer *tcp, uint8_t *serverReply, uint16_t regLen)
{
    int i, recvSize;
    uint8_t respBufferLen = IDEAS_calcLength_systemRegisterReadback(regLen);

    if ((recvSize = recv((*tcp).s, serverReply, IDEAS_PACKET_HEADER_LENGTH + 3 + (int)regLen, 0)) == SOCKET_ERROR)
    {
        puts("recv failed");
    }

    return recvSize;
}