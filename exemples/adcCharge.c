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

#define DELTA_VOLT 0.1
#define MEAN_TIME 1
#define MAX_VOLT 4.2
#define MIN_VOLT 0.1

int main()
{
    WSADATA wsa;
    TcpServer tcp;
    UdpServer udp;

    // Connect TCP & UDP servers
    startWinsock(&wsa);
    startTCP(&tcp);
    startUDP(&udp);

    enableExternalADC(&tcp);
    int **data = adcFunctionCharge(&udp, MAX_VOLT, MIN_VOLT, DELTA_VOLT, MEAN_TIME);

    // Close servers
    closesocket((&tcp)->s);
    closesocket((&udp)->s);
    WSACleanup();

    // Save data
    FILE *file = NULL;
    file = fopen("adcCharge.csv", "w");
    if (file != NULL)
    {
        saveADCChargeData(file, data, MAX_VOLT, MIN_VOLT, DELTA_VOLT);
    }
    fclose(file);

    // END
    return 0;
}