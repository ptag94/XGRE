#include <stdio.h>

#include "headers/IDEAS_packetDecode.h"
#include "headers/IDEAS_packetEncode.h"
#include "headers/IDEAS_packetTypes.h"

#define SINGLE_EVENT_PULSE_HEIGHT_DATA_LENGHT 7

int decodeError(IDEAS_packet_decodeStatus decodeStatus)
{
    if (decodeStatus != IDEAS_PACKET_DECODE_SUCCESS)
    {
        switch (decodeStatus)
        {
        case IDEAS_PACKET_DECODE_ERROR_INCORRECT_LENGTH:
            printf("Decode error: The packet or a packet field had incorrect length.\n");
            break;
        case IDEAS_PACKET_DECODE_ERROR_DATA_LENGTH_FIELD_MISMATCH:
            printf("Decode error: The data field of the packet did not match the length of the packet.\n");
            break;
        case IDEAS_PACKET_DECODE_ERROR_MEMORY_ALLOCATION:
            printf("Decode error: The system was not able to allocate the requested memory.\n");
            break;
        case IDEAS_PACKET_DECODE_ERROR_UNKNOWN:
        default:
            printf("Decode error: An unknown decoding error occured.\n");
            break;
        }

        return 1;
    }

    return 0;
}

int testDecoder(uint8_t *pPacket, uint16_t packet_len)
{
    int i;

    IDEAS_packet_header packetHeader;
    IDEAS_packet_decodeStatus decodeStatus;

    decodeStatus = IDEAS_decode_header(
        pPacket,
        packet_len,
        &packetHeader);

    if (decodeError(decodeStatus))
    {
        printf("Decode error: Header decode failed.\n");
        return 1;
    }

    printf("packetHeader:\n"
           "\tversion:\t %02X\n"
           "\tsystem_num:\t %02X\n"
           "\tpacket_type:\t %02X\n"
           "\tseq_flags:\t %02X\n"
           "\tpacket_count:\t %04X\n"
           "\ttimestamp:\t %08X\n"
           "\tdata_len:\t %04X\n\n",

           packetHeader.version,
           packetHeader.system_num,
           packetHeader.packet_type,
           packetHeader.seq_flags,
           packetHeader.packet_count,
           packetHeader.timestamp,
           packetHeader.data_len);

    switch (packetHeader.packet_type)
    {
    case IDEAS_PACKET_TYPE_WRITE_SYSTEM_REGISTER:
    {
        IDEAS_packet_writeSystemRegister packetData;

        decodeStatus = IDEAS_decode_writeSystemRegister(
            pPacket + IDEAS_PACKET_HEADER_LENGTH,
            packet_len - IDEAS_PACKET_HEADER_LENGTH,
            &packetData);

        if (decodeError(decodeStatus))
        {
            printf("Decode error: Write System Register decode failed.\n");
            return 1;
        }

        printf("Decoded Write System Register packet:\n"
               "packetData:\n"
               "\treg_addr:\t %04X\n"
               "\treg_len:\t %02X\n",

               packetData.reg_addr,
               packetData.reg_len);
        printf("\treg_data:\t {");
        for (i = 0; i < packetData.reg_len; i++)
        {
            printf("%02X ", packetData.reg_data[i]);
        }
        printf("\b}\n");

        IDEAS_free_writeSystemRegister(&packetData);

        break;
    }

    case IDEAS_PACKET_TYPE_READ_SYSTEM_REGISTER:
    {
        IDEAS_packet_readSystemRegister packetData;

        decodeStatus = IDEAS_decode_readSystemRegister(
            pPacket + IDEAS_PACKET_HEADER_LENGTH,
            packet_len - IDEAS_PACKET_HEADER_LENGTH,
            &packetData);

        if (decodeError(decodeStatus))
        {
            printf("Decode error: Read System Register decode failed.\n");
            return 1;
        }

        printf("Decoded Read System Register packet:\n"
               "packetData:\n"
               "\treg_addr:\t %04X\n",
               packetData.reg_addr);
        break;
    }

    case IDEAS_PACKET_TYPE_SYSTEM_REGISTER_READBACK:
    {
        IDEAS_packet_systemRegisterReadBack packetData;

        decodeStatus = IDEAS_decode_systemRegisterReadBack(
            pPacket + IDEAS_PACKET_HEADER_LENGTH,
            packet_len - IDEAS_PACKET_HEADER_LENGTH,
            &packetData);

        if (decodeError(decodeStatus))
        {
            printf("Decode error: System Register Readback decode failed.\n");
            return 1;
        }

        printf("Decoded System Register Readback packet:\n"
               "packetData:\n"
               "\treg_addr:\t %04X\n"
               "\treg_len:\t %02X\n",

               packetData.reg_addr,
               packetData.reg_len);
        printf("\treg_data:\t {");
        for (i = 0; i < packetData.reg_len; i++)
        {
            printf("%02X ", packetData.reg_data[i]);
        }
        printf("\b}\n");

        IDEAS_free_systemRegisterReadBack(&packetData);
        break;
    }

    case IDEAS_PACKET_TYPE_ASIC_CONF_WRITE_READ:
    {
        IDEAS_packet_asicConfig packetData;

        decodeStatus = IDEAS_decode_asicConfigRegister(
            pPacket + IDEAS_PACKET_HEADER_LENGTH,
            packet_len - IDEAS_PACKET_HEADER_LENGTH,
            &packetData);

        if (decodeError(decodeStatus))
        {
            printf("Decode error: ASIC Configuration Register Write/Read decode failed.\n");
            return 1;
        }

        printf("Decoded ASIC Configuration Register Write/Read packet:\n"
               "packetData:\n"
               "\tasic_id:\t %02X\n"
               "\tconf_len:\t %04X\n",

               packetData.asic_id,
               packetData.conf_len);
        printf("\tconf_data:\t {");
        for (i = 0; i < packetData.conf_len; i++)
        {
            printf("%02X ", packetData.conf_data[i]);
        }
        printf("\b}\n");

        IDEAS_free_asicConfigRegister(&packetData);
        break;
    }

    case IDEAS_PACKET_TYPE_ASIC_CONF_READBACK:
    {
        IDEAS_packet_asicConfig packetData;

        decodeStatus = IDEAS_decode_asicConfigRegister(
            pPacket + IDEAS_PACKET_HEADER_LENGTH,
            packet_len - IDEAS_PACKET_HEADER_LENGTH,
            &packetData);

        if (decodeError(decodeStatus))
        {
            printf("Decode error: ASIC Configuration Register Readback decode failed.\n");
            return 1;
        }

        printf("Decoded ASIC Configuration Register Readback packet:\n"
               "packetData:\n"
               "\tasic_id:\t %02X\n"
               "\tconf_len:\t %04X\n",

               packetData.asic_id,
               packetData.conf_len);

        printf("\tconf_data:\t {");
        for (i = 0; i < packetData.conf_len; i++)
        {
            printf("%02X ", packetData.conf_data[i]);
        }
        printf("\b}\n");

        IDEAS_free_asicConfigRegister(&packetData);
        break;
    }

    case IDEAS_PACKET_TYPE_ASIC_SPI_REGISTER_READBACK:
    {
        IDEAS_packet_asicSpiRegister packetData;

        decodeStatus = IDEAS_decode_asicSpiRegisterReadback(
            pPacket + IDEAS_PACKET_HEADER_LENGTH,
            packet_len - IDEAS_PACKET_HEADER_LENGTH,
            &packetData);

        if (decodeError(decodeStatus))
        {
            printf("Decode error: ASIC SPI Register Readback decode failed.\n");
            return 1;
        }

        printf("Decoded ASIC Configuration Register Readback packet:\n"
               "packetData:\n"
               "\tasic_id:\t %02X\n"
               "\reg_addr:\t %04X\n",

               packetData.asic_id,
               packetData.reg_addr);

        printf("\treg_data:\t {");
        for (i = 0; i < packetData.reg_len; i++)
        {
            printf("%02X ", packetData.reg_data[i]);
        }
        printf("\b}\n");
        // uint8_t bin;
        // for (i = 0; i < packetData.reg_len; i++)
        // {
        //     bin = packetData.reg_data[i];
        //     printf("Converting number to binary 0b%d%d%d%d%d%d%d%d\n",
        //            (bin >> 7) & 1, (bin >> 6) & 1, (bin >> 5) & 1, (bin >> 4) & 1,
        //            (bin >> 3) & 1, (bin >> 2) & 1, (bin >> 1) & 1, (bin >> 0) & 1);
        // }

        IDEAS_free_asicSpiRegister(&packetData);
        break;
    }

    case IDEAS_PACKET_TYPE_PULSE_HEIGHT_DATA:
    {
        uint8_t *packetDataFields;
        uint8_t *packetData;
        packetDataFields = pPacket + IDEAS_PACKET_HEADER_LENGTH;
        packetData = pPacket + IDEAS_PACKET_HEADER_LENGTH + SINGLE_EVENT_PULSE_HEIGHT_DATA_LENGHT;

        for (i = 0; i < 7; i++)
        {
            printf("test %02X\n", packetDataFields[i]);
        }

        for (i = 0; i < 17; i++)
        {
            printf("Channel %i: %02X%02X\n", i, packetData[2 * i], packetData[2 * i + 1]);
        }

        free(packetData);
        free(packetDataFields);
        break;
    }

    default:
        printf("Decode error: Unknown packet type.\n");
        return 1;
        break;
    }

    return 0;
}