#include "mcp_can.h"

/*********************************************************************************************************
** Function name:           MCP_CAN
** Descriptions:            Constructor
*********************************************************************************************************/
/*MCP_CAN(unsigned char _CS)
{
    pSPI = &SPI;
    init_CS(_CS);
}

/*********************************************************************************************************
** Function name:           init_CS
** Descriptions:            init CS pin and set UNSELECTED
*********************************************************************************************************/
void init_CS(void)
{
    /*if (_CS == 0)
    {
        return;
    }*/
    _CS_TRIS = 0;
    _CS_PIN = 1;
    
}

/*void setSPI(SPIClass *_pSPI)
{
    pSPI = _pSPI; // define SPI port to use before begin()
}*/
