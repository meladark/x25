#pragma once
#include <cstdint>
#include <string.h>

struct Const_variables{
public:
	const int N1;
	const int N2;
	const int Z1;
	const int Z2;
	const int m;
};

struct Free_block {
public:
	Free_block *pr_block_add = 0;			// 2 байта под адрес предыдущего блока в списке блоков 
	Free_block *next_block_add = 0;			// 2 байта под адрес следующего блока в списке блоков
	uint8_t	packet_header[3] = { 0, 0, 0 };	// 3 байта под заголовок пакета
	uint8_t frame_header = 0;				// 1 байт под заголовк кадра
	char information_part[128];				// 128 байт информационной части
	uint16_t CRC = 0;						//2 байта под контрольно-проверочную комбинацию КПК
	Free_block() {
		memset(information_part, 0, sizeof(char)*128);
	}
};

struct СharacteristicFB{
public:
	Free_block *First_fb = 0;	//адрес начала массива первого свободного блока в очереди
	Free_block *Last_fb = 0;	//адрес начала массива последнего свободного блока в очереди
	int N1 = 0;
};

struct RRPacket
{
public:
	uint8_t RRframe = 0;
	uint16_t CRC = 0;
};