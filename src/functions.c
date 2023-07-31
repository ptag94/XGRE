#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <winsock2.h>
#include <math.h>
#include <time.h>
#include <windows.h>

#include "headers/cJSON.h"
#include "headers/ideas_functions.h"

#define BUFF_LEN 250000
#define MAX_EVENTS_SECONDS 10000
#define NB_CHANNELS 17
#define NB_COMP 4
#define NB_THRESHOLD 1023
#define BYTES_LEN 10
#define READBACK_SYS_DATA 13

char *loadJSON(FILE *file)
{
    static char buffer[BUFF_LEN];

    // open the file
    file = fopen("asic.json", "r");
    if (file == NULL)
    {
        printf("Error: Unable to open the file.\n");
    }

    // read the file contents into a string
    int len = fread(buffer, 1, sizeof(buffer), file);
    fclose(file);

    return buffer;
}

int parseJSON(char *buffer)
{
    cJSON *json = cJSON_Parse(buffer);
    if (json == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Parsing Error: %s\n", error_ptr);
        }
        cJSON_Delete(json);
        return 1;
    }
    return 0;
}

void intToHexList(uint8_t *regData, int var)
{
    int n = var;
    unsigned char hexBuffer[3] = {0};

    memcpy((char *)hexBuffer, (char *)&n, sizeof(int));

    regData[0] = (uint8_t)hexBuffer[2];
    regData[1] = (uint8_t)hexBuffer[1];
    regData[2] = (uint8_t)hexBuffer[0];
}

void intToBibaryList(int in, int count, int *out)
{
    unsigned int mask = 1U << (count - 1);
    int i;
    for (i = 0; i < count; i++)
    {
        out[i] = (in & mask) ? 1 : 0;
        in <<= 1;
    }
}

double hexListToInt(uint8_t *hexList, int range)
{
    int i;
    double sum = 0;
    for (i = 0; i < range; i++)
    {
        sum += (double)hexList[i] * pow(2, 8 * (range - 1 - i));
    }

    return sum;
}

void hexListToBinaryList(uint8_t *dataList, int *intList)
{
    uint8_t i;
    int j;
    int counter = 0;
    uint8_t a;

    for (j = 0; j < 3; j++)
    {
        a = dataList[j];
        for (i = 0x80; i != 0; i >>= 1)
        {
            intList[counter] = ((a & i) ? 1 : 0);
            counter += 1;
        }
    }
}

int binaryToInt(int *bList, int bLen)
{
    int i;
    int sum = 0;

    for (i = 0; i < bLen; i++)
    {
        sum += bList[i] * pow(2, bLen - (i + 1));
    }

    return sum;
}

void iterateJSON(TcpServer *tcp, cJSON *blocks)
{
    cJSON *block, *registers, *reg;
    char *blockString;
    uint16_t spi = 0x044;
    uint8_t regData[3] = {0};
    int spiVal, i;

    int recvSize;
    uint8_t serverReply[20];

    while (blocks)
    {
        // Get block
        block = blocks->child;
        blockString = block->valuestring;

        // Match block name
        if (strcmp(blockString, "ASIC_Configuration") == 0)
        {
            registers = block->next->child;
            while (registers)
            {
                reg = registers->child;
                spiVal = reg->next->valueint;
                intToHexList(regData, spiVal);

                writeAsicSpi(tcp, spi, regData);
                if (recvSize = readbackSys(tcp, serverReply, 1) <= 0)
                {
                    printf("Readout error\n");
                }

                // Next register
                registers = registers->next;
                spi += 0x1;
            }
        }

        // Next block
        blocks = blocks->next;
    }
    // for (i = 0; i < (int)(spi - 0x044); i++)
    // {
    //     recvSize = readbackAsicSpi(tcp, serverReply);
    // }
}

