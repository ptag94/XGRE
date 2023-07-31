#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "cJSON.h"

#define NB_CHANNELS 17
#define NB_COMP 4
#define NB_THRESHOLD 1023

/**
 * @brief Load a JSON file
 *
 * @param tcp Address of tcp object
 */
void loadConfiguration(TcpServer *tcp);

/**
 * @brief Enable counter module
 *
 * @param tcp Adress of tcp object
 */
void startCounter(TcpServer *tcp);

/**
 * @brief Disable all comparators
 *
 * @param tcp Adress of tcp object
 */
void resetComp(TcpServer *tcp);

/**
 * @brief Enable one comparator
 *
 * @param tcp Adress of tcp object
 * @param comp Choice of comparator to enable (1, 2, 3, 4)
 */
void enableComp(TcpServer *tcp, int comp);

/**
 * @brief Reset counters
 *
 * @param tcp Adress of tcp object
 */
void resetCount(TcpServer *tcp);

/**
 * @brief Set all threshold to 0
 *
 * @param tcp Adress of tcp object
 */
void resetThreshold(TcpServer *tcp);

/**
 * @brief Perform a S-curv
 *
 * @param tcp Adress of tcp object
 * @param data 2D array
 * @param channel Channel to gather data
 * @param minDac Minimum value of threshold
 * @param maxDac Maximum value of threshold
 * @param step Step for threshold
 * @param choiceNumberComparator 1 if all comparators will be activated at once of 0 for one at a time
 */
void scurve(TcpServer *tcp, int **data, int channel, int minDac, int maxDac, int step, int choiceNumberComparator, int choiceComparator);

/**
 * @brief Save 2D array in a csv file
 *
 * @param file FILE pointer address
 * @param data 2D array
 * @param minDac Minimum value for threshold
 * @param maxDac Maximum value for threshold
 * @param step Step for threshold
 */
void saveData(FILE *file, int **data, int minDac, int maxDac, int step, int choiceComparator);

/**
 * @brief Create a 2D array depending of the arguements bellow
 *
 * @param minDac Minimum value
 * @param maxDac Maximum value
 * @param step Step for each iteration
 * @return int** 2D array
 */
int **defineData(int minDac, int maxDac, int step);

/**
 * @brief Enable all comparators
 *
 * @param tcp Adress of tcp object
 */
void enableAllComp(TcpServer *tcp);

/**
 * @brief Check if external ADC is enable, if not, enable it
 *
 * @param tcp
 */
void enableExternalADC(TcpServer *tcp);

/**
 * @brief Create a 2D array for ADC datas. The number of rows depends on MAX_EVENTS_SECONDS times a duration
 *
 * @param time Duration of collecting data
 * @return uint8_t** 2D array
 */
uint8_t **defineADCBufferData(int time);

/**
 * @brief Save 2D array into a file
 *
 * @param file Pointer to the file where data will be save
 * @param data 2D array containing ADC values recorded for deltaTime seconds
 * @param deltaTime Time interval for data recording
 */
void saveADCData(FILE *file, uint8_t **data, int deltaTime);

/**
 * @brief Iterate over Voltage for gathering ADC in function of voltage
 *
 * @param udp Address of udp object
 * @param maxVolt Maximum voltage input
 * @param minVolt Minimum voltage input
 * @param deltaVolt Step for each voltage itaration
 * @param meanTime Time for gathering data
 * @return int** 2D array
 */
int **adcFunctionCharge(UdpServer *udp, double maxVolt, double minVolt, double deltaVolt, int meanTime);

/**
 * @brief Save ADC data into a csv
 *
 * @param file FILE pointer
 * @param data 2D array
 * @param maxVolt Maximum voltage
 * @param minVolt Minimum voltage
 * @param deltaVolt Step of voltage iteration
 */
void saveADCChargeData(FILE *file, int **data, double maxVolt, double minVolt, double deltaVolt);

/**
 * @brief Read each register of ASIC
 *
 * @param tcp Address of tcp object
 * @param root cJSON object
 */
void readSystemConfiguration(TcpServer *tcp, cJSON *root);

/**
 * @brief Read each APOCAT SPI register
 *
 * @param tcp Address of tcp object
 * @param root cJSON object
 */
void readAsicConfiguration(TcpServer *tcp, cJSON *root);

/**
 * @brief Write JSON to APOCAT
 *
 * @param root cJSON object
 */
void writeJSON(cJSON *root);
#endif