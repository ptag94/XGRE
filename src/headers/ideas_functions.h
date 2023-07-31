#ifndef IDEAS_FUNCTIONS_H
#define IDEAS_FUNCTIONS_H

#include "IDEAS_packetEncode.h"
#include "server.h"

/**
 * @brief Define the buffer needed to APOCAT
 * 
 * @param choice Which type of buffer
 * @param encBuffer The buffer
 * @param buffer_len Lenght of the buffer in bytes
 * @param reg_len Hex numbers used
 */
void define_buffer(int choice, uint8_t **encBuffer, uint16_t *buffer_len, uint16_t reg_len);

/**
 * @brief Write to APOCAT SPI register
 * 
 * @param tcp Adress of tcp object
 * @param regAddr SPI register address
 * @param regData list of hex object to send 
 */
void writeAsicSpi(TcpServer *tcp, uint16_t regAddr, uint8_t *regData);

/**
 * @brief Read the readback packet
 * 
 * @param tcp Adress of tcp object
 * @param serverReply List for containing the response in hex
 * @return int Number of hex received
 */
int readbackAsicSpi(TcpServer *tcp, uint8_t *serverReply);

/**
 * @brief Read APOCAT SPI register
 * 
 * @param tcp Adress of tcp object
 * @param regAddr Hex address of SPI register
 */
void readAsicSpi(TcpServer *tcp, uint16_t regAddr);

/**
 * @brief Write system
 * 
 * @param tcp Adress of tcp object
 * @param regAddr Hex address of system
 * @param regData Hex list of data
 * @param regLen Hex numbers used
 */
void writeSys(TcpServer *tcp, uint16_t regAddr, uint8_t *regData, uint16_t regLen);

/**
 * @brief Read the readback packet
 * 
 * @param tcp Adress of tcp object
 * @param serverReply List containing the response in hex
 * @param regLen Hex numbers received
 * @return int Numbers of hex received
 */
int readbackSys(TcpServer *tcp, uint8_t *serverReply, uint16_t regLen);

/**
 * @brief Read system 
 * 
 * @param tcp Adress of tcp object
 * @param regAddr Hex address to read
 */
void readSys(TcpServer *tcp, uint16_t regAddr);

#endif