void loadConfiguration(TcpServer *tcp)
{
    FILE *file = NULL;
    char *buffer;

    printf("Load configuration\n");

    buffer = loadJSON(file);

    cJSON *json = cJSON_Parse(buffer);

    cJSON *deviceData = cJSON_GetObjectItem(json, "consolidated_model");
    if (deviceData)
    {
        // Select only one asic "IDE3381" -> "blocks"
        cJSON *blocks = deviceData->child->child->next->child;
        iterateJSON(tcp, blocks);
    }
}

void startCounter(TcpServer *tcp)
{
    uint8_t regData[1] = {0x1};
    uint16_t regAddr = 0x0B08;
    uint16_t regLen = 0x1;
    int recvSize;

    uint8_t serverReply[20];

    // Check if Counters are already enable
    readSys(tcp, regAddr);
    if (readbackSys(tcp, serverReply, 1) <= 0)
    {
        printf("Readout error\n");
        exit(EXIT_FAILURE);
    }
    printf("Activation des compteurs\n");
    printf("readback: ");
    int i;
    for (i = 0; i < 14; i++)
    {
        printf("%02X ", serverReply[i]);
    }
    printf("\n");
    // If not, enable it
    if (serverReply[READBACK_SYS_DATA] == 0x0)
    {
        writeSys(tcp, regAddr, regData, regLen);
        if (readbackSys(tcp, serverReply, 1) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
    }
}

// void changeSpiValue(TcpServer *tcp, uint16_t regAddr, int data, int len, int pos)
// {
//     int i, recvSize, *bin, asicData[24], intData;
//     uint8_t serverReply[20], replayData[3] = {0}, regData[3] = {0};

//     readAsicSpi(tcp, regAddr);
//     if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//     {
//         printf("Readout error\n");
//     }
//     for (i = 0; i < 4; i++)
//     {
//         replayData[i] = serverReply[16 + i];
//     }

//     hexListToBinaryList(replayData, asicData);

//     bin = (int *)malloc(sizeof(int) * len);
//     intToBibaryList(data, len, bin);
//     for (i = 0; i < len; i++)
//     {
//         asicData[(20 - pos - len) + 4 + i] = bin[i];
//     }
//     intData = binaryToInt(asicData, 24);
//     intToHexList(regData, intData);

//     writeAsicSpi(tcp, regAddr, regData);
//     if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//     {
//         printf("Readout error\n");
//     }
// }

void changeSpiValue(TcpServer *tcp, uint16_t regAddr, int data, int len, int pos)
{
    int i, recvSize, *bin, asicData[24], intData;
    uint8_t serverReply[20], replayData[3] = {0}, regData[3] = {0};

    readAsicSpi(tcp, regAddr);
    if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
    {
        printf("Readout error\n");
    }
    hexListToBinaryList(serverReply + 16, asicData);

    bin = (int *)malloc(sizeof(int) * len);
    intToBibaryList(data, len, bin);
    for (i = 0; i < len; i++)
    {
        asicData[(20 - pos - len) + 4 + i] = bin[i];
    }
    intData = binaryToInt(asicData, 24);
    intToHexList(regData, intData);

    writeAsicSpi(tcp, regAddr, regData);
    if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
    {
        printf("Readout error\n");
    }
}

// void enableComp(TcpServer *tcp, int comp)
// {
//     uint16_t regAddr;
//     uint8_t regData[3] = {0};
//     int spiVal;

//     int recvSize;
//     uint8_t serverReply[20];

//     switch (comp)
//     {
//     case 1:
//         regAddr = 0x47;
//         spiVal = 1048568;
//         intToHexList(regData, spiVal);
//         writeAsicSpi(tcp, regAddr, regData);

//         if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//         {
//             printf("Readout error\n");
//         }
//         break;
//     case 2:
//         regAddr = 0x48;
//         spiVal = 131071;
//         intToHexList(regData, spiVal);
//         writeAsicSpi(tcp, regAddr, regData);

//         if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//         {
//             printf("Readout error\n");
//         }
//         break;
//     case 3:
//         regAddr = 0x48;
//         spiVal = 917504;
//         intToHexList(regData, spiVal);
//         writeAsicSpi(tcp, regAddr, regData);

//         if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//         {
//             printf("Readout error\n");
//         }

//         regAddr = 0x49;
//         spiVal = 16383;
//         intToHexList(regData, spiVal);
//         writeAsicSpi(tcp, regAddr, regData);

//         if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//         {
//             printf("Readout error\n");
//         }
//         break;
//     case 4:
//         regAddr = 0x49;
//         spiVal = 1032192;
//         intToHexList(regData, spiVal);
//         writeAsicSpi(tcp, regAddr, regData);

//         if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//         {
//             printf("Readout error\n");
//         }

//         regAddr = 0x4A;
//         spiVal = 2047;
//         intToHexList(regData, spiVal);
//         writeAsicSpi(tcp, regAddr, regData);

//         if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//         {
//             printf("Readout error\n");
//         }
//         break;

//     default:
//         break;
//     }
// }

void enableComp(TcpServer *tcp, int comp)
{
    uint16_t regAddr;
    int spiValue = 131071;

    switch (comp)
    {
    case 1:
        regAddr = 0x47;
        changeSpiValue(tcp, regAddr, spiValue, 17, 3);
        break;

    case 2:
        regAddr = 0x48;
        changeSpiValue(tcp, regAddr, spiValue, 17, 0);
        break;
    case 3:
        regAddr = 0x48;
        spiValue = 7;
        changeSpiValue(tcp, regAddr, spiValue, 3, 17);

        regAddr = 0x49;
        spiValue = 16383;
        changeSpiValue(tcp, regAddr, spiValue, 14, 0);
        break;
    case 4:
        regAddr = 0x49;
        spiValue = 63;
        changeSpiValue(tcp, regAddr, spiValue, 6, 14);

        regAddr = 0x4A;
        spiValue = 2047;
        changeSpiValue(tcp, regAddr, spiValue, 11, 0);
        break;

    default:
        break;
    }
}

void resetComp(TcpServer *tcp)
{
    int i;
    uint16_t regAddr[4] = {0x47, 0x48, 0x49, 0x4A};
    uint8_t regData[3] = {0x0, 0x0, 0x0};

    int recvSize;
    uint8_t serverReply[20];

    for (i = 0; i < 4; i++)
    {
        writeAsicSpi(tcp, regAddr[i], regData);
    }

    for (i = 0; i < 4; i++)
    {
        recvSize = readbackAsicSpi(tcp, serverReply);
    }
}

void resetCount(TcpServer *tcp)
{
    uint8_t regData[1] = {0x1};
    uint16_t regAddr = 0x0B06;
    uint16_t regLen = 0x1;

    uint8_t serverReply[20];

    writeSys(tcp, regAddr, regData, regLen);
    if (readbackSys(tcp, serverReply, 1) <= 0)
    {
        printf("Readout error\n");
    }

    regData[0] = 0x0;

    writeSys(tcp, regAddr, regData, regLen);
    if (readbackSys(tcp, serverReply, 1) <= 0)
    {
        printf("Readout error\n");
    }
}

// void resetThreshold(TcpServer *tcp)
// {
//     int i, recvCounter = 0, spiVal;
//     uint16_t regAddr = 0x63;
//     uint8_t regData[3] = {0};

//     int recvSize;
//     uint8_t serverReply[20];

//     for (i = 0; i < (int)(0x86 - 0x63) + 1; i++)
//     {
//         if (regAddr == 0x63)
//         {
//             spiVal = 32840;
//             intToHexList(regData, spiVal);
//             writeAsicSpi(tcp, regAddr, regData);
//         }
//         else
//         {
//             intToHexList(regData, 0);
//             writeAsicSpi(tcp, regAddr, regData);
//         }
//         regAddr += 0x1;
//         recvCounter += 1;
//     }

//     for (i = 0; i < recvCounter; i++)
//     {
//         if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
//         {
//             printf("Readout error\n");
//         }
//     }
// }

void setThreshold(TcpServer *tcp, int comparator, int channel, int value)
{
    int i, *valueBinary;
    uint16_t regAddr;

    switch (comparator)
    {

    // Comparator 1
    case 1:
        if (channel % 2 == 1)
        {
            if (value < 16)
            {
                regAddr = 0x63 + channel / 2;
                changeSpiValue(tcp, regAddr, value, 4, 16);
                changeSpiValue(tcp, regAddr + 1, 0, 6, 0);
            }
            else
            {
                valueBinary = (int *)malloc(sizeof(int) * BYTES_LEN);
                intToBibaryList(value, BYTES_LEN, valueBinary);

                regAddr = 0x63 + channel / 2;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary + 6, 4), 4, 16);

                regAddr += 1;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary, 6), 6, 0);
            }
        }
        else if (channel % 2 == 0)
        {
            regAddr = 0x63 + channel / 2;
            changeSpiValue(tcp, regAddr, value, 10, 6);
        }
        else
        {
            printf("setThreshold Error\n");
        }
        // End comparator 1
        break;

    case 2:
        if (channel % 2 == 1)
        {
            if (value < 256)
            {
                regAddr = 0x6C + channel / 2;
                changeSpiValue(tcp, regAddr, value, 8, 12);
                changeSpiValue(tcp, regAddr + 1, 0, 2, 0);
            }
            else
            {
                valueBinary = (int *)malloc(sizeof(int) * BYTES_LEN);
                intToBibaryList(value, BYTES_LEN, valueBinary);

                regAddr = 0x6C + channel / 2;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary + 2, 8), 8, 12);

                regAddr += 1;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary, 2), 2, 0);
            }
        }
        else if (channel % 2 == 0)
        {
            regAddr = 0x6C + channel / 2;
            changeSpiValue(tcp, regAddr, value, 10, 2);
        }
        else
        {
            printf("setThreshold Error\n");
        }
        // End comparator 2
        break;

    case 3:
        if (channel % 2 == 0)
        {
            if (value < 64)
            {
                regAddr = 0x75 + channel / 2 - 1;
                changeSpiValue(tcp, regAddr, value, 6, 14);
                changeSpiValue(tcp, regAddr + 1, 0, 4, 0);
            }
            else
            {
                valueBinary = (int *)malloc(sizeof(int) * BYTES_LEN);
                intToBibaryList(value, BYTES_LEN, valueBinary);

                regAddr = 0x75 + channel / 2 - 1;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary + 4, 6), 6, 14);

                regAddr += 1;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary, 4), 4, 0);
            }
        }
        else if (channel % 2 == 1)
        {
            regAddr = 0x75 + channel / 2;
            changeSpiValue(tcp, regAddr, value, 10, 4);
        }
        else
        {
            printf("setThreshold Error\n");
        }
        // End comparator 3
        break;

    case 4:
        if (channel % 2 == 1)
        {
            if (value < 16)
            {
                regAddr = 0x7D + channel / 2;
                changeSpiValue(tcp, regAddr, value, 4, 16);
                changeSpiValue(tcp, regAddr + 1, 0, 6, 0);
            }
            else
            {
                valueBinary = (int *)malloc(sizeof(int) * BYTES_LEN);
                intToBibaryList(value, BYTES_LEN, valueBinary);

                regAddr = 0x7D + channel / 2;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary + 6, 4), 4, 16);

                regAddr += 1;
                changeSpiValue(tcp, regAddr, binaryToInt(valueBinary, 6), 6, 0);
            }
        }
        else if (channel % 2 == 0)
        {
            regAddr = 0x7D + channel / 2;
            changeSpiValue(tcp, regAddr, value, 10, 6);
        }
        else
        {
            printf("setThreshold Error\n");
        }
        // End comparator 4
        break;

    default:
        break;
    }
}

