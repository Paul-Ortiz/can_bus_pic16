/*
    mcp_can.cpp
    2012 Copyright (c) Seeed Technology Inc.  All right reserved.

    Author:Loovee (loovee@seeed.cc)
    2014-1-16

    Contributor:

    Cory J. Fowler
    Latonita
    Woodward1
    Mehtajaghvi
    BykeBlast
    TheRo0T
    Tsipizic
    ralfEdmund
    Nathancheek
    BlueAndi
    Adlerweb
    Btetz
    Hurvajs
    xboxpro1
    ttlappalainen

    The MIT License (MIT)

    Copyright (c) 2013 Seeed Technology Inc.

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.
*/
#define DEBUG_EN 0
#include "mcp2515_can.h"

/*#define spi_readwrite      pSPI->transfer
#define spi_read()         spi_readwrite(0x00)
#define spi_write(spi_val) spi_readwrite(spi_val)
#define SPI_BEGIN()        pSPI->beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0))
#define SPI_END()          pSPI->endTransaction()
*/
/*********************************************************************************************************
** Function name:           txCtrlReg
** Descriptions:            return tx ctrl reg according to tx buffer index.
**                          According to my tests this is faster and saves memory compared using vector
*********************************************************************************************************/
unsigned char txCtrlReg(unsigned char i) {
    switch (i) {
        case 0: return MCP_TXB0CTRL;
        case 1: return MCP_TXB1CTRL;
        case 2: return MCP_TXB2CTRL;
    }
    return MCP_TXB2CTRL;
}

/*********************************************************************************************************
** Function name:           statusToBuffer
** Descriptions:            converts CANINTF status to tx buffer index
*********************************************************************************************************/
unsigned char statusToTxBuffer(unsigned char status) {
    switch (status) {
        case MCP_TX0IF : return 0;
        case MCP_TX1IF : return 1;
        case MCP_TX2IF : return 2;
    }

    return 0xff;
}

/*********************************************************************************************************
** Function name:           statusToBuffer
** Descriptions:            converts CANINTF status to tx buffer sidh
*********************************************************************************************************/
unsigned char statusToTxSidh(unsigned char status) {
    switch (status) {
        case MCP_TX0IF : return MCP_TXB0SIDH;
        case MCP_TX1IF : return MCP_TXB1SIDH;
        case MCP_TX2IF : return MCP_TXB2SIDH;
    }

    return 0;
}

/*********************************************************************************************************
** Function name:           txSidhToTxLoad
** Descriptions:            return tx load command according to tx buffer sidh register
*********************************************************************************************************/
unsigned char txSidhToRTS(unsigned char sidh) {
    switch (sidh) {
        case MCP_TXB0SIDH: return MCP_RTS_TX0;
        case MCP_TXB1SIDH: return MCP_RTS_TX1;
        case MCP_TXB2SIDH: return MCP_RTS_TX2;
    }
    return 0;
}

/*********************************************************************************************************
** Function name:           txSidhToTxLoad
** Descriptions:            return tx load command according to tx buffer sidh register
*********************************************************************************************************/
unsigned char txSidhToTxLoad(unsigned char sidh) {
    switch (sidh) {
        case MCP_TXB0SIDH: return MCP_LOAD_TX0;
        case MCP_TXB1SIDH: return MCP_LOAD_TX1;
        case MCP_TXB2SIDH: return MCP_LOAD_TX2;
    }
    return 0;
}

/*********************************************************************************************************
** Function name:           txIfFlag
** Descriptions:            return tx interrupt flag
*********************************************************************************************************/
unsigned char txIfFlag(unsigned char i) {
    switch (i) {
        case 0: return MCP_TX0IF;
        case 1: return MCP_TX1IF;
        case 2: return MCP_TX2IF;
    }
    return 0;
}

/*********************************************************************************************************
** Function name:           txStatusPendingFlag
** Descriptions:            return buffer tx pending flag on status
*********************************************************************************************************/
unsigned char txStatusPendingFlag(unsigned char i) {
    switch (i) {
        case 0: return MCP_STAT_TX0_PENDING;
        case 1: return MCP_STAT_TX1_PENDING;
        case 2: return MCP_STAT_TX2_PENDING;
    }
    return 0xff;
}

