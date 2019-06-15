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
	Free_block *pr_block_add = 0;			// 2 ����� ��� ����� ����������� ����� � ������ ������ 
	Free_block *next_block_add = 0;			// 2 ����� ��� ����� ���������� ����� � ������ ������
	uint8_t	packet_header[3] = { 0, 0, 0 };	// 3 ����� ��� ��������� ������
	uint8_t frame_header = 0;				// 1 ���� ��� �������� �����
	char information_part[128];				// 128 ���� �������������� �����
	uint16_t CRC = 0;						//2 ����� ��� ����������-����������� ���������� ���
	Free_block() {
		memset(information_part, 0, sizeof(char)*128);
	}
};

struct �haracteristicFB{
public:
	Free_block *First_fb = 0;	//����� ������ ������� ������� ���������� ����� � �������
	Free_block *Last_fb = 0;	//����� ������ ������� ���������� ���������� ����� � �������
	int N1 = 0;
};

struct RRPacket
{
public:
	uint8_t RRframe = 0;
	uint16_t CRC = 0;
};