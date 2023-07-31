#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "IDEAS_packetEncode.h"
#include "IDEAS_packetDecode.h"
#include "IDEAS_packetTypes.h"
#include "server.h"
#include "ideas_functions.h"
#include "decoder.h"
#include "functions.h"

int main()
{
    WSADATA wsa;
    TcpServer tcp;
    UdpServer udp;

    int deltaTime = 900;

    // Connect TCP & UDP servers
    startWinsock(&wsa);
    startTCP(&tcp);
    startUDP(&udp);

    // Gather datas
    enableExternalADC(&tcp);
    uint8_t **bufferData = listenData(&udp, deltaTime);

    // Close servers and Winsock
    closesocket((&tcp)->s);
    closesocket((&udp)->s);
    WSACleanup();

    // Save data
    FILE *file = NULL;
    file = fopen("adc.csv", "w");
    if (file != NULL)
    {
        saveADCData(file, bufferData, deltaTime);
    }
    fclose(file);

    // END
    exit(EXIT_SUCCESS);
}