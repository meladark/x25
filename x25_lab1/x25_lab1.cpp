// x25_lab1.cpp : Программа формирвоания и передачи в канал связи одного информационного кадра
//

#include "pch.h"
#include "Header.h"
#include <iostream>
#include <cstdint>
#include <string>
#include <stdio.h>

void transfer(СharacteristicFB *out = NULL, СharacteristicFB *in = NULL, int Count = 1) {
	if (in == NULL) {
		out->First_fb = out->First_fb->next_block_add;
		out->First_fb->pr_block_add = 0;
		return;
	}
	for (int i = 0; i < Count; i++) {
		if (in->Last_fb != nullptr) {
			in->Last_fb->next_block_add = out->First_fb;
			out->First_fb->pr_block_add = in->Last_fb;		
		}
		else {
			in->First_fb = out->First_fb;
		}
		in->Last_fb = out->First_fb;
		out->First_fb = out->First_fb->next_block_add;
		if (!out->First_fb) out->Last_fb = 0;
		in->N1 += 1;
		out->N1 -= 1;
	}
	in->Last_fb->next_block_add = 0;
}

//формирование очереди из N1 свободных блоков
void P1(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree) {
	int N1 = Cvar.N1;
	fBlocks[0].next_block_add = &fBlocks[1];
	fBlocks[N1].pr_block_add = &fBlocks[N1 - 1];
	memset(fBlocks[0].information_part, 0, sizeof(char) * 128);
	memset(fBlocks[N1].information_part, 0, sizeof(char) * 128);
	for (int i = 1; i < Cvar.N1 - 1; i++) {
		fBlocks[i].pr_block_add = &fBlocks[i - 1];
		fBlocks[i].next_block_add = &fBlocks[i + 1];
		memset(fBlocks[i].information_part, 0, sizeof(char) * 128);
	}
	Hfree->First_fb = &fBlocks[0];
	Hfree->Last_fb = &fBlocks[N1 - 1];
	Hfree->N1 = N1;
}

//формирование N2 пакетов данных
int P2(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, char *massage, int size) {
	Free_block *inUse;
	inUse = Hfree->First_fb;
	int offset = 0;
	while (inUse != Hfree->Last_fb)
	{
		for (int i = 0; i < 128; i++) {
			if (i < size) {
				inUse->information_part[i] = massage[i + offset];
			}
			else {
				return 0;
			}
		}
		size -= 128;
		offset += 128;
		inUse = inUse->next_block_add;
	}
	return -1;
}

//перенос N2 пакетов данных из очереди Освоб в очередь пакетов Оп32;
void P3(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32) {
	int N2 = Cvar.N2;
	transfer(Hfree, Hp32, N2);
}

//формирование информационного кадра, включающего первый пакет в очереди пакетов Оп32
void P4(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32, int ilab3 = 0) {
	int VS = Cvar.Z1;
	int VR = Cvar.Z2;
	if (ilab3 == 0) {
		//Hp32->First_fb->pr_block_add = 0;
		Hp32->First_fb->frame_header = VR << 5;
		Hp32->First_fb->frame_header += VS << 1;
		Hp32->First_fb->frame_header &= 238;
		uint8_t CRC = 0;
		for (int i = 0; i < 128; i++) {
			CRC ^= Hp32->First_fb->information_part[i];
		}
		Hp32->First_fb->CRC = CRC << 8;
		Hp32->First_fb->CRC += (uint8_t)(Hp32->First_fb->frame_header ^ Cvar.m + 1);
	}
	else if (ilab3 == 1) {
		Free_block *fb = Hp32->First_fb;
		for (int i = 0; i < Cvar.MCICL; i++) {
			fb->frame_header = VR << 5;
			fb->frame_header += VS << 1;
			fb->frame_header &= 238;
			uint8_t CRC = 0;
			for (int i = 0; i < 128; i++) {
				CRC ^= fb->information_part[i];
			}
			fb->CRC = CRC << 8;
			fb->CRC += (uint8_t)(fb->frame_header ^ Cvar.m + 1);
			fb = fb->next_block_add;
			VS++;
		}
	}

}

