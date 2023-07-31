#ifndef SERVER_H
#define SERVER_H

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

/**
 * @brief Initialize Winsock
 *
 * @param wsa
 */
void startWinsock(WSADATA *wsa);

/**
 * @brief Start TCP socket and connect to 10.10.0.50:50010
 *
 * @param tcp TcpServer structure object
 */
void startTCP(TcpServer *tcp);

/**
 * @brief Start UDP socket and connect to
 *
 * @param udp UdpServer structure object
 */
void startUDP(UdpServer *udp);

/**
 * @brief Send buffer data throught TCP socket
 *
 * @param tcp       Tcp socket
 * @param encBuffer Buffer containing data to send
 * @param bufferLen Len of buffer
 */
void sendBuffer(TcpServer *tcp, uint8_t *encBuffer, uint16_t bufferLen);

/**
 * @brief Listen data throught UDP for a certain amount of time.
 * Maximum registrable events is set with MAX_EVENTS_SECONDS and deltaTime.
 *
 * @param udp       UdpServer object
 * @param deltaTime Time to listen data
 * @return uint8_t** 2D array with rows for events and columns for channels
 */
uint8_t **listenData(UdpServer *udp, int deltaTime);

#endif