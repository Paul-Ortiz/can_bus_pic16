/*
 * File:   main.c
 * Author: Paul
 *
 * Created on December 29, 2023, 1:45 PM
 */


//#include <xc.h>
#include "mcp_can.h"
#include "mcp2515_can.h"
#pragma config WDTE = OFF //
#pragma config CPD = OFF // EEPROM
#pragma config CP = OFF  // CODE PORTECTION
#pragma config FOSC = HS
//#define _XTAL_FREQ 4000000 // 4Mhz init in "mcp_can.h"

#define LED RD3


unsigned char stmp[8] = {1, 1, 2, 3, 0, 5, 6, 7};

unsigned char readMsgBuf(unsigned char *len, unsigned char *buf) {
        return readMsgBufID(readRxTxStatus(), &can_id, &ext_flg, &rtr, len, buf);
    }

unsigned long getCanId(void) { return can_id; }

void main(void) {

    TRISDbits.TRISD3 = 0;

    LED = 0; //Initial condition
    
    init_CS();  // init chip select 
    
    while (CAN_OK != begin(CAN_500KBPS, MCP_8MHz)) {             // init can bus : baudrate = 500k
        __delay_ms(100);
    }
    
    while(1){       
        // send data:  id = 0x04, standard frame, rtrBit = 0, data len = 8, stmp: data buf

        sendMsgBuf(0x04, 0, 0, 8, stmp, 1);
        __delay_ms(1000);

        
        // receive data
        unsigned char len = 0;
        unsigned char buf[8];

        if (CAN_MSGAVAIL == checkReceive()) {         // check if data coming
            readMsgBuf(&len, buf);    // read data,  len: data length, buf: data buf

            unsigned long canId = getCanId();

            for (int i = 0; i < len; i++) { // print the data

                if ((canId == 0x00000005) & (buf[i] == 0x72)){
                    LED =1;
                    __delay_ms(100);
                    LED = 0;
                    __delay_ms(100);
                }

            }
        }
        
    }
    return;
}

