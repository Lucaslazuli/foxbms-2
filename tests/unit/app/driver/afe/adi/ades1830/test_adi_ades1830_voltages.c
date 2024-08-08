/**
 *
 * @copyright &copy; 2010 - 2024, Fraunhofer-Gesellschaft zur Foerderung der angewandten Forschung e.V.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * We kindly request you to use one or more of the following phrases to refer to
 * foxBMS in your hardware, software, documentation or advertising materials:
 *
 * - "This product uses parts of foxBMS&reg;"
 * - "This product includes parts of foxBMS&reg;"
 * - "This product is derived from foxBMS&reg;"
 *
 */

/**
 * @file    test_adi_ades1830_voltages.c
 * @author  foxBMS Team
 * @date    2022-12-07 (date of creation)
 * @updated 2024-08-08 (date of last update)
 * @version v1.7.0
 * @ingroup UNIT_TEST_IMPLEMENTATION
 * @prefix  TEST
 *
 * @brief   Test of some module
 *
 */

/*========== Includes =======================================================*/
#include "unity.h"
#include "Mockadi_ades183x_cfg.h"
#include "Mockadi_ades183x_diagnostic.h"
#include "Mockos.h"
#include "Mockspi.h"

#include "adi_ades183x_buffers.h" /* use the real buffer configuration */
#include "adi_ades183x_commands.h"
#include "adi_ades183x_commands_voltages.h"
#include "adi_ades183x_defs.h"
#include "adi_ades183x_helpers.h"
#include "adi_ades183x_pec.h"
#include "adi_ades183x_voltages.h"
#include "spi_cfg-helper.h"
#include "test_assert_helper.h"

#include <stdbool.h>
#include <stdint.h>

/*========== Unit Testing Framework Directives ==============================*/
TEST_SOURCE_FILE("adi_ades183x_buffers.c")
TEST_SOURCE_FILE("adi_ades183x_voltages.c")

TEST_INCLUDE_PATH("../../src/app/application/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/ades1830")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/config")
TEST_INCLUDE_PATH("../../src/app/driver/afe/adi/common/ades183x/pec")
TEST_INCLUDE_PATH("../../src/app/driver/afe/api")
TEST_INCLUDE_PATH("../../src/app/driver/config")
TEST_INCLUDE_PATH("../../src/app/driver/dma")
TEST_INCLUDE_PATH("../../src/app/driver/io")
TEST_INCLUDE_PATH("../../src/app/driver/pex")
TEST_INCLUDE_PATH("../../src/app/driver/rtc")
TEST_INCLUDE_PATH("../../src/app/driver/spi")
TEST_INCLUDE_PATH("../../src/app/driver/ts/api")
TEST_INCLUDE_PATH("../../src/app/engine/database")
TEST_INCLUDE_PATH("../../src/app/engine/diag")
TEST_INCLUDE_PATH("../../src/app/task/config")
TEST_INCLUDE_PATH("../../src/app/task/ftask")

/*========== Definitions and Implementations for Unit Test ==================*/