//перенос информационного кадра, сформированного программой P4, в очередь повтора Оповт и в регистр на передачу в канал.
Free_block P5(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32, СharacteristicFB *Hrep, int MCICL = 1) {
	transfer(Hp32, Hrep, MCICL);
	Free_block *fb = Hp32->First_fb;
	fb->pr_block_add = 0;
	fb->next_block_add = 0;
	return *fb;
}

//формирование принятого кадра “RR”, подтверждающего правильный прием переданного на противоположную сторону информационного кадра “I”(cм.лаб. 1).Проверка безошибочного приема кадра RR с канала связи;
RRPacket P6(int RREJ = 0) {
	RRPacket RGin;
	if (RREJ == 0) {
		RGin.CRC = 97;
		RGin.RRframe = 97;
	}
	else if (RREJ == 1) {
		RGin.CRC = 101;
		RGin.RRframe = 101;
	}
	return RGin;
}

//запись этого кадра RR с контрольно-проверочной комбинацией КПК в первый блок очереди Освоб
void P7(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, RRPacket RGin){
	Hfree->First_fb->CRC = RGin.CRC;
	Hfree->First_fb->frame_header = RGin.RRframe;
}

//перенос принятого кадра RR из Освоб в очередь Окпм;
void P8(СharacteristicFB *Hfree, СharacteristicFB *Hkpm) {
	transfer(Hfree, Hkpm);
}

//проверка правильного приема переданного ранее кадра “I” и находящегося в очереди повтора Оповт;
void P9(СharacteristicFB *Hrep, СharacteristicFB *Hkpm, int iform = 0) {
	uint8_t NSRR = (Hkpm->First_fb->frame_header & 224) >> 5;
	uint8_t NSI = (Hrep->First_fb->frame_header & 14) >> 1;
	if (NSRR - 1 == NSI) return;
	else std::cout << "OSHIBKA";
}
int P11(СharacteristicFB *Hfree, СharacteristicFB *Hrep, int iform);
Free_block P12(СharacteristicFB *Hfree, СharacteristicFB *Hrep, Free_block *fb);
//считывание кадров “RR” из очереди Окпм и “I” из Оповт и установка их в очередь Освоб;
void P10(СharacteristicFB *Hfree, СharacteristicFB *Hkpm, СharacteristicFB *Hrep, int iform = 0) {
	if (iform == 0) {
		Hkpm->First_fb->CRC = 0;
		Hkpm->First_fb->frame_header = 0;
		transfer(Hkpm, Hfree);
		transfer(Hrep, Hfree);
	}
	else if (iform == 1) {
		transfer(Hkpm, Hfree);
		uint8_t NS = (Hrep->First_fb->frame_header >> 1) & 7;
		uint8_t NSp = (Hfree->Last_fb->frame_header & 224) >> 5;
		Free_block *fb = Hrep->First_fb->next_block_add;
		while (fb != NULL) {
			if (NS < NSp) P11(Hfree, Hrep, 1);
			else 
				P12(Hfree, Hrep, fb);
			NS = (fb->frame_header >> 1) & 7;
			fb = fb->next_block_add;
		}
	}
}

//установление режима передачи очередного информационного кадра “I” в канал.
int P11(СharacteristicFB *Hfree = NULL, СharacteristicFB *Hrep = NULL, int iform = 0) {
	if (iform == 0) return 1;
	transfer(Hrep, Hfree);
	return 0;
}

