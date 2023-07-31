#include <WinSock2.h>
#include <stdio.h>

#include "IDEAS_packetEncode.h"
#include "IDEAS_packetDecode.h"
#include "IDEAS_packetTypes.h"
#include "server.h"
#include "ideas_functions.h"
#include "decoder.h"
#include "functions.h"

#define CHANNEL 1
#define MIN_DAC 0
#define MAX_DAC 150
#define STEP 1
#define BOOL_ALL_COMP 1
#define COMPARATOR 1

int main()
{
    FILE *file = NULL;

    WSADATA wsa;
    TcpServer tcp;
    UdpServer udp;

    int **data;

    data = defineData(MIN_DAC, MAX_DAC, STEP);

    // Connect TCP & UDP servers
    startWinsock(&wsa);
    startTCP(&tcp);
    startUDP(&udp);

    // Load json configuration
    // loadConfiguration(&tcp);

    startCounter(&tcp);
    resetCount(&tcp);

    resetComp(&tcp);
    resetThreshold(&tcp);

    // Start data
    scurve(&tcp, data, CHANNEL, MIN_DAC, MAX_DAC, STEP, BOOL_ALL_COMP, COMPARATOR);

    // Save data
    file = fopen("oneComparator.csv", "w");
    saveData(file, data, MIN_DAC, MAX_DAC, STEP, COMPARATOR);
    fclose(file);

    // Close servers
    closesocket((&tcp)->s);
    closesocket((&udp)->s);
    WSACleanup();

    // END
    return 0;
}