void resetThreshold(TcpServer *tcp)
{
    int i, comparator, channel, value = 10;

    for (channel = 1; channel < 18; channel++)
    {
        for (comparator = 1; comparator < 5; comparator++)
        {
            setThreshold(tcp, comparator, channel, value);
        }
    }
}

void enableAllComp(TcpServer *tcp)
{
    int i, recvCounter = 0, spiVal;
    uint16_t regAddr = 0x47;
    uint8_t regData[3] = {0};

    int recvSize;
    uint8_t serverReply[20];

    for (i = 0; i < (int)(0x4A - 0x47) + 1; i++)
    {
        if (regAddr == 0x47)
        {
            spiVal = 1048568;
            intToHexList(regData, spiVal);
            writeAsicSpi(tcp, regAddr, regData);
        }
        else if (regAddr == 0x4A)
        {
            spiVal = 2047;
            intToHexList(regData, spiVal);
            writeAsicSpi(tcp, regAddr, regData);
        }
        else
        {
            spiVal = 1048575;
            intToHexList(regData, spiVal);
            writeAsicSpi(tcp, regAddr, regData);
        }
        regAddr += 0x1;
        recvCounter += 1;
    }

    for (i = 0; i < recvCounter; i++)
    {
        if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
        {
            printf("Readout error\n");
        }
    }
}