void printing_FB(Free_block *fb, int inform, int form);
//программа стирания кадра (кадров) “I” с очереди повтора Оповт.
Free_block P12(СharacteristicFB *Hfree, СharacteristicFB *Hrep, Free_block *fb) {
	std::cout << "\n\nПоследний кадр Освоб" << std::endl;
	printing_FB(Hfree->Last_fb, 1, 0);
	Free_block *_fb = Hrep->First_fb;
	int i = 1;
	while (_fb != NULL) {
		std::cout << "\n\nКадр Оповт " << i << std::endl;
		i++;
		printing_FB(_fb, 1, 0);
		_fb = _fb->next_block_add;
	}
	std::cout << "\n\nРегистр RGвых " << std::endl;
	printing_FB(fb, 1, 0);
	return *fb;
}
void printing_FB(Free_block *fb, int inform = 0, int form = 0) {

	std::cout << "\nАдресс предыдущего блока" << std::endl;
	if (form == 0 && fb != NULL) {
		for (int i = sizeof(fb->pr_block_add) * 8 - 1; i >= 0; i--) {
			std::cout << (((1 << i) & (uint32_t)fb->pr_block_add) ? '1' : '0');
			if (i % 8 == 0) std::cout << " ";
		}
	}
	else if (form == 1 && fb != NULL) {
		std::cout << (uint32_t)fb->pr_block_add;
	}
	else if (fb == NULL) std::cout << 0;

	std::cout << "\nАдресс следуюшего блока" << std::endl;
	if (form == 0 && fb != NULL) {
		for (int i = sizeof(fb->next_block_add) * 8 - 1; i >= 0; i--) {
			std::cout << (((1 << i) & (uint32_t)fb->next_block_add) ? '1' : '0');
			if (i % 8 == 0) std::cout << " ";
		}
	}
	else if (form == 1 && fb != NULL) {
		std::cout << (uint32_t)fb->next_block_add;
	}
	else if (fb == NULL) std::cout << 0;

	std::cout << "\nЗаголовок фрейма" << std::endl;
	if (form == 0) {
		for (int i = sizeof(fb->frame_header) * 8 - 1; i >= 0; i--) {
			std::cout << (((1 << i) & fb->frame_header) ? '1' : '0');
			if (i % 8 == 0) std::cout << " ";
		}
		std::cout << "\nNS " << ((fb->frame_header >> 1) & 7);
	}
	else if (form == 1) {
		std::cout << (uint32_t)fb->frame_header;
		std::cout << "\nNS " << ((fb->frame_header >> 1) & 7);
	}

	std::cout << "\nЗаголовок пакета" << std::endl;
	if (form == 0) {
		for (int j = 0; j < 3; j++) {
			for (int i = sizeof(fb->packet_header[j]) * 8 - 1; i >= 0; i--) {
				std::cout << (((1 << i) & fb->packet_header[j]) ? '1' : '0');
				if (i % 8 == 0) std::cout << " ";
			}
		}
	}
	else if (form == 1) {
		for (int j = 0; j < 3; j++) {
			std::cout << (uint32_t)fb->packet_header[j];
		}
	}
	if (inform == 1) {

		std::cout << "\nИнформационная часть" << std::endl;
		if (form == 0) {
			for (int j = 0; j < 128; j++) {
				for (int i = sizeof(fb->information_part[j]) * 8 - 1; i >= 0; i--) {
					std::cout << (((1 << i) & fb->information_part[j]) ? '1' : '0');
					if (i % 8 == 0) std::cout << " ";
				}
			}
		}
		else if (form == 1) {
			for (int j = 0; j < 128; j++) {
				std::cout << (uint32_t)fb->information_part[j];
			}
		}

		std::cout << "\nCRC" << std::endl;
		if (form == 0) {
			for (int i = sizeof(fb->CRC) * 8 - 1; i >= 0; i--) {
				std::cout << (((1 << i) & fb->CRC) ? '1' : '0');
				if (i % 8 == 0) std::cout << " ";
			}
		}
		else if (form == 1) {
			std::cout << fb->CRC;
		}
	}
}
//Программа формирования и передачи в канал связи одного информационного кадра
void lab1(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32, СharacteristicFB *Hrep, СharacteristicFB *Hkpm, int &Regim){
	Free_block *fBlocks_in_lab1 = new Free_block[Cvar.N1];
	СharacteristicFB *Hfree_in_lab1 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hp32_in_lab1 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hrep_in_lab1 = new СharacteristicFB{ 0, 0, 0 };
	fBlocks_in_lab1 = fBlocks;
	Hfree_in_lab1 = Hfree;
	Hp32_in_lab1 = Hp32;
	Hrep_in_lab1 = Hrep;
	std::string temp = "Слава тебе, безысходная боль!\nУмер вчера сероглазый король.\n\nВечер осенний был душен и ал,\nМуж мой, вернувшись, спокойно сказал:\n\n«Знаешь, с охоты его принесли,\nТело у старого дуба нашли.\n\nЖаль королеву. Такой молодой!..\nЗа ночь одну она стала седой».\n\nТрубку свою на камине нашел\nИ на работу ночную ушел.\n\nДочку мою я сейчас разбужу,\nВ серые глазки ее погляжу.\n\nА за окном шелестят тополя:\n«Нет на земле твоего короля…»\n\n\nДвадцать первое. Ночь. Понедельник.\nОчертанья столицы во мгле.\nСочинил же какой - то бездельник,\nЧто бывает любовь на земле.\n\nИ от лености или со скуки\nВсе поверили, так и живут:\nЖдут свиданий, боятся разлуки\nИ любовные песни поют.\n\nНо иным открывается тайна,\nИ почиет на них тишина\nЯ на это наткнулась случайно\nИ с тех пор все как будто больна.";
	char mas[1024];
	strncpy_s(mas, temp.c_str(), sizeof(mas));
	mas[sizeof(mas) - 1] = 0;
	for (int i = 0; i < 1024; i++) {
		mas[i] = Cvar.m + 1;
	}
	P1(Cvar, fBlocks_in_lab1, Hfree_in_lab1);
	P2(Cvar, fBlocks_in_lab1, Hfree_in_lab1, mas, temp.length());
	P3(Cvar, fBlocks_in_lab1, Hfree_in_lab1, Hp32_in_lab1);
	P4(Cvar, fBlocks_in_lab1, Hfree_in_lab1, Hp32_in_lab1);
	Free_block RGout = P5(Cvar, fBlocks_in_lab1, Hfree_in_lab1, Hp32_in_lab1, Hrep_in_lab1);
	//print_this_shit(Cvar, fBlocks);
	std::cout << "Результаты Лабораторной работы 1" << std::endl;
	std::cout << "\nВыходной регистр" << std::endl;
	printing_FB(&RGout, 1);
	return;
}

