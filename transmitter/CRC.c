
#include "xil_types.h"
#include "crc.h"

static u16 crcTable[256];

/*
 * Brief	:	Reflect a 16-bit value about the center bit
 * Return	:	Reflected value
 * Parameter: 	value to be reflected and the num of bits for reflection
 */
u16 Reflect(u16 val, u8 nbits)
{
	u8 i;
    u16 resVal = 0;

//	Reflect the data about the center bit.
    for (i = 0; i < nbits; ++i)
    {
        if ((val & (1 << i)) != 0)
        {
            resVal |= (u16)(1 << ((nbits-1) - i));
        }
    }

    return resVal;
}

/*
 * Brief	:	Calculate table for CRC looking up
 * MUST called ONCE (offline mode)
 * Return	:	none
 * Parameter: 	none
 */
void CalculateTable_CRC16()
{
	u8 bit;
	unsigned int divident;
	u16 curByte;

	// iterate over all possible input byte values 0 - 255
	for (divident = 0; divident < 256; ++divident)
    {
		// move divident byte into MSB of 16Bit CRC
        curByte = (u16)(divident << 8);

        for (bit = 0; bit < 8; ++bit)
        {
            if ((curByte & 0x8000) != 0)
            {
                curByte <<= 1;
                curByte ^= POLYNOMIAL;
            }
            else
            {
                curByte <<= 1;
            }
        }

        crcTable[divident] = curByte;
    }
}

/*
 * Brief	:	Calculate CRC by looking up to the table which
 * is initially pre-computed once
 * Return	:	CRC 16-bit value
 * Parameter: 	frame (characters) and the length of them
 */
u16 Fast_Lookup_CRC16(u8 const frame[], unsigned int len)
{
    u16 crc = INITIAL_CRC;
    unsigned int byte;
    u8 pos;

    for (byte = 0; byte < len; ++byte)
    {
        // XOR next input byte into MSB of crc -> new divident
        pos = (u8)( (crc >> 8) ^ ( (u8)Reflect(frame[byte], 8) ) );

        // Shift out the MSB used for division per lookup-table and XOR with the crc(remainder)
        crc = ( (crc << 8) ^ crcTable[pos] );
    }

    return Reflect(crc, WIDTH);
}