void printStatusScurve(int channel, int dac, int comparator)
{
    system("CLS");
    printf("---------------\n");
    printf("Channel: %i\n", channel);
    printf("Comparator: %i\n", comparator);
    printf("DAC: %i\n", dac);
    printf("---------------\n");
}

void scurve(TcpServer *tcp, int **data, int channel, int minDac, int maxDac, int step, int choiceNumberComparator, int choiceComparator)
{
    uint8_t serverReply[20];
    uint16_t regAddr;

    int i, dac, comparator, recvSize;

    if (choiceNumberComparator == 1)
    {
        enableAllComp(tcp);
    }

    if (choiceNumberComparator == 0)
    {
        resetComp(tcp);
        enableComp(tcp, choiceComparator);
    }
    for (comparator = 1; comparator < 5; comparator++)
    {
        if (choiceComparator == comparator || choiceComparator == 0)
        {
            for (dac = minDac; dac < maxDac + step; dac += step)
            {
                printStatusScurve(channel, dac, comparator);
                setThreshold(tcp, comparator, channel, dac);
                resetCount(tcp);

                Sleep(1000);

                regAddr = (channel - 1) * 4 + (comparator - 1);
                readAsicSpi(tcp, regAddr);
                if (recvSize = readbackAsicSpi(tcp, serverReply) <= 0)
                {
                    printf("Readout error\n");
                }

                data[(dac - minDac) / step][comparator - 1] = hexListToInt(serverReply + 16, 3);
            }
        }
        else
        {
            for (dac = minDac; dac < maxDac + step; dac += step)
            {
                data[(dac - minDac) / step][comparator - 1] = 0;
            }
        }
    }
}