//Программа приема c канала кадра “RR”
void lab2(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32, СharacteristicFB *Hrep, СharacteristicFB *Hkpm, int &Regim) {
	Free_block *fBlocks_in_lab2 = new Free_block[Cvar.N1];
	СharacteristicFB *Hfree_in_lab2 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hp32_in_lab2 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hrep_in_lab2 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hkpm_in_lab2 = new СharacteristicFB{ 0,0,0 };
	int REGIM = 0;
	fBlocks_in_lab2 = fBlocks;
	Hfree_in_lab2 = Hfree;
	Hp32_in_lab2 = Hp32;
	Hrep_in_lab2 = Hrep;
	Hkpm_in_lab2 = Hkpm;
	RRPacket RGin = P6();
	P7(Cvar, fBlocks_in_lab2, Hfree_in_lab2, RGin);
	P8(Hfree_in_lab2, Hkpm_in_lab2);
	P9(Hrep_in_lab2, Hkpm_in_lab2);
	P10(Hfree_in_lab2, Hkpm_in_lab2, Hrep_in_lab2);
	Regim = P11();
	std::cout << "\nРезультаты Лабораторной работы 2" << std::endl;
	std::cout << "\nПакет I" << std::endl;
	printing_FB(Hfree_in_lab2->Last_fb, 1);
	std::cout << "\n\nПакет RR" << std::endl;
	printing_FB(Hfree_in_lab2->Last_fb->pr_block_add, 1);
	return;
}

//Программа передачи в канал связи нескольких информационных кадров
void lab3(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32, СharacteristicFB *Hrep, СharacteristicFB *Hkpm, int &Regim) {
	Free_block *fBlocks_in_lab3 = new Free_block[Cvar.N1];
	СharacteristicFB *Hfree_in_lab3 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hp32_in_lab3 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hrep_in_lab3 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hkpm_in_lab3 = new СharacteristicFB{ 0, 0, 0 };
	fBlocks_in_lab3 = fBlocks;
	Hfree_in_lab3 = Hfree;
	Hp32_in_lab3 = Hp32;
	Hrep_in_lab3 = Hrep;
	Hkpm_in_lab3 = Hkpm;
	char mas[1024];
	mas[sizeof(mas) - 1] = 0;
	for (int i = 0; i < 1024; i++) {
		mas[i] = Cvar.m + 1;
	}
	P1(Cvar, fBlocks_in_lab3, Hfree_in_lab3);
	P2(Cvar, fBlocks_in_lab3, Hfree_in_lab3, mas, 700);
	P3(Cvar, fBlocks_in_lab3, Hfree_in_lab3, Hp32_in_lab3);
	P4(Cvar, fBlocks_in_lab3, Hfree_in_lab3, Hp32_in_lab3, 1);
	Free_block RGout = P5(Cvar, fBlocks_in_lab3, Hfree_in_lab3, Hp32_in_lab3, Hrep_in_lab3, Cvar.MCICL);
	std::cout << "\nРезультаты Лабораторной работы 3" << std::endl;
	std::cout << "\nРегистр вывода 1";
	printing_FB(&RGout, 1);
	std::cout << "\n";
	printing_FB(&RGout, 1, 1);
	Free_block *fb = Hrep_in_lab3->First_fb;
	for (int i = 0; i < Cvar.MCICL; i++) {
		std::cout << "\n\nОповт " << i + 1 << std::endl;
		printing_FB(fb, 1);
		std::cout << "\n";
		printing_FB(fb, 1, 1);
		fb = fb->next_block_add;
	}
	return;
}