static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageTable          = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageAverageTable   = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageFilteredTable  = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageRedundantTable = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageOpenWireEvenTable = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageOpenWireOddTable = {.header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageAverageOpenWireTable = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static DATA_BLOCK_CELL_VOLTAGE_s adi_cellVoltageRedundantOpenWireTable = {
    .header.uniqueId = DATA_BLOCK_ID_CELL_VOLTAGE_BASE};
static ADI_ERROR_TABLE_s adi_errorTable = {0}; /*!< init in ADI_ResetErrorTable-function */
static uint16_t adi_bufferRxPecTest[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};
static uint16_t adi_bufferTxPecTest[ADI_N_BYTES_FOR_DATA_TRANSMISSION] = {0};

ADI_STATE_s adi_stateBase = {
    .data.txBuffer                     = adi_bufferTxPecTest,
    .data.rxBuffer                     = adi_bufferRxPecTest,
    .data.cellVoltage                  = &adi_cellVoltageTable,
    .data.cellVoltageAverage           = &adi_cellVoltageAverageTable,
    .data.cellVoltageFiltered          = &adi_cellVoltageFilteredTable,
    .data.cellVoltageRedundant         = &adi_cellVoltageRedundantTable,
    .data.cellVoltageOpenWireEven      = &adi_cellVoltageOpenWireEvenTable,
    .data.cellVoltageOpenWireOdd       = &adi_cellVoltageOpenWireOddTable,
    .data.cellVoltageAverageOpenWire   = &adi_cellVoltageAverageOpenWireTable,
    .data.cellVoltageRedundantOpenWire = &adi_cellVoltageRedundantOpenWireTable,
    .data.errorTable                   = &adi_errorTable,
};

ADI_COMMAND_READ_REGISTERS_s commandBytes = {
    .registerA = {0},
    .registerB = {0},
    .registerC = {0},
    .registerD = {0},
    .registerE = {0},
    .registerF = {0},
};

/* If this configuration is changed, testADI_GetStoredVoltageIndex() must be adapted */
const uint8_t adi_voltageInputsUsed[ADI_MAX_SUPPORTED_CELLS] = {
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
    1,
};

/** SPI data configuration struct for ADI communication */
static spiDAT1_t spi_kAdiDataConfig[BS_NR_OF_STRINGS] = {
    {                      /* struct is implemented in the TI HAL and uses uppercase true and false */
     .CS_HOLD = TRUE,      /* If true, HW chip select kept active between words */
     .WDEL    = FALSE,     /* Activation of delay between words */
     .DFSEL   = SPI_FMT_0, /* Data word format selection */
     .CSNR    = SPI_HARDWARE_CHIP_SELECT_2_ACTIVE},
};

/**
 * SPI interface configuration for ADI communication
 * This is a list of structs because of multi string
 */
SPI_INTERFACE_CONFIG_s spi_adiInterface[BS_NR_OF_STRINGS] = {
    {
        .pConfig  = &spi_kAdiDataConfig[0u],
        .pNode    = spiREG1,
        .pGioPort = &(spiREG1->PC3),
        .csPin    = 2u,
        .csType   = SPI_CHIP_SELECT_HARDWARE,
    },
};

/*========== Setup and Teardown =============================================*/
void setUp(void) {
}

void tearDown(void) {
}

/*========== Test Cases =====================================================*/

/*========== Externalized Static Function Test Cases ========================*/
void testADI_CopyCommandBytesCellVoltageRegister(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_CopyCommandBytesCellVoltageRegister(NULL_PTR));

    TEST_ADI_CopyCommandBytesCellVoltageRegister(&commandBytes);
    for (uint8_t i = 0u; i < ADI_COMMAND_DEFINITION_LENGTH; i++) {
        TEST_ASSERT_EQUAL(adi_cmdRdcva[i], commandBytes.registerA[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdcvb[i], commandBytes.registerB[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdcvc[i], commandBytes.registerC[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdcvd[i], commandBytes.registerD[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdcve[i], commandBytes.registerE[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdcvf[i], commandBytes.registerF[i]);
    }
}
void testADI_CopyCommandBytesAverageCellVoltageRegisters(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_CopyCommandBytesAverageCellVoltageRegisters(NULL_PTR));

    TEST_ADI_CopyCommandBytesAverageCellVoltageRegisters(&commandBytes);
    for (uint8_t i = 0u; i < ADI_COMMAND_DEFINITION_LENGTH; i++) {
        TEST_ASSERT_EQUAL(adi_cmdRdaca[i], commandBytes.registerA[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdacb[i], commandBytes.registerB[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdacc[i], commandBytes.registerC[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdacd[i], commandBytes.registerD[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdace[i], commandBytes.registerE[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdacf[i], commandBytes.registerF[i]);
    }
}
void testADI_CopyCommandBytesFilteredCellVoltageRegisters(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_CopyCommandBytesFilteredCellVoltageRegisters(NULL_PTR));

    TEST_ADI_CopyCommandBytesFilteredCellVoltageRegisters(&commandBytes);
    for (uint8_t i = 0u; i < ADI_COMMAND_DEFINITION_LENGTH; i++) {
        TEST_ASSERT_EQUAL(adi_cmdRdfca[i], commandBytes.registerA[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdfcb[i], commandBytes.registerB[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdfcc[i], commandBytes.registerC[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdfcd[i], commandBytes.registerD[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdfce[i], commandBytes.registerE[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdfcf[i], commandBytes.registerF[i]);
    }
}
void testADI_CopyCommandBytesRedundantCellVoltageRegisters(void) {
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_CopyCommandBytesRedundantCellVoltageRegisters(NULL_PTR));

    TEST_ADI_CopyCommandBytesRedundantCellVoltageRegisters(&commandBytes);
    for (uint8_t i = 0u; i < ADI_COMMAND_DEFINITION_LENGTH; i++) {
        TEST_ASSERT_EQUAL(adi_cmdRdsva[i], commandBytes.registerA[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdsvb[i], commandBytes.registerB[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdsvc[i], commandBytes.registerC[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdsvd[i], commandBytes.registerD[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdsve[i], commandBytes.registerE[i]);
        TEST_ASSERT_EQUAL(adi_cmdRdsvf[i], commandBytes.registerF[i]);
    }
}
void testADI_ReadAndStoreVoltages(void) {
    /* Invalid state */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_ReadAndStoreVoltages(NULL_PTR, &commandBytes, ADI_CELL_VOLTAGE));
    /* Invalid command bytes */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_ReadAndStoreVoltages(&adi_stateBase, NULL_PTR, ADI_CELL_VOLTAGE));
    /* Invalid store location */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_ADI_ReadAndStoreVoltages(&adi_stateBase, &commandBytes, ADI_VOLTAGE_STORE_LOCATION_E_MAX));
}

void testADI_SaveRxToCellVoltageBuffer(void) {
    /* Test invalid state */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_ADI_SaveRxToCellVoltageBuffer(NULL_PTR, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, ADI_CELL_VOLTAGE));
    /* Test invalid data */
    TEST_ASSERT_FAIL_ASSERT(
        TEST_ADI_SaveRxToCellVoltageBuffer(&adi_stateBase, NULL_PTR, ADI_RESULT_REGISTER_SET_A, ADI_CELL_VOLTAGE));
    /* Test invalid register set */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_SaveRxToCellVoltageBuffer(
        &adi_stateBase, adi_dataReceive, (ADI_RESULT_REGISTER_SET_F + 1u), ADI_CELL_VOLTAGE));
    /* Test invalid store location */
    TEST_ASSERT_FAIL_ASSERT(TEST_ADI_SaveRxToCellVoltageBuffer(
        &adi_stateBase, adi_dataReceive, ADI_RESULT_REGISTER_SET_A, ADI_VOLTAGE_STORE_LOCATION_E_MAX));
}

/*========== Extern Function Test Cases =====================================*/
void testADI_GetStoredVoltageIndex(void) {
    /* Invalid index */
    TEST_ASSERT_FAIL_ASSERT(ADI_GetStoredVoltageIndex(ADI_MAX_SUPPORTED_CELLS));

    /* All inputs used, indices must be equal */
    for (uint8_t c = 0u; c < ADI_MAX_SUPPORTED_CELLS; c++) {
        TEST_ASSERT_EQUAL(c, ADI_GetStoredVoltageIndex(c));
    }
}

void testADI_GetVoltages(void) {
    /* Invalid pointer */
    TEST_ASSERT_FAIL_ASSERT(ADI_GetVoltages(NULL_PTR, ADI_CELL_VOLTAGE_REGISTER, ADI_CELL_VOLTAGE));
    /* Invalid register type */
    TEST_ASSERT_FAIL_ASSERT(ADI_GetVoltages(&adi_stateBase, ADI_VOLTAGE_REGISTER_TYPE_E_MAX, ADI_CELL_VOLTAGE));
    /* Invalid store location */
    TEST_ASSERT_FAIL_ASSERT(
        ADI_GetVoltages(&adi_stateBase, ADI_CELL_VOLTAGE_REGISTER, ADI_VOLTAGE_STORE_LOCATION_E_MAX));
}