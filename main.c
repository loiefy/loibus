#include "string.h"
#include "stdio.h"
#include "stdint.h"


typedef unsigned char uint8;
typedef unsigned int uint16;

uint8 databuff[] = " . fasdfskjfl; #W12AB03010203! dcmpqrckstackwahsdkf";
uint8 memcom_hexdec_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
uint8 memcom_hexdec_map2[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};

#define VFX_LOG     printf

#define MEMCOM_CMD_SIZE     64
#define MEMCOM_DATA_SIZE    64
#define E_OK        0
#define E_NOT_OK    1

#define MEMCOM_START_CODE   '#'
#define MEMCOM_END_CODE     '!'


uint8 memcom_cmdbuff_index = 0;
uint8 memcom_commandbuff[MEMCOM_CMD_SIZE];
uint8 memcom_data[MEMCOM_DATA_SIZE];

void memcom_init(void);
void memcom_mainfunction(void);
uint8 memcom_is_ascci_hex(uint8 ch );
uint8 memcom_process_command( uint8 * str );
uint8 memcom_is_cmd_option( uint8 ch );
uint8 memcom_hex_to_1bytedec( uint8 * str );
uint16 memcom_hex_to_2bytesdec( uint8 * str );


uint8 getbyte(uint8 *ch);

int main(void)
{
    printf("Hello people \r\n");
    memcom_init();

    for (int i = 0; i < 65535; i++ )
    {
        memcom_mainfunction();
    }

    getchar();
}



 
void memcom_init()
{

}
 
/* Frame: 
    [START] [Read/Write] [Address] [Length] [byte 0 ... byte n] [STOP] 
 
    where:
    [START] = '#', field length = 1 bytes
    [Read/Write] = 'W' = write, field length = 1 byte
    [Read/Write] = 'R' = read, field length = 1 byte
    [Address] range = [0000..FFFF] show as ASCII, if DEC(address) = 100 then the value show '0064', field length = 4bytes
    [Length] is number of byte to be read/write, range = [00..FF], 
    [byte i] show as ASCII hex, if Dec(value) = 10, then then value send as ASCII hex is '0A'
    [STOP] '!'
*/
 
void memcom_mainfunction(void)
{
    uint8 retval;
    uint8 newchar; 
    while ( E_OK == (retval = getbyte(&newchar)))
    {
        if ( 0 == memcom_cmdbuff_index )
        {
            if ( MEMCOM_START_CODE == newchar)
            {
                /* meet start code characters, begin a frame */
                memcom_commandbuff[memcom_cmdbuff_index ++] = newchar;
            }
        } else
        {
            if ( MEMCOM_END_CODE == newchar )
            {
                /* meet endcode characters, enclose the frame, then process */
                memcom_commandbuff[memcom_cmdbuff_index ++] = newchar;
                memcom_commandbuff[memcom_cmdbuff_index ++] = 0;
                memcom_process_command(memcom_commandbuff);
                VFX_LOG("%s\r\n", memcom_commandbuff);
            }
            else if ( E_OK == memcom_is_ascci_hex(newchar) )
            {
                /* getting frame except START CODE and END CODE */
                memcom_commandbuff[memcom_cmdbuff_index ++] = newchar;
            }
            else if ( E_OK == memcom_is_cmd_option(newchar) )
            {
                /* getting frame except START CODE and END CODE */
                memcom_commandbuff[memcom_cmdbuff_index ++] = newchar;
            }
            else
            {
                /* meet endcode characters */
                memcom_cmdbuff_index = 0;
            }
        }
    }
}

/**
 * return E_OK/E_NOT_OK
*/
uint8 getbyte(uint8 *ch)
{
    static int i = 0;
    i ++;
    if ( i > 100)
    {
        i = 0;
    }

    if ( i < strlen(databuff) )
    {
        *ch = databuff[i];
        return E_OK;
    }
    else 
    {
        return E_NOT_OK;
    }
}

uint8 memcom_is_cmd_option( uint8 ch )
{
    if (( ch == 'w' ) || ( ch == 'W') || ( ch == 'r' ) || ( ch == 'R' ))
    {
        return E_OK;
    }
    return E_NOT_OK;
}

uint8 memcom_is_ascci_hex(uint8 ch )
{
    if ((( ch >= '0' ) && ( ch <= '9')) ||
        (( ch >= 'a' ) && ( ch <= 'z')) ||
        (( ch >= 'A' ) && ( ch <= 'Z')))
    {
        return E_OK;
    }
    return E_NOT_OK;
}

uint8 memcom_process_command( uint8 * str )
{
    uint16 u16address = 0;
    uint8 data[MEMCOM_DATA_SIZE];

    if ( E_OK == memcom_is_cmd_option(str[1]) )
    {
        if (( str[1] == 'W') || ( str[1] == 'w'))
        {
            /* This is write command */
            u16address = memcom_hex_to_2bytesdec(str + 2);
            if ( u16address >= MEMCOM_DATA_SIZE )
            {
                VFX_LOG("%s, address %d out of memory \r\n", __FUNCTION__, u16address);
                return E_NOT_OK;
            }
        } else
        {
            /* This is read command */
        }
    }
    return E_NOT_OK;
}

uint8 memcom_hex_to_1bytedec( uint8 * str )
{
    uint8 u8msb = 0;
    uint8 u8lsb = 0;

    for ( uint8 i = 0; i < 16; i++ )
    {
        if (( str[0] == memcom_hexdec_map[i] ) || ( str[0] == memcom_hexdec_map2[i] ))
        {
            u8msb = i;
        }
        if (( str[1] == memcom_hexdec_map[i] ) || ( str[1] == memcom_hexdec_map2[i] ))
        {
            u8lsb = i;
        }
    }
    return (u8msb << 4) | (u8lsb & 0x0F);
}

uint16 memcom_hex_to_2bytesdec( uint8 * str )
{
    uint16 u16msb = memcom_hex_to_1bytedec( str );
    uint16 u16lsb = memcom_hex_to_1bytedec( str + 2);

    return (u16msb << 8) | ( u16lsb & 0x00FF);
}