//Программа приема c канала кадра “REJ”
void lab4(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32, СharacteristicFB *Hrep, СharacteristicFB *Hkpm, int &Regim) {
	Free_block *fBlocks_in_lab4 = new Free_block[Cvar.N1];
	СharacteristicFB *Hfree_in_lab4 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hkpm_in_lab4 = new СharacteristicFB{ 0,0,0 };
	Hkpm_in_lab4 = Hkpm;
	fBlocks_in_lab4 = fBlocks;
	Hfree_in_lab4 = Hfree;
	RRPacket RGin = P6(1);
	std::cout << "\n\nРезультаты Лабораторной работы 4\n" << std::endl;
	std::cout << "REJ до отправки в Освоб\nЗаголовк кадра\n";
	for (int i = sizeof(RGin.RRframe) * 8 - 1; i >= 0; i--) {
		std::cout << (((1 << i) & (uint8_t)RGin.RRframe) ? '1' : '0');
		if (i % 8 == 0) std::cout << " ";
	}
	std::cout << "\nNS " << ((RGin.RRframe >> 5) & 7);
	std::cout << "\nCRC\n";
	for (int i = sizeof(RGin.CRC) * 8 - 1; i >= 0; i--) {
		std::cout << (((1 << i) & (uint8_t)RGin.CRC) ? '1' : '0');
		if (i % 8 == 0) std::cout << " ";
	}
	P7(Cvar, fBlocks_in_lab4, Hfree_in_lab4, RGin);
	P8(Hfree_in_lab4, Hkpm_in_lab4);
	Regim = 2;
	P9(Hfree_in_lab4, Hkpm_in_lab4);
}

//Программа передачи в канал кадров “I” c очереди повтора Оповт или стирания кадров из этой очереди при приеме кадра REG
void lab5(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree, СharacteristicFB *Hp32, СharacteristicFB *Hrep, СharacteristicFB *Hkpm, int &Regim) {
	Free_block *fBlocks_in_lab5 = new Free_block[Cvar.N1];
	СharacteristicFB *Hfree_in_lab5 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hp32_in_lab5 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hrep_in_lab5 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hkpm_in_lab5 = new СharacteristicFB{ 0,0,0 };
	Hkpm_in_lab5 = Hkpm;
	fBlocks_in_lab5 = fBlocks;
	Hfree_in_lab5 = Hfree;
	Hp32_in_lab5 = Hp32;
	Hrep_in_lab5 = Hrep;
	std::cout << "\n\nРезультаты Лабораторной работы 5";
	P10(Hfree, Hkpm, Hrep, 1);
}

int main()
{
	setlocale(LC_ALL, "Russian");
	int Regim = 0;
	Const_variables Cvar = { 20, 8, 2, 1, 1, 3};
	Free_block *fBlocks = new Free_block[Cvar.N1];
	СharacteristicFB *Hfree = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hp32 = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hrep = new СharacteristicFB{ 0, 0, 0 };
	СharacteristicFB *Hkpm = new СharacteristicFB{ 0,0,0 };
	lab1(Cvar, fBlocks, Hfree, Hp32, Hrep, Hkpm, Regim);
	lab2(Cvar, fBlocks, Hfree, Hp32, Hrep, Hkpm, Regim);
	lab3(Cvar, fBlocks, Hfree, Hp32, Hrep, Hkpm, Regim);
	lab4(Cvar, fBlocks, Hfree, Hp32, Hrep, Hkpm, Regim);
	lab5(Cvar, fBlocks, Hfree, Hp32, Hrep, Hkpm, Regim);
	return 143;
}
