// x25_lab1.cpp : Программа формирвоания и передачи в канал связи одного информационного кадра
//

#include "pch.h"
#include "Header.h"
#include <iostream>
#include <cstdint>
#include <string>
#include <stdio.h>

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
	Hfree->Last_fb = &fBlocks[N1];
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
void P3(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree) {
	int N2 = Cvar.N2;
	СharacteristicFB *Hp32 = new СharacteristicFB{ &fBlocks[0], &fBlocks[N2], N2 };
	fBlocks[N2 - 1].next_block_add = 0;
	Hfree->First_fb = &fBlocks[N2];
	Hfree->N1 -= N2;
}

//формирование информационного кадра, включающего первый пакет в очереди пакетов Оп32
void P4(Const_variables Cvar, Free_block *fBlocks, СharacteristicFB *Hfree) {

}

void print_this_shit(Const_variables Cvar, Free_block *fBlocks) {
	setlocale(LC_ALL, "Russian");
	for (int i = 0; i < Cvar.N1; i++) {
		std::cout << fBlocks[i].information_part;
	}
}
int main()
{
	Const_variables Cvar = { 14, 6, 1, 3, 3 };
	Free_block *fBlocks = new Free_block[Cvar.N1];
	СharacteristicFB *Hfree = new СharacteristicFB{ 0, 0, 0 };
	std::string temp = "Слава тебе, безысходная боль!\nУмер вчера сероглазый король.\n\nВечер осенний был душен и ал,\nМуж мой, вернувшись, спокойно сказал:\n\n«Знаешь, с охоты его принесли,\nТело у старого дуба нашли.\n\nЖаль королеву. Такой молодой!..\nЗа ночь одну она стала седой».\n\nТрубку свою на камине нашел\nИ на работу ночную ушел.\n\nДочку мою я сейчас разбужу,\nВ серые глазки ее погляжу.\n\nА за окном шелестят тополя:\n«Нет на земле твоего короля…»\n\n\nДвадцать первое. Ночь. Понедельник.\nОчертанья столицы во мгле.\nСочинил же какой - то бездельник,\nЧто бывает любовь на земле.\n\nИ от лености или со скуки\nВсе поверили, так и живут:\nЖдут свиданий, боятся разлуки\nИ любовные песни поют.\n\nНо иным открывается тайна,\nИ почиет на них тишина\nЯ на это наткнулась случайно\nИ с тех пор все как будто больна.";
	char mas[1024];
	strncpy_s(mas, temp.c_str(), sizeof(mas));
	mas[sizeof(mas) - 1] = 0;
	P1(Cvar, fBlocks, Hfree);
	P2(Cvar, fBlocks, Hfree, mas, temp.length());
	P3(Cvar, fBlocks, Hfree);
	print_this_shit(Cvar, fBlocks);
}
