#include <WinSock2.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "headers/IDEAS_packetEncode.h"
#include "headers/decoder.h"

#define MAX_EVENTS_SECONDS 10000

typedef struct TcpServer TcpServer;
struct TcpServer
{
    SOCKET s;
    struct sockaddr_in server;
};

typedef struct UdpServer UdpServer;
struct UdpServer
{
    SOCKET s;
    struct sockaddr_in server;
};

void startWinsock(WSADATA *wsa)
{
    system("CLS");
    printf("---------------");
    printf("\nInitialising Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), wsa) != 0)
    {
        printf("Failed. Error Code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }

    printf("Initialised.\n");
}

void startTCP(TcpServer *tcp)
{
    // Create a socket
    if (((*tcp).s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create TCP socket : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("TCP socket created.\n");

    (*tcp).server.sin_addr.s_addr = inet_addr("10.10.0.50");
    (*tcp).server.sin_family = AF_INET;
    (*tcp).server.sin_port = htons(50010);

    // Connect to remote server
    if (connect((*tcp).s, (struct sockaddr *)&((*tcp).server), sizeof((*tcp).server)) < 0)
    {
        puts("connect error");
        exit(EXIT_FAILURE);
    }

    puts("TCP Connected");
}

void startUDP(UdpServer *udp)
{
    // Create a socket
    if (((*udp).s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
    {
        printf("Could not create UDP socket : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    printf("UDP socket created.\n");

    (*udp).server.sin_family = AF_INET;
    (*udp).server.sin_addr.s_addr = INADDR_ANY;
    (*udp).server.sin_port = htons(50011);

    // Connect to remote server
    if (bind((*udp).s, (struct sockaddr *)&((*udp).server), sizeof((*udp).server)) == SOCKET_ERROR)
    {
        printf("Bind failed with error code : %d", WSAGetLastError());
        exit(EXIT_FAILURE);
    }
    puts("Bind done");
}

void sendBuffer(TcpServer *tcp, uint8_t *encBuffer, uint16_t bufferLen)
{
    if (send((*tcp).s, encBuffer, bufferLen, 0) < 0)
    {
        puts("Send failed");
        exit(EXIT_FAILURE);
    }
}

uint8_t **listenData(UdpServer *udp, int deltaTime)
{
    int slen, recv_len, i = 0, j;
    struct sockaddr_in si_other;
    uint8_t **dataBuffer = malloc(sizeof(uint8_t *) * MAX_EVENTS_SECONDS * deltaTime);
    time_t start = time(NULL), end;
    do
    {
        // Data for one event
        uint8_t *buffer = (uint8_t *)malloc(sizeof(uint8_t) * 57);

        end = time(NULL);

        slen = sizeof(si_other);
        fflush(stdout);

        // clear the buffer by filling null, it might have previously received data
        memset(buffer, '\0', 57);

        // try to receive some data, this is a blocking call
        if ((recv_len = recvfrom((*udp).s, buffer, 57, 0, (struct sockaddr *)&si_other, &slen)) == SOCKET_ERROR)
        {
            printf("recvfrom() failed with error code : %d", WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        // Affects one event in overall data
        dataBuffer[i] = buffer;
        // if (i == MAX_EVENTS_SECONDS * deltaTime)
        // {
        //     printf("Reached max value\nSome values might be troncated");
        //     break;
        // }

        i++;
    } while (end - start < deltaTime);

    // Add a stop event data when saving into file
    uint8_t endData[57] = {1};
    dataBuffer[i] = endData;
    printf("%02X\t%02X\n", dataBuffer[0][0], dataBuffer[i][1]);
    printf("%i evenements enregistres\n", i);
    return dataBuffer;
}