void saveData(FILE *file, int **data, int minDac, int maxDac, int step, int choiceComparator)
{
    system("CLS");
    printf("---------------\n");
    printf("Sauvegarde des donnees\n");
    int dac, comparator;
    for (dac = minDac; dac < maxDac + step; dac += step)
    {
        fprintf(file, "%i", dac);
        if (choiceComparator == 0)
        {
            for (comparator = 0; comparator < NB_COMP; comparator++)
            {
                fprintf(file, ",%i", data[(dac - minDac) / step][comparator]);
            }
            fprintf(file, "\n");
        }
        else
        {
            fprintf(file, ",%i\n", data[(dac - minDac) / step][choiceComparator - 1]);
        }
    }
}

int **defineData(int minDac, int maxDac, int step)
{
    int i, **data, n;

    n = (maxDac + step - minDac) / step;
    data = malloc(sizeof(int *) * n);
    for (i = 0; i < n; i++)
    {
        data[i] = malloc(sizeof(int) * 4);
    }

    return data;
}

// int *defineData(int minDac, int maxDac, int step)
// {
//     int i, *data, n;

//     n = (maxDac + step - minDac) / step;
//     data = malloc(sizeof(int) * n);

//     return data;
// }

void enableExternalADC(TcpServer *tcp)
{
    uint8_t regData[1] = {0x1};
    uint16_t regAddr = 0xE020, regLen = 0x1;
    uint8_t serverReply[20];

    // Check if external ADC is already enable
    readSys(tcp, regAddr);
    if (readbackSys(tcp, serverReply, 1) <= 0)
    {
        printf("Readout error\n");
        exit(EXIT_FAILURE);
    }

    // If not, enable it
    if (serverReply[READBACK_SYS_DATA] == 0x0)
    {
        writeSys(tcp, regAddr, regData, regLen);
        if (readbackSys(tcp, serverReply, 1) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
    }
}

uint8_t **defineADCBufferData(int time)
{
    int i;
    uint8_t **data;
    data = malloc(sizeof(uint8_t *) * 10000 * time);

    return data;
}

void saveADCData(FILE *file, uint8_t **data, int deltaTime)
{
    printf("Sauvegarde des donnees\n");
    int i, j, k, temp;
    unsigned long int timeStamp;
    for (i = 0; i < MAX_EVENTS_SECONDS * deltaTime; i++)
    {
        timeStamp = 0;
        if (data[i][0] == 0)
        {
            timeStamp = (unsigned long int)(0x1000000 * data[i][4] + 0x10000 * data[i][5] + 0x100 * data[i][6] + data[i][7]);
            printf("Evenement %i, %02X %02X %02X %02X timestamp: %lu\n", i, data[i][4], data[i][5], data[i][6], data[i][7], timeStamp);
            fprintf(file, "%lu", timeStamp);
            for (j = 0; j < 20; j++)
            {
                fprintf(file, ", %i", (int)(data[i][17 + j * 2] * 0x100 + data[i][18 + j * 2]));
            }
            fprintf(file, "\n");
        }
        else
        {
            printf("Fin de sauvegarde.\n");
            break;
        }
    }
}

void printVoltUpdate(float volt)
{
    // system("CLS");
    printf("---------------\n");
    printf("Mettre %f Volt\n", volt);
    printf("Appuyez sur entree pour continuer...\n");
    getchar();
}

int **defineAdcChargeData(double deltaVolt)
{
    int i, n = (int)(4.0 / deltaVolt);
    printf("%i rows\n", n + 1);
    int **data = malloc(sizeof(int *) * (n + 1));

    return data;
}

int **adcFunctionCharge(UdpServer *udp, double maxVolt, double minVolt, double deltaVolt, int meanTime)
{
    float volt;
    uint8_t **bufferData;
    int **data = defineAdcChargeData(deltaVolt);
    int sum = 0, rows = 0, cols = 0, dataRows = 0;

    for (volt = minVolt; volt < maxVolt + deltaVolt; volt += deltaVolt)
    {
        printVoltUpdate(volt);

        int *mean = (int *)malloc(sizeof(int) * 20);

        // res = listenData(udp, bufferData, meanTime);
        bufferData = listenData(udp, meanTime);
        for (cols = 0; cols < 20; cols++)
        {
            rows = 0;
            sum = 0;
            while (bufferData[rows][0] != 1)
            {
                sum += (int)(bufferData[rows][17 + cols * 2] * 0x100 + bufferData[rows][18 + cols * 2]);
                rows++;
            }
            mean[cols] = sum / rows;
        }
        data[dataRows] = mean;
        dataRows++;
    }

    return data;
}

void saveADCChargeData(FILE *file, int **data, double maxVolt, double minVolt, double deltaVolt)
{
    system("CLS");
    printf("---------------\n");
    printf("Sauvegarde des donnees\n");
    int i, j, n;
    n = (int)((maxVolt + deltaVolt - minVolt) / deltaVolt);
    for (i = 0; i < n; i++)
    {
        fprintf(file, "%lf", (float)(minVolt + i * deltaVolt));
        for (j = 0; j < 20; j++)
        {
            fprintf(file, ",%i", data[i][j]);
        }
        fprintf(file, "\n");
    }
}

void readSystemConfiguration(TcpServer *tcp, cJSON *root)
{
    int i, j, range = 0, hexLen = 0;

    uint16_t regAddr = 0;
    uint16_t regLen = 0;

    uint8_t serverReply[77];

    cJSON *main = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "System", main);

    // System
    range = 4;
    uint16_t sys_addr[4] = {0x0000, 0x0001, 0x0002, 0x0010};
    uint16_t sys_len[4] = {0x8, 0x2, 0x2, 0x1};
    char *sys_names[] = {"Serial number", "Firmware type", "Firmware version", "System number"};

    cJSON *system = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "System", system);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, sys_addr[i]);
        if (readbackSys(tcp, serverReply, sys_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        printf("name: %s\n", sys_names[i]);
        printf("int: %i\n", hexListToInt(serverReply + 13, sys_len[i]));
        printf("len: %i\n", sys_len[i]);
        printf("received: ");
        for (j = 0; j < sys_len[i]; j++)
        {
            printf("%02X ", serverReply[13 + j]);
        }
        cJSON_AddItemToObject(system, sys_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, sys_len[i])));
    }

    // Power supply
    range = 2;
    uint16_t ps_addr[2] = {0x0B00, 0x0B01};
    uint16_t ps_len[2] = {0x1, 0x1};
    char *ps_names[] = {"ASIC AVDD ENABLE", "ASIC DVDD ENABLE"};

    cJSON *ps = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "Power supply", ps);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, ps_addr[i]);
        if (readbackSys(tcp, serverReply, ps_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        cJSON_AddItemToObject(ps, ps_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, ps_len[i])));
    }

    // Counter
    range = 3;
    uint16_t ctr_addr[3] = {0x0B06, 0x0B07, 0x0B08};
    uint16_t ctr_len[3] = {0x1, 0x1, 0x1};
    char *ctr_names[] = {"COUNT_CLEAR", "COUNT2BUF", "COUNT_ENABLE"};

    cJSON *ctr = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "Counter", ctr);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, ctr_addr[i]);
        if (readbackSys(tcp, serverReply, ctr_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        cJSON_AddItemToObject(ctr, ctr_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, ctr_len[i])));
    }

    // ASIC signal
    uint16_t as_addr = 0x0B09;
    uint16_t as_len = 1;
    cJSON *as = cJSON_CreateObject();

    cJSON_AddItemToObject(main, "ASIC signal", as);
    readSys(tcp, as_addr);
    if (readbackSys(tcp, serverReply, as_len) <= 0)
    {
        printf("Readout error\n");
        exit(EXIT_FAILURE);
    }
    cJSON_AddItemToObject(as, "RESET_I", cJSON_CreateNumber(hexListToInt(serverReply + 13, as_len)));

    // Cal pulse generator
    range = 7;
    uint16_t cal_addr[7] = {0x0C00, 0x0C01, 0x0C02, 0x0C03, 0x0C04, 0x0C05, 0x0C06};
    uint16_t cal_len[7] = {0x1, 0x1, 0x4, 0x4, 0x4, 0x2, 0x4};
    char *cal_names[] = {"Calibration execute", "Pulse polarity", "Pulse amount", "Pulse length", "Pulse Interval", "Pulse trigger delay", "Output mask"};

    cJSON *cal = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "Cal pulse generator", cal);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, cal_addr[i]);
        if (readbackSys(tcp, serverReply, cal_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        cJSON_AddItemToObject(cal, cal_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, cal_len[i])));
    }

    // DACs
    range = 12;
    uint16_t dac_addr[12] = {0x0E00, 0x0E01, 0x0E02, 0x0E03, 0x0E04, 0x0E05, 0x0E06, 0x0E07, 0x0E08, 0x0E09, 0x0E0A, 0x0E0B};
    uint16_t dac_len[12] = {0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2, 0x2};
    char *dac_names[] = {"MREF_I", "I leak", "Sqare(GPibias 3)", "Sqare (GPibias 4)", "ADC_VREFP", "ADC_VREFN", "Sqare (Vbias 3)", "Sqare (Vbias 3)", "Mbias", "Sqare (Mbias 2)", "Cal", "Analog_ref"};

    cJSON *dac = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "DACs", dac);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, dac_addr[i]);
        if (readbackSys(tcp, serverReply, dac_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        cJSON_AddItemToObject(dac, dac_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, dac_len[i])));
    }

    // Internal ADC readout
    range = 6;
    uint16_t iadc_addr[6] = {0xE010, 0xE012, 0xE013, 0xE014, 0xE015, 0xE016};
    uint16_t iadc_len[6] = {0x1, 0x2, 0x1, 0x1, 0x2, 0x2};
    char *iadc_names[] = {"Enable internal ADC readout", "Number of channels per event", "Number of events per readout", "Internal ADC mode", "Hold delay", "Hold duration"};

    cJSON *iadc = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "Counter", iadc);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, iadc_addr[i]);
        if (readbackSys(tcp, serverReply, iadc_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        cJSON_AddItemToObject(iadc, iadc_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, iadc_len[i])));
    }

    // Galao ADC readout
    range = 5;
    uint16_t eadc_addr[5] = {0xE020, 0xE023, 0xE024, 0xE025, 0xE026};
    uint16_t eadc_len[5] = {0x1, 0x4, 0x4, 0x4, 0x2};
    char *eadc_names[] = {"Enable Galao ADC readout", "ro_fixed_list", "SHIFT_IN delay", "SHIFT_IN duration", "Phase shift"};

    cJSON *eadc = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "Counter", eadc);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, eadc_addr[i]);
        if (readbackSys(tcp, serverReply, eadc_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        cJSON_AddItemToObject(eadc, eadc_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, eadc_len[i])));
    }

    // Clock generator
    range = 3;
    uint16_t clk_addr[3] = {0xF009, 0xF00A, 0xF00B};
    uint16_t clk_len[3] = {0x1, 0x1, 0x2};
    char *clk_names[] = {"Enable clock generator", "Clock period", "Clock division"};

    cJSON *clk = cJSON_CreateObject();
    cJSON_AddItemToObject(main, "Counter", clk);
    for (i = 0; i < range; i++)
    {
        readSys(tcp, clk_addr[i]);
        if (readbackSys(tcp, serverReply, clk_len[i]) <= 0)
        {
            printf("Readout error\n");
            exit(EXIT_FAILURE);
        }
        cJSON_AddItemToObject(clk, clk_names[i], cJSON_CreateNumber(hexListToInt(serverReply + 13, clk_len[i])));
    }
}

void readAsicConfiguration(TcpServer *tcp, cJSON *root)
{
    int i;
    uint16_t regAddr = 0x44;
    uint16_t regLen = 0x3;

    cJSON *asic = cJSON_CreateObject();

    cJSON_AddItemToObject(root, "ASIC", asic);
}

void writeJSON(cJSON *root)
{
    char *out;
    out = cJSON_Print(root);
    printf("%s\n", out);

    // FILE *file = fopen("configuration.json", "w");
    // fprintf(file, "%s", out);
    // fclose(file);
}