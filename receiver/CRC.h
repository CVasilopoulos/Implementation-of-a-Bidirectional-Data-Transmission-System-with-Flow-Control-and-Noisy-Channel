
#ifndef CRC_H_
#define CRC_H_

#include "xil_types.h"

#define WIDTH    			16
#define POLYNOMIAL			0x8005
#define INITIAL_CRC			0x800D

u16 Reflect(u16 val, u8 nbits);
void CalculateTable_CRC16();
u16 Fast_Lookup_CRC16(u8 const frame[], unsigned int len);





#endif /* CRC_H_ */
