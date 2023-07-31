#ifndef DECODER_H
#define DECODER_H

/**
 * @brief Decode APOCAT return packet et print it
 * 
 * @param pPacket Buffer containing the packet 
 * @param packet_len Len of packet
 * @return int if correctly passed
 */
int testDecoder(uint8_t *pPacket, uint16_t packet_len);

#endif