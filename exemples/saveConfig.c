#include <WinSock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "IDEAS_packetEncode.h"
#include "IDEAS_packetDecode.h"
#include "IDEAS_packetTypes.h"
#include "cJSON.h"
#include "server.h"
#include "ideas_functions.h"
#include "decoder.h"
#include "functions.h"

int main()
{
    WSADATA wsa;
    TcpServer tcp;
    UdpServer udp;

    // Connect TCP & UDP servers
    startWinsock(&wsa);
    startTCP(&tcp);
    startUDP(&udp);

    cJSON *root;
    root = cJSON_CreateObject();
    readSystemConfiguration(&tcp, root);
    readAsicConfiguration(&tcp, root);

    writeJSON(root);

    // Close servers
    closesocket((&tcp)->s);
    closesocket((&udp)->s);
    WSACleanup();

    // END
    return 0;
}