/*********************************************************************************************************
** Function name:           mcp2515_reset
** Descriptions:            reset the device
*********************************************************************************************************/
void mcp2515_reset(void) {
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(MCP_RESET);
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
    __delay_ms(10);
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegister
** Descriptions:            read register
*********************************************************************************************************/
unsigned char mcp2515_readRegister(const unsigned char address) {
    unsigned char ret;

    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(MCP_READ);
    WriteSPI(address);
    ret = ReadSPI();
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif

    return ret;
}

/*********************************************************************************************************
** Function name:           mcp2515_readRegisterS
** Descriptions:            read registerS
*********************************************************************************************************/
void mcp2515_readRegisterS(const unsigned char address, unsigned char values[], const unsigned char n) {
    unsigned char i;
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(MCP_READ);
    WriteSPI(address);
    // mcp2515 has auto-increment of address-pointer
    for (i = 0; i < n && i < CAN_MAX_CHAR_IN_MESSAGE; i++) {
        values[i] = ReadSPI();
    }
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegister
** Descriptions:            set register
*********************************************************************************************************/
void mcp2515_setRegister(const unsigned char address, const unsigned char value) {
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(MCP_WRITE);
    WriteSPI(address);
    WriteSPI(value);
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}

/*********************************************************************************************************
** Function name:           mcp2515_setRegisterS
** Descriptions:            set registerS
*********************************************************************************************************/
void mcp2515_setRegisterS(const unsigned char address, const unsigned char values[], const unsigned char n) {
    unsigned char i;
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(MCP_WRITE);
    WriteSPI(address);

    for (i = 0; i < n; i++) {
        WriteSPI(values[i]);
    }
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}

/*********************************************************************************************************
** Function name:           mcp2515_modifyRegister
** Descriptions:            set bit of one register
*********************************************************************************************************/
void mcp2515_modifyRegister(const unsigned char address, const unsigned char mask, const unsigned char data) {
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(MCP_BITMOD);
    WriteSPI(address);
    WriteSPI(mask);
    WriteSPI(data);
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}

/*********************************************************************************************************
** Function name:           mcp2515_readStatus
** Descriptions:            read mcp2515's Status
*********************************************************************************************************/
unsigned char mcp2515_readStatus(void) {
    unsigned char i;
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(MCP_READ_STATUS);
    i = ReadSPI();
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif

    return i;
}

/*********************************************************************************************************
** Function name:           setSleepWakeup
** Descriptions:            Enable or disable the wake up interrupt (If disabled the MCP2515 will not be woken up by CAN bus activity)
*********************************************************************************************************/
void setSleepWakeup(const unsigned char enable) {
    mcp2515_modifyRegister(MCP_CANINTE, MCP_WAKIF, enable ? MCP_WAKIF : 0);
}

/*********************************************************************************************************
** Function name:           sleep
** Descriptions:            Put mcp2515 in sleep mode to save power
*********************************************************************************************************/
unsigned char sleep() {
    if (getMode() != MODE_SLEEP) {
        return mcp2515_setCANCTRL_Mode(MODE_SLEEP);
    } else {
        return CAN_OK;
    }
}

/*********************************************************************************************************
** Function name:           wake
** Descriptions:            wake MCP2515 manually from sleep. It will come back in the mode it was before sleeping.
*********************************************************************************************************/
unsigned char wake() {
    unsigned char currMode = getMode();
    if (currMode != mcpMode) {
        return mcp2515_setCANCTRL_Mode(mcpMode);
    } else {
        return CAN_OK;
    }
}

/*********************************************************************************************************
** Function name:           setMode
** Descriptions:            Sets control mode
*********************************************************************************************************/
unsigned char setMode(const unsigned char opMode) {
    if (opMode != MODE_SLEEP)  
        mcpMode = opMode;
    return mcp2515_setCANCTRL_Mode(opMode);
}

/*********************************************************************************************************
** Function name:           getMode
** Descriptions:            Returns current control mode
*********************************************************************************************************/
unsigned char getMode() {
    return (mcp2515_readRegister(MCP_CANSTAT) & MODE_MASK);
}

/*********************************************************************************************************
** Function name:           mcp2515_setCANCTRL_Mode
** Descriptions:            set control mode
*********************************************************************************************************/
unsigned char mcp2515_setCANCTRL_Mode(const unsigned char newmode) {
    // If the chip is asleep and we want to change mode then a manual wake needs to be done
    // This is done by setting the wake up interrupt flag
    // This undocumented trick was found at https://github.com/mkleemann/can/blob/master/can_sleep_mcp2515.c
    if ((getMode()) == MODE_SLEEP && newmode != MODE_SLEEP) {
        // Make sure wake interrupt is enabled
        unsigned char wakeIntEnabled = (mcp2515_readRegister(MCP_CANINTE) & MCP_WAKIF);
        if (!wakeIntEnabled) {
            mcp2515_modifyRegister(MCP_CANINTE, MCP_WAKIF, MCP_WAKIF);
        }

        // Set wake flag (this does the actual waking up)
        mcp2515_modifyRegister(MCP_CANINTF, MCP_WAKIF, MCP_WAKIF);

        // Wait for the chip to exit SLEEP and enter LISTENONLY mode.

        // If the chip is not connected to a CAN bus (or the bus has no other powered nodes) it will sometimes trigger the wake interrupt as soon
        // as it's put to sleep, but it will stay in SLEEP mode instead of automatically switching to LISTENONLY mode.
        // In this situation the mode needs to be manually set to LISTENONLY.

        if (mcp2515_requestNewMode(MODE_LISTENONLY) != MCP2515_OK) {
            return MCP2515_FAIL;
        }

        // Turn wake interrupt back off if it was originally off
        if (!wakeIntEnabled) {
            mcp2515_modifyRegister(MCP_CANINTE, MCP_WAKIF, 0);
        }
    }

    // Clear wake flag
    mcp2515_modifyRegister(MCP_CANINTF, MCP_WAKIF, 0);

    return mcp2515_requestNewMode(newmode);
}

/*********************************************************************************************************
** Function name:           mcp2515_requestNewMode
** Descriptions:            Set control mode
*********************************************************************************************************/
unsigned char mcp2515_requestNewMode(const unsigned char newmode) {
    int Count = 0;
    // Spam new mode request and wait for the operation  to complete
    while (1) {
        // Request new mode
        // This is inside the loop as sometimes requesting the new mode once doesn't work (usually when attempting to sleep)
        mcp2515_modifyRegister(MCP_CANCTRL, MODE_MASK, newmode);

        unsigned char statReg = mcp2515_readRegister(MCP_CANSTAT);
        if ((statReg & MODE_MASK) == newmode) { // We're now in the new mode
            return MCP2515_OK;
        } else if ( Count > 200) { // Wait no more than 200ms for the operation to complete
            return MCP2515_FAIL;
        } else {
            __delay_ms(1);
            Count++;
        }
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_configRate
** Descriptions:            set baudrate
*********************************************************************************************************/
unsigned char mcp2515_configRate(const unsigned char canSpeed, const unsigned char clock) {
    unsigned char set, cfg1, cfg2, cfg3;
    set = 1;
    switch (clock) {
        case (MCP_16MHz) :
            switch (canSpeed) {
                case (CAN_5KBPS):
                    cfg1 = MCP_16MHz_5kBPS_CFG1;
                    cfg2 = MCP_16MHz_5kBPS_CFG2;
                    cfg3 = MCP_16MHz_5kBPS_CFG3;
                    break;

                case (CAN_10KBPS):
                    cfg1 = MCP_16MHz_10kBPS_CFG1;
                    cfg2 = MCP_16MHz_10kBPS_CFG2;
                    cfg3 = MCP_16MHz_10kBPS_CFG3;
                    break;

                case (CAN_20KBPS):
                    cfg1 = MCP_16MHz_20kBPS_CFG1;
                    cfg2 = MCP_16MHz_20kBPS_CFG2;
                    cfg3 = MCP_16MHz_20kBPS_CFG3;
                    break;

                case (CAN_25KBPS):
                    cfg1 = MCP_16MHz_25kBPS_CFG1;
                    cfg2 = MCP_16MHz_25kBPS_CFG2;
                    cfg3 = MCP_16MHz_25kBPS_CFG3;
                    break;

                case (CAN_31K25BPS):
                    cfg1 = MCP_16MHz_31k25BPS_CFG1;
                    cfg2 = MCP_16MHz_31k25BPS_CFG2;
                    cfg3 = MCP_16MHz_31k25BPS_CFG3;
                    break;

                case (CAN_33KBPS):
                    cfg1 = MCP_16MHz_33kBPS_CFG1;
                    cfg2 = MCP_16MHz_33kBPS_CFG2;
                    cfg3 = MCP_16MHz_33kBPS_CFG3;
                    break;

                case (CAN_40KBPS):
                    cfg1 = MCP_16MHz_40kBPS_CFG1;
                    cfg2 = MCP_16MHz_40kBPS_CFG2;
                    cfg3 = MCP_16MHz_40kBPS_CFG3;
                    break;

                case (CAN_50KBPS):
                    cfg1 = MCP_16MHz_50kBPS_CFG1;
                    cfg2 = MCP_16MHz_50kBPS_CFG2;
                    cfg3 = MCP_16MHz_50kBPS_CFG3;
                    break;

                case (CAN_80KBPS):
                    cfg1 = MCP_16MHz_80kBPS_CFG1;
                    cfg2 = MCP_16MHz_80kBPS_CFG2;
                    cfg3 = MCP_16MHz_80kBPS_CFG3;
                    break;

                case (CAN_83K3BPS):
                    cfg1 = MCP_16MHz_83k3BPS_CFG1;
                    cfg2 = MCP_16MHz_83k3BPS_CFG2;
                    cfg3 = MCP_16MHz_83k3BPS_CFG3;
                    break;

                case (CAN_95KBPS):
                    cfg1 = MCP_16MHz_95kBPS_CFG1;
                    cfg2 = MCP_16MHz_95kBPS_CFG2;
                    cfg3 = MCP_16MHz_95kBPS_CFG3;
                    break;

                case (CAN_100KBPS):
                    cfg1 = MCP_16MHz_100kBPS_CFG1;
                    cfg2 = MCP_16MHz_100kBPS_CFG2;
                    cfg3 = MCP_16MHz_100kBPS_CFG3;
                    break;

                case (CAN_125KBPS):
                    cfg1 = MCP_16MHz_125kBPS_CFG1;
                    cfg2 = MCP_16MHz_125kBPS_CFG2;
                    cfg3 = MCP_16MHz_125kBPS_CFG3;
                    break;

                case (CAN_200KBPS):
                    cfg1 = MCP_16MHz_200kBPS_CFG1;
                    cfg2 = MCP_16MHz_200kBPS_CFG2;
                    cfg3 = MCP_16MHz_200kBPS_CFG3;
                    break;

                case (CAN_250KBPS):
                    cfg1 = MCP_16MHz_250kBPS_CFG1;
                    cfg2 = MCP_16MHz_250kBPS_CFG2;
                    cfg3 = MCP_16MHz_250kBPS_CFG3;
                    break;

                case (CAN_500KBPS):
                    cfg1 = MCP_16MHz_500kBPS_CFG1;
                    cfg2 = MCP_16MHz_500kBPS_CFG2;
                    cfg3 = MCP_16MHz_500kBPS_CFG3;
                    break;

                case (CAN_666KBPS):
                    cfg1 = MCP_16MHz_666kBPS_CFG1;
                    cfg2 = MCP_16MHz_666kBPS_CFG2;
                    cfg3 = MCP_16MHz_666kBPS_CFG3;
                    break;

                case (CAN_800KBPS) :
                    cfg1 = MCP_16MHz_800kBPS_CFG1;
                    cfg2 = MCP_16MHz_800kBPS_CFG2;
                    cfg3 = MCP_16MHz_800kBPS_CFG3;
                    break;

                case (CAN_1000KBPS):
                    cfg1 = MCP_16MHz_1000kBPS_CFG1;
                    cfg2 = MCP_16MHz_1000kBPS_CFG2;
                    cfg3 = MCP_16MHz_1000kBPS_CFG3;
                    break;

                default:
                    set = 0;
                    break;
            }
            break;

        case (MCP_12MHz) :
            switch (canSpeed) {
                case (CAN_20KBPS) :
                    cfg1 = MCP_12MHz_20kBPS_CFG1;
                    cfg2 = MCP_12MHz_20kBPS_CFG2;
                    cfg3 = MCP_12MHz_20kBPS_CFG3;
                    break;

                case (CAN_25KBPS) :
                    cfg1 = MCP_12MHz_25kBPS_CFG1;
                    cfg2 = MCP_12MHz_25kBPS_CFG2;
                    cfg3 = MCP_12MHz_25kBPS_CFG3;
                    break;

                case (CAN_31K25BPS) :
                    cfg1 = MCP_12MHz_31k25BPS_CFG1;
                    cfg2 = MCP_12MHz_31k25BPS_CFG2;
                    cfg3 = MCP_12MHz_31k25BPS_CFG3;
                    break;

                case (CAN_33KBPS) :
                    cfg1 = MCP_12MHz_33kBPS_CFG1;
                    cfg2 = MCP_12MHz_33kBPS_CFG2;
                    cfg3 = MCP_12MHz_33kBPS_CFG3;
                    break;

                case (CAN_40KBPS) :
                    cfg1 = MCP_12MHz_40kBPS_CFG1;
                    cfg2 = MCP_12MHz_40kBPS_CFG2;
                    cfg3 = MCP_12MHz_40kBPS_CFG3;
                    break;

                case (CAN_50KBPS) :
                    cfg1 = MCP_12MHz_50kBPS_CFG1;
                    cfg2 = MCP_12MHz_50kBPS_CFG2;
                    cfg3 = MCP_12MHz_50kBPS_CFG3;
                    break;

                case (CAN_80KBPS) :
                    cfg1 = MCP_12MHz_80kBPS_CFG1;
                    cfg2 = MCP_12MHz_80kBPS_CFG2;
                    cfg3 = MCP_12MHz_80kBPS_CFG3;
                    break;

                case (CAN_83K3BPS) :
                    cfg1 = MCP_12MHz_83k3BPS_CFG1;
                    cfg2 = MCP_12MHz_83k3BPS_CFG2;
                    cfg3 = MCP_12MHz_83k3BPS_CFG3;
                    break;

                case (CAN_95KBPS) :
                    cfg1 = MCP_12MHz_95kBPS_CFG1;
                    cfg2 = MCP_12MHz_95kBPS_CFG2;
                    cfg3 = MCP_12MHz_95kBPS_CFG3;
                    break;

                case (CAN_100KBPS) :
                    cfg1 = MCP_12MHz_100kBPS_CFG1;
                    cfg2 = MCP_12MHz_100kBPS_CFG2;
                    cfg3 = MCP_12MHz_100kBPS_CFG3;
                    break;

                case (CAN_125KBPS) :
                    cfg1 = MCP_12MHz_125kBPS_CFG1;
                    cfg2 = MCP_12MHz_125kBPS_CFG2;
                    cfg3 = MCP_12MHz_125kBPS_CFG3;
                    break;

                case (CAN_200KBPS) :
                    cfg1 = MCP_12MHz_200kBPS_CFG1;
                    cfg2 = MCP_12MHz_200kBPS_CFG2;
                    cfg3 = MCP_12MHz_200kBPS_CFG3;
                    break;

                case (CAN_250KBPS) :
                    cfg1 = MCP_12MHz_250kBPS_CFG1;
                    cfg2 = MCP_12MHz_250kBPS_CFG2;
                    cfg3 = MCP_12MHz_250kBPS_CFG3;
                    break;

                case (CAN_500KBPS) :
                    cfg1 = MCP_12MHz_500kBPS_CFG1;
                    cfg2 = MCP_12MHz_500kBPS_CFG2;
                    cfg3 = MCP_12MHz_500kBPS_CFG3;
                    break;

                case (CAN_666KBPS) :
                    cfg1 = MCP_12MHz_666kBPS_CFG1;
                    cfg2 = MCP_12MHz_666kBPS_CFG2;
                    cfg3 = MCP_12MHz_666kBPS_CFG3;
                    break;

                case (CAN_1000KBPS) :
                    cfg1 = MCP_12MHz_1000kBPS_CFG1;
                    cfg2 = MCP_12MHz_1000kBPS_CFG2;
                    cfg3 = MCP_12MHz_1000kBPS_CFG3;
                    break;

                default:
                    set = 0;
                    break;
            }
            break;
        case (MCP_8MHz) :
            switch (canSpeed) {
                case (CAN_5KBPS) :
                    cfg1 = MCP_8MHz_5kBPS_CFG1;
                    cfg2 = MCP_8MHz_5kBPS_CFG2;
                    cfg3 = MCP_8MHz_5kBPS_CFG3;
                    break;

                case (CAN_10KBPS) :
                    cfg1 = MCP_8MHz_10kBPS_CFG1;
                    cfg2 = MCP_8MHz_10kBPS_CFG2;
                    cfg3 = MCP_8MHz_10kBPS_CFG3;
                    break;

                case (CAN_20KBPS) :
                    cfg1 = MCP_8MHz_20kBPS_CFG1;
                    cfg2 = MCP_8MHz_20kBPS_CFG2;
                    cfg3 = MCP_8MHz_20kBPS_CFG3;
                    break;

                case (CAN_31K25BPS) :
                    cfg1 = MCP_8MHz_31k25BPS_CFG1;
                    cfg2 = MCP_8MHz_31k25BPS_CFG2;
                    cfg3 = MCP_8MHz_31k25BPS_CFG3;
                    break;

                case (CAN_40KBPS) :
                    cfg1 = MCP_8MHz_40kBPS_CFG1;
                    cfg2 = MCP_8MHz_40kBPS_CFG2;
                    cfg3 = MCP_8MHz_40kBPS_CFG3;
                    break;

                case (CAN_50KBPS) :
                    cfg1 = MCP_8MHz_50kBPS_CFG1;
                    cfg2 = MCP_8MHz_50kBPS_CFG2;
                    cfg3 = MCP_8MHz_50kBPS_CFG3;
                    break;

                case (CAN_80KBPS) :
                    cfg1 = MCP_8MHz_80kBPS_CFG1;
                    cfg2 = MCP_8MHz_80kBPS_CFG2;
                    cfg3 = MCP_8MHz_80kBPS_CFG3;
                    break;

                case (CAN_100KBPS) :
                    cfg1 = MCP_8MHz_100kBPS_CFG1;
                    cfg2 = MCP_8MHz_100kBPS_CFG2;
                    cfg3 = MCP_8MHz_100kBPS_CFG3;
                    break;

                case (CAN_125KBPS) :
                    cfg1 = MCP_8MHz_125kBPS_CFG1;
                    cfg2 = MCP_8MHz_125kBPS_CFG2;
                    cfg3 = MCP_8MHz_125kBPS_CFG3;
                    break;

                case (CAN_200KBPS) :
                    cfg1 = MCP_8MHz_200kBPS_CFG1;
                    cfg2 = MCP_8MHz_200kBPS_CFG2;
                    cfg3 = MCP_8MHz_200kBPS_CFG3;
                    break;

                case (CAN_250KBPS) :
                    cfg1 = MCP_8MHz_250kBPS_CFG1;
                    cfg2 = MCP_8MHz_250kBPS_CFG2;
                    cfg3 = MCP_8MHz_250kBPS_CFG3;
                    break;

                case (CAN_500KBPS) :
                    cfg1 = MCP_8MHz_500kBPS_CFG1;
                    cfg2 = MCP_8MHz_500kBPS_CFG2;
                    cfg3 = MCP_8MHz_500kBPS_CFG3;
                    break;

                case (CAN_800KBPS) :
                    cfg1 = MCP_8MHz_800kBPS_CFG1;
                    cfg2 = MCP_8MHz_800kBPS_CFG2;
                    cfg3 = MCP_8MHz_800kBPS_CFG3;
                    break;

                case (CAN_1000KBPS) :
                    cfg1 = MCP_8MHz_1000kBPS_CFG1;
                    cfg2 = MCP_8MHz_1000kBPS_CFG2;
                    cfg3 = MCP_8MHz_1000kBPS_CFG3;
                    break;

                default:
                    set = 0;
                    break;
            }
            break;

        default:
            set = 0;
            break;
    }

    if (set) {
        mcp2515_setRegister(MCP_CNF1, cfg1);
        mcp2515_setRegister(MCP_CNF2, cfg2);
        mcp2515_setRegister(MCP_CNF3, cfg3);
        return MCP2515_OK;
    } else {
        return MCP2515_FAIL;
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_initCANBuffers
** Descriptions:            init canbuffers
*********************************************************************************************************/
void mcp2515_initCANBuffers(void) {
    unsigned char i, a1, a2, a3;

    a1 = MCP_TXB0CTRL;
    a2 = MCP_TXB1CTRL;
    a3 = MCP_TXB2CTRL;
    for (i = 0; i < 14; i++) {                       // in-buffer loop
        mcp2515_setRegister(a1, 0);
        mcp2515_setRegister(a2, 0);
        mcp2515_setRegister(a3, 0);
        a1++;
        a2++;
        a3++;
    }
    mcp2515_setRegister(MCP_RXB0CTRL, 0);
    mcp2515_setRegister(MCP_RXB1CTRL, 0);
}

/*********************************************************************************************************
** Function name:           mcp2515_init
** Descriptions:            init the device
*********************************************************************************************************/
unsigned char mcp2515_init(const unsigned char canSpeed, const unsigned char clock) {

    unsigned char res;

    mcp2515_reset();

    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if (res > 0) {
        #if DEBUG_EN
        SERIAL_PORT_MONITOR.println(F("Enter setting mode fail"));
        #else
        __delay_ms(10);
        #endif
        return res;
    }
    #if DEBUG_EN
    SERIAL_PORT_MONITOR.println(F("Enter setting mode success "));
    #else
    __delay_ms(10);
    #endif

    // set boadrate
    if (mcp2515_configRate(canSpeed, clock)) {
        #if DEBUG_EN
        SERIAL_PORT_MONITOR.println(F("set rate fall!!"));
        #else
        __delay_ms(10);
        #endif
        return res;
    }
    #if DEBUG_EN
    SERIAL_PORT_MONITOR.println(F("set rate success!!"));
    #else
    __delay_ms(10);
    #endif

    if (res == MCP2515_OK) {

        // init canbuffers
        mcp2515_initCANBuffers();

        // interrupt mode
        mcp2515_setRegister(MCP_CANINTE, MCP_RX0IF | MCP_RX1IF);

        #if (DEBUG_RXANY==1)
        // enable both receive-buffers to receive any message and enable rollover
        mcp2515_modifyRegister(MCP_RXB0CTRL,
                               MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                               MCP_RXB_RX_ANY | MCP_RXB_BUKT_MASK);
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                               MCP_RXB_RX_ANY);
        #else
        // enable both receive-buffers to receive messages with std. and ext. identifiers and enable rollover
        mcp2515_modifyRegister(MCP_RXB0CTRL,
                               MCP_RXB_RX_MASK | MCP_RXB_BUKT_MASK,
                               MCP_RXB_RX_STDEXT | MCP_RXB_BUKT_MASK);
        mcp2515_modifyRegister(MCP_RXB1CTRL, MCP_RXB_RX_MASK,
                               MCP_RXB_RX_STDEXT);
        #endif
        // enter normal mode
        res = setMode(MODE_NORMAL);
        if (res) {
            #if DEBUG_EN
            SERIAL_PORT_MONITOR.println(F("Enter Normal Mode Fail!!"));
            #else
            __delay_ms(10);
            #endif
            return res;
        }


        #if DEBUG_EN
        SERIAL_PORT_MONITOR.println(F("Enter Normal Mode Success!!"));
        #else
        __delay_ms(10);
        #endif

    }
    return res;

}

/*********************************************************************************************************
** Function name:           mcp2515_id_to_buf
** Descriptions:            configure tbufdata[4] from id and ext
*********************************************************************************************************/
void mcp2515_id_to_buf(const unsigned char ext, const unsigned long id, unsigned char* tbufdata) {
    uint16_t canid;

    canid = (uint16_t)(id & 0x0FFFF);

    if (ext == 1) {
        tbufdata[MCP_EID0] = (unsigned char)(canid & 0xFF);
        tbufdata[MCP_EID8] = (unsigned char)(canid >> 8);
        canid = (uint16_t)(id >> 16);
        tbufdata[MCP_SIDL] = (unsigned char)(canid & 0x03);
        tbufdata[MCP_SIDL] += (unsigned char)((canid & 0x1C) << 3);
        tbufdata[MCP_SIDL] |= MCP_TXB_EXIDE_M;
        tbufdata[MCP_SIDH] = (unsigned char)(canid >> 5);
    } else {
        tbufdata[MCP_SIDH] = (unsigned char)(canid >> 3);
        tbufdata[MCP_SIDL] = (unsigned char)((canid & 0x07) << 5);
        tbufdata[MCP_EID0] = 0;
        tbufdata[MCP_EID8] = 0;
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_write_id
** Descriptions:            write can id
*********************************************************************************************************/
void mcp2515_write_id(const unsigned char mcp_addr, const unsigned char ext, const unsigned long id) {
    unsigned char tbufdata[4];

    mcp2515_id_to_buf(ext, id, tbufdata);
    mcp2515_setRegisterS(mcp_addr, tbufdata, 4);
}

/*********************************************************************************************************
** Function name:           mcp2515_read_id
** Descriptions:            read can id
*********************************************************************************************************/
void mcp2515_read_id(const unsigned char mcp_addr, unsigned char* ext, unsigned long* id) {
    unsigned char tbufdata[4];

    *ext    = 0;
    *id     = 0;

    mcp2515_readRegisterS(mcp_addr, tbufdata, 4);

    *id = (tbufdata[MCP_SIDH] << 3) + (tbufdata[MCP_SIDL] >> 5);

    if ((tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M) {
        // extended id
        *id = (*id << 2) + (tbufdata[MCP_SIDL] & 0x03);
        *id = (*id << 8) + tbufdata[MCP_EID8];
        *id = (*id << 8) + tbufdata[MCP_EID0];
        *ext = 1;
    }
}

/*********************************************************************************************************
** Function name:           mcp2515_write_canMsg
** Descriptions:            write msg
**                          Note! There is no check for right address!
*********************************************************************************************************/
void mcp2515_write_canMsg(const unsigned char buffer_sidh_addr, unsigned long id, unsigned char ext, unsigned char rtrBit, unsigned char len,
                                   volatile const unsigned char* buf) {
    unsigned char load_addr = txSidhToTxLoad(buffer_sidh_addr);

    unsigned char tbufdata[4];
    unsigned char dlc = len | (rtrBit ? MCP_RTR_MASK : 0) ;
    unsigned char i;

    mcp2515_id_to_buf(ext, id, tbufdata);

    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(load_addr);
    for (i = 0; i < 4; i++) {
        WriteSPI(tbufdata[i]);
    }
    WriteSPI(dlc);
    for (i = 0; i < len && i < CAN_MAX_CHAR_IN_MESSAGE; i++) {
        WriteSPI(buf[i]);
    }

    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif

    mcp2515_start_transmit(buffer_sidh_addr);

}

/*********************************************************************************************************
** Function name:           mcp2515_read_canMsg
** Descriptions:            read message
*********************************************************************************************************/
void mcp2515_read_canMsg(const unsigned char buffer_load_addr, volatile unsigned long* id, volatile unsigned char* ext,
                                  volatile unsigned char* rtrBit, volatile unsigned char* len, volatile unsigned char* buf) {      /* read can msg                 */
    unsigned char tbufdata[4];
    unsigned char i;

    MCP2515_SELECT();
    WriteSPI(buffer_load_addr);
    // mcp2515 has auto-increment of address-pointer
    for (i = 0; i < 4; i++) {
        tbufdata[i] = ReadSPI();
    }

    *id = (tbufdata[MCP_SIDH] << 3) + (tbufdata[MCP_SIDL] >> 5);
    *ext = 0;
    if ((tbufdata[MCP_SIDL] & MCP_TXB_EXIDE_M) ==  MCP_TXB_EXIDE_M) {
        /* extended id                  */
        *id = (*id << 2) + (tbufdata[MCP_SIDL] & 0x03);
        *id = (*id << 8) + tbufdata[MCP_EID8];
        *id = (*id << 8) + tbufdata[MCP_EID0];
        *ext = 1;
    }

    unsigned char pMsgSize = ReadSPI();
    *len = pMsgSize & MCP_DLC_MASK;
    *rtrBit = (pMsgSize & MCP_RTR_MASK) ? 1 : 0;
    for (i = 0; i < *len && i < CAN_MAX_CHAR_IN_MESSAGE; i++) {
        buf[i] = ReadSPI();
    }

    MCP2515_UNSELECT();
}

/*********************************************************************************************************
** Function name:           mcp2515_start_transmit
** Descriptions:            Start message transmit on mcp2515
*********************************************************************************************************/
void mcp2515_start_transmit(const unsigned char mcp_addr) {            // start transmit
    #ifdef SPI_HAS_TRANSACTION
    SPI_BEGIN();
    #endif
    MCP2515_SELECT();
    WriteSPI(txSidhToRTS(mcp_addr));
    MCP2515_UNSELECT();
    #ifdef SPI_HAS_TRANSACTION
    SPI_END();
    #endif
}

/*********************************************************************************************************
** Function name:           mcp2515_isTXBufFree
** Descriptions:            Test is tx buffer free for transmitting
*********************************************************************************************************/
unsigned char mcp2515_isTXBufFree(unsigned char* txbuf_n, unsigned char iBuf) {         /* get Next free txbuf          */
    *txbuf_n = 0x00;

    if (iBuf >= MCP_N_TXBUFFERS ||
            (mcp2515_readStatus() & txStatusPendingFlag(iBuf)) != 0) {
        return MCP_ALLTXBUSY;
    }

    *txbuf_n = txCtrlReg(iBuf) + 1;                                /* return SIDH-address of Buffer */
    mcp2515_modifyRegister(MCP_CANINTF, txIfFlag(iBuf), 0);

    return MCP2515_OK;
}

/*********************************************************************************************************
** Function name:           mcp2515_getNextFreeTXBuf
** Descriptions:            finds next free tx buffer for sending. Return MCP_ALLTXBUSY, if there is none.
*********************************************************************************************************/
unsigned char mcp2515_getNextFreeTXBuf(unsigned char* txbuf_n) {               // get Next free txbuf
    unsigned char status = mcp2515_readStatus() & MCP_STAT_TX_PENDING_MASK;
    unsigned char i;

    *txbuf_n = 0x00;

    if (status == MCP_STAT_TX_PENDING_MASK) {
        return MCP_ALLTXBUSY;    // All buffers are pending
    }

    // check all 3 TX-Buffers except reserved
    for (i = 0; i < MCP_N_TXBUFFERS - nReservedTx; i++) {
        if ((status & txStatusPendingFlag(i)) == 0) {
            *txbuf_n = txCtrlReg(i) + 1;                                   // return SIDH-address of Buffer
            mcp2515_modifyRegister(MCP_CANINTF, txIfFlag(i), 0);
            return MCP2515_OK;                                                 // ! function exit
        }
    }

    return MCP_ALLTXBUSY;
}
/*********************************************************************************************************
** Function name:           begin
** Descriptions:            init can and set speed
*********************************************************************************************************/
unsigned char begin(uint32_t speedset, const unsigned char clockset) {
    OpenSPI(MODE_00, SPI_FOSC_4, SMPEND);
    unsigned char res = mcp2515_init((unsigned char)speedset, clockset);

    return ((res == MCP2515_OK) ? CAN_OK : CAN_FAILINIT);
}

/*********************************************************************************************************
** Function name:           enableTxInterrupt
** Descriptions:            enable interrupt for all tx buffers
*********************************************************************************************************/
void enableTxInterrupt(unsigned char enable) {
    unsigned char interruptStatus = mcp2515_readRegister(MCP_CANINTE);

    if (enable) {
        interruptStatus |= MCP_TX_INT;
    } else {
        interruptStatus &= ~MCP_TX_INT;
    }

    mcp2515_setRegister(MCP_CANINTE, interruptStatus);
}

/*********************************************************************************************************
** Function name:           init_Mask
** Descriptions:            init canid Masks
*********************************************************************************************************/
unsigned char init_Mask(unsigned char num, unsigned char ext, unsigned long ulData) {
    unsigned char res = MCP2515_OK;
    #if DEBUG_EN
    SERIAL_PORT_MONITOR.println(F("Begin to set Mask!!"));
    #else
    __delay_ms(10);
    #endif
    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if (res > 0) {
        #if DEBUG_EN
        SERIAL_PORT_MONITOR.println(F("Enter setting mode fall"));
        #else
        __delay_ms(10);
        #endif
        return res;
    }

    if (num == 0) {
        mcp2515_write_id(MCP_RXM0SIDH, ext, ulData);

    } else if (num == 1) {
        mcp2515_write_id(MCP_RXM1SIDH, ext, ulData);
    } else {
        res =  MCP2515_FAIL;
    }

    res = mcp2515_setCANCTRL_Mode(mcpMode);
    if (res > 0) {
        #if DEBUG_EN
        SERIAL_PORT_MONITOR.println(F("Enter normal mode fall"));
        #else
        __delay_ms(10);
        #endif
        return res;
    }
    #if DEBUG_EN
    SERIAL_PORT_MONITOR.println(F("set Mask success!!"));
    #else
    __delay_ms(10);
    #endif
    return res;
}

/*********************************************************************************************************
** Function name:           init_Filt
** Descriptions:            init canid filters
*********************************************************************************************************/
unsigned char init_Filt(unsigned char num, unsigned char ext, unsigned long ulData) {
    unsigned char res = MCP2515_OK;
    #if DEBUG_EN
    SERIAL_PORT_MONITOR.println(F("Begin to set Filter!!"));
    #else
    __delay_ms(10);
    #endif
    res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
    if (res > 0) {
        #if DEBUG_EN
        SERIAL_PORT_MONITOR.println(F("Enter setting mode fall"));
        #else
        __delay_ms(10);
        #endif
        return res;
    }

    switch (num) {
        case 0:
            mcp2515_write_id(MCP_RXF0SIDH, ext, ulData);
            break;

        case 1:
            mcp2515_write_id(MCP_RXF1SIDH, ext, ulData);
            break;

        case 2:
            mcp2515_write_id(MCP_RXF2SIDH, ext, ulData);
            break;

        case 3:
            mcp2515_write_id(MCP_RXF3SIDH, ext, ulData);
            break;

        case 4:
            mcp2515_write_id(MCP_RXF4SIDH, ext, ulData);
            break;

        case 5:
            mcp2515_write_id(MCP_RXF5SIDH, ext, ulData);
            break;

        default:
            res = MCP2515_FAIL;
    }

    res = mcp2515_setCANCTRL_Mode(mcpMode);
    if (res > 0) {
        #if DEBUG_EN
        SERIAL_PORT_MONITOR.println(F("Enter normal mode fall\r\nSet filter fail!!"));
        #else
        __delay_ms(10);
        #endif
        return res;
    }
    #if DEBUG_EN
    SERIAL_PORT_MONITOR.println(F("set Filter success!!"));
    #else
    __delay_ms(10);
    #endif

    return res;
}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            Send message by using buffer read as free from CANINTF status
**                          Status has to be read with readRxTxStatus and filtered with checkClearTxStatus
*********************************************************************************************************/
unsigned char sendMsgBuf1(unsigned char status, unsigned long id, unsigned char ext, unsigned char rtrBit, unsigned char len, volatile const unsigned char* buf) {
    unsigned char txbuf_n = statusToTxSidh(status);

    if (txbuf_n == 0) {
        return CAN_FAILTX;    // Invalid status
    }

    mcp2515_modifyRegister(MCP_CANINTF, status, 0);  // Clear interrupt flag
    mcp2515_write_canMsg(txbuf_n, id, ext, rtrBit, len, buf);

    return CAN_OK;
}

/*********************************************************************************************************
** Function name:           trySendMsgBuf
** Descriptions:            Try to send message. There is no delays for waiting free buffer.
*********************************************************************************************************/
unsigned char trySendMsgBuf(unsigned long id, unsigned char ext, unsigned char rtrBit, unsigned char len, const unsigned char* buf, unsigned char iTxBuf) {
    unsigned char txbuf_n;

    if (iTxBuf < MCP_N_TXBUFFERS) { // Use specified buffer
        if (mcp2515_isTXBufFree(&txbuf_n, iTxBuf) != MCP2515_OK) {
            return CAN_FAILTX;
        }
    } else {
        if (mcp2515_getNextFreeTXBuf(&txbuf_n) != MCP2515_OK) {
            return CAN_FAILTX;
        }
    }

    mcp2515_write_canMsg(txbuf_n, id, ext, rtrBit, len, buf);

    return CAN_OK;
}

/*********************************************************************************************************
** Function name:           sendMsg
** Descriptions:            send message
*********************************************************************************************************/
unsigned char sendMsg(unsigned long id, unsigned char ext, unsigned char rtrBit, unsigned char len, const unsigned char* buf, unsigned char wait_sent) {
    unsigned char res, res1, txbuf_n;
    uint16_t uiTimeOut = 0;

    can_id = id;
    ext_flg = ext;
    rtr = rtrBit;

    do {
        if (uiTimeOut > 0) {
            __delay_ms(10);
        }
        res = mcp2515_getNextFreeTXBuf(&txbuf_n);                       // info = addr.
        uiTimeOut++;
    } while (res == MCP_ALLTXBUSY && (uiTimeOut < TIMEOUTVALUE));

    if (uiTimeOut == TIMEOUTVALUE) {
        return CAN_GETTXBFTIMEOUT;                                      // get tx buff time out
    }
    mcp2515_write_canMsg(txbuf_n, id, ext, rtrBit, len, buf);

    if (wait_sent) {
        uiTimeOut = 0;
        do {
            if (uiTimeOut > 0) {
                __delay_ms(10);
            }
            uiTimeOut++;
            res1 = mcp2515_readRegister(txbuf_n - 1);  // read send buff ctrl reg
            res1 = res1 & 0x08;
        } while (res1 && (uiTimeOut < TIMEOUTVALUE));

        if (uiTimeOut == TIMEOUTVALUE) {                                     // send msg timeout
            return CAN_SENDMSGTIMEOUT;
        }
    }

    return CAN_OK;

}

/*********************************************************************************************************
** Function name:           sendMsgBuf
** Descriptions:            send buf
*********************************************************************************************************/
unsigned char sendMsgBuf(unsigned long id, unsigned char ext, unsigned char rtrBit, unsigned char len, const unsigned char* buf, unsigned char wait_sent) {
    return sendMsg(id, ext, rtrBit, len, buf, wait_sent);
}

/*********************************************************************************************************
** Function name:           readMsgBufID
** Descriptions:            Read message buf and can bus source ID according to status.
**                          Status has to be read with readRxTxStatus.
*********************************************************************************************************/
unsigned char readMsgBufID(unsigned char status, volatile unsigned long* id, volatile unsigned char* ext, volatile unsigned char* rtrBit,
                           volatile unsigned char* len, volatile unsigned char* buf) {
    unsigned char rc = CAN_NOMSG;

    if (status & MCP_RX0IF) {                                        // Msg in Buffer 0
        mcp2515_read_canMsg(MCP_READ_RX0, id, ext, rtrBit, len, buf);
        rc = CAN_OK;
    } else if (status & MCP_RX1IF) {                                 // Msg in Buffer 1
        mcp2515_read_canMsg(MCP_READ_RX1, id, ext, rtrBit, len, buf);
        rc = CAN_OK;
    }

    if (rc == CAN_OK) {
        rtr = *rtrBit;
        // dta_len=*len; // not used on any interface function
        ext_flg = *ext;
        can_id = *id;
    } else {
        *len = 0;
    }

    return rc;
}

/*********************************************************************************************************
** Function name:           readRxTxStatus
** Descriptions:            Read RX and TX interrupt bits. Function uses status reading, but translates.
**                          result to MCP_CANINTF. With this you can check status e.g. on interrupt sr
**                          with one single call to save SPI calls. Then use checkClearRxStatus and
**                          checkClearTxStatus for testing.
*********************************************************************************************************/
unsigned char readRxTxStatus(void) {
    unsigned char ret = (mcp2515_readStatus() & (MCP_STAT_TXIF_MASK | MCP_STAT_RXIF_MASK));
    ret = (ret & MCP_STAT_TX0IF ? MCP_TX0IF : 0) |
          (ret & MCP_STAT_TX1IF ? MCP_TX1IF : 0) |
          (ret & MCP_STAT_TX2IF ? MCP_TX2IF : 0) |
          (ret & MCP_STAT_RXIF_MASK); // Rx bits happend to be same on status and MCP_CANINTF
    return ret;
}

/*********************************************************************************************************
** Function name:           checkClearRxStatus
** Descriptions:            Return first found rx CANINTF status and clears it from parameter.
**                          Note that this does not affect to chip CANINTF at all. You can use this
**                          with one single readRxTxStatus call.
*********************************************************************************************************/
unsigned char checkClearRxStatus(unsigned char* status) {
    unsigned char ret;

    ret = *status & MCP_RX0IF; *status &= ~MCP_RX0IF;

    if (ret == 0) {
        ret = *status & MCP_RX1IF;
        *status &= ~MCP_RX1IF;
    }

    return ret;
}

/*********************************************************************************************************
** Function name:           checkClearTxStatus
** Descriptions:            Return specified buffer of first found tx CANINTF status and clears it from parameter.
**                          Note that this does not affect to chip CANINTF at all. You can use this
**                          with one single readRxTxStatus call.
*********************************************************************************************************/
unsigned char checkClearTxStatus(unsigned char* status, unsigned char iTxBuf) {
    unsigned char ret;

    if (iTxBuf < MCP_N_TXBUFFERS) { // Clear specific buffer flag
        ret = *status & txIfFlag(iTxBuf); *status &= ~txIfFlag(iTxBuf);
    } else {
        ret = 0;
        for (unsigned char i = 0; i < MCP_N_TXBUFFERS - nReservedTx; i++) {
            ret = *status & txIfFlag(i);
            if (ret != 0) {
                *status &= ~txIfFlag(i);
                return ret;
            }
        };
    }

    return ret;
}

/*********************************************************************************************************
** Function name:           clearBufferTransmitIfFlags
** Descriptions:            Clear transmit interrupt flags for specific buffer or for all unreserved buffers.
**                          If interrupt will be used, it is important to clear all flags, when there is no
**                          more data to be sent. Otherwise IRQ will newer change state.
*********************************************************************************************************/
void clearBufferTransmitIfFlags(unsigned char flags) {
    flags &= MCP_TX_INT;
    if (flags == 0) {
        return;
    }
    mcp2515_modifyRegister(MCP_CANINTF, flags, 0);
}

/*********************************************************************************************************
** Function name:           checkReceive
** Descriptions:            check if got something
*********************************************************************************************************/
unsigned char checkReceive(void) {
    unsigned char res;
    res = mcp2515_readStatus();                                         // RXnIF in Bit 1 and 0
    return ((res & MCP_STAT_RXIF_MASK) ? CAN_MSGAVAIL : CAN_NOMSG);
}

/*********************************************************************************************************
** Function name:           checkError
** Descriptions:            if something error
*********************************************************************************************************/
unsigned char checkError(unsigned char* err_ptr) {
    unsigned char eflg = mcp2515_readRegister(MCP_EFLG);
    if (err_ptr) {
        *err_ptr = eflg;
    }
    return ((eflg & MCP_EFLG_ERRORMASK) ? CAN_CTRLERROR : CAN_OK);
}


/*********************************************************************************************************
** Function name:           mcpPinMode
** Descriptions:            switch supported pins between HiZ, interrupt, output or input
*********************************************************************************************************/
unsigned char mcpPinMode(const unsigned char pin, const unsigned char mode) {
    unsigned char res;
    unsigned char ret = 1;

    switch (pin) {
        case MCP_RX0BF:
            switch (mode) {
                case MCP_PIN_HIZ:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B0BFE, 0);
                    break;
                case MCP_PIN_INT:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B0BFM | B0BFE, B0BFM | B0BFE);
                    break;
                case MCP_PIN_OUT:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B0BFM | B0BFE, B0BFE);
                    break;
                default:
                    #if DEBUG_EN
                    SERIAL_PORT_MONITOR.println(F("Invalid pin mode request"));
                    #endif
                    return 0;
            }
            return 1;
            break;
        case MCP_RX1BF:
            switch (mode) {
                case MCP_PIN_HIZ:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B1BFE, 0);
                    break;
                case MCP_PIN_INT:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B1BFM | B1BFE, B1BFM | B1BFE);
                    break;
                case MCP_PIN_OUT:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B1BFM | B1BFE, B1BFE);
                    break;
                default:
                    #if DEBUG_EN
                    SERIAL_PORT_MONITOR.println(F("Invalid pin mode request"));
                    #endif
                    return 0;
            }
            return 1;
            break;
        case MCP_TX0RTS:
            res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
            if (res > 0) {
                #if DEBUG_EN
                SERIAL_PORT_MONITOR.println(F("Entering Configuration Mode Failure..."));
                #else
                __delay_ms(10);
                #endif
                return 0;
            }
            switch (mode) {
                case MCP_PIN_INT:
                    mcp2515_modifyRegister(MCP_TXRTSCTRL, B0RTSM, B0RTSM);
                    break;
                case MCP_PIN_IN:
                    mcp2515_modifyRegister(MCP_TXRTSCTRL, B0RTSM, 0);
                    break;
                default:
                    #if DEBUG_EN
                    SERIAL_PORT_MONITOR.println(F("Invalid pin mode request"));
                    #endif
                    ret = 0;
            }
            res = mcp2515_setCANCTRL_Mode(mcpMode);
            if (res) {
                #if DEBUG_EN
                SERIAL_PORT_MONITOR.println(F("`Setting ID Mode Failure..."));
                #else
                __delay_ms(10);
                #endif
                return 0;
            }
            return ret;
            break;
        case MCP_TX1RTS:
            res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
            if (res > 0) {
                #if DEBUG_EN
                SERIAL_PORT_MONITOR.println(F("Entering Configuration Mode Failure..."));
                #else
                __delay_ms(10);
                #endif
                return 0;
            }
            switch (mode) {
                case MCP_PIN_INT:
                    mcp2515_modifyRegister(MCP_TXRTSCTRL, B1RTSM, B1RTSM);
                    break;
                case MCP_PIN_IN:
                    mcp2515_modifyRegister(MCP_TXRTSCTRL, B1RTSM, 0);
                    break;
                default:
                    #if DEBUG_EN
                    SERIAL_PORT_MONITOR.println(F("Invalid pin mode request"));
                    #endif
                    ret = 0;
            }
            res = mcp2515_setCANCTRL_Mode(mcpMode);
            if (res) {
                #if DEBUG_EN
                SERIAL_PORT_MONITOR.println(F("`Setting ID Mode Failure..."));
                #else
                __delay_ms(10);
                #endif
                return 0;
            }
            return ret;
            break;
        case MCP_TX2RTS:
            res = mcp2515_setCANCTRL_Mode(MODE_CONFIG);
            if (res > 0) {
                #if DEBUG_EN
                SERIAL_PORT_MONITOR.println(F("Entering Configuration Mode Failure..."));
                #else
                __delay_ms(10);
                #endif
                return 0;
            }
            switch (mode) {
                case MCP_PIN_INT:
                    mcp2515_modifyRegister(MCP_TXRTSCTRL, B2RTSM, B2RTSM);
                    break;
                case MCP_PIN_IN:
                    mcp2515_modifyRegister(MCP_TXRTSCTRL, B2RTSM, 0);
                    break;
                default:
                    #if DEBUG_EN
                    SERIAL_PORT_MONITOR.println(F("Invalid pin mode request"));
                    #endif
                    ret = 0;
            }
            res = mcp2515_setCANCTRL_Mode(mcpMode);
            if (res) {
                #if DEBUG_EN
                SERIAL_PORT_MONITOR.println(F("`Setting ID Mode Failure..."));
                #else
                __delay_ms(10);
                #endif
                return 0;
            }
            return ret;
            break;
        default:
            #if DEBUG_EN
            SERIAL_PORT_MONITOR.println(F("Invalid pin for mode request"));
            #endif
            return 0;
    }
}

/*********************************************************************************************************
** Function name:           mcpDigitalWrite
** Descriptions:            write 1 or 0 to RX0BF/RX1BF
*********************************************************************************************************/
unsigned char mcpDigitalWrite(const unsigned char pin, const unsigned char mode) {
    switch (pin) {
        case MCP_RX0BF:
            switch (mode) {
                case 1:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B0BFS, B0BFS);
                    return 1;
                    break;
                default:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B0BFS, 0);
                    return 1;
            }
            break;
        case MCP_RX1BF:
            switch (mode) {
                case 1:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B1BFS, B1BFS);
                    return 1;
                    break;
                default:
                    mcp2515_modifyRegister(MCP_BFPCTRL, B1BFS, 0);
                    return 1;
            }
            break;
        default:
            #if DEBUG_EN
            SERIAL_PORT_MONITOR.println(F("Invalid pin for mcpDigitalWrite"));
            #endif
            return 0;
    }
}

/*********************************************************************************************************
** Function name:           mcpDigitalRead
** Descriptions:            read 1 or 0 from supported pins
*********************************************************************************************************/
unsigned char mcpDigitalRead(const unsigned char pin) {
    switch (pin) {
        case MCP_RX0BF:
            if ((mcp2515_readRegister(MCP_BFPCTRL) & B0BFS) > 0) {
                return 1;
            } else {
                return 0;
            }
            break;
        case MCP_RX1BF:
            if ((mcp2515_readRegister(MCP_BFPCTRL) & B1BFS) > 0) {
                return 1;
            } else {
                return 0;
            }
            break;
        case MCP_TX0RTS:
            if ((mcp2515_readRegister(MCP_TXRTSCTRL) & B0RTS) > 0) {
                return 1;
            } else {
                return 0;
            }
            break;
        case MCP_TX1RTS:
            if ((mcp2515_readRegister(MCP_TXRTSCTRL) & B1RTS) > 0) {
                return 1;
            } else {
                return 0;
            }
            break;
        case MCP_TX2RTS:
            if ((mcp2515_readRegister(MCP_TXRTSCTRL) & B2RTS) > 0) {
                return 1;
            } else {
                return 0;
            }
            break;
        default:
            #if DEBUG_EN
            SERIAL_PORT_MONITOR.println(F("Invalid pin for mcpDigitalRead"));
            #endif
            return 0;
    }
}

/*********************************************************************************************************
    END FILE
*********************************************************************************************************/
