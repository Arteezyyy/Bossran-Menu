#pragma once

#include <vector>
#include <array>      // <-- kailangan
#include <thread>
#include <chrono>
#include <cstring> 

bool ReadBytes(DWORD address, BYTE* buffer, SIZE_T size) {
	memcpy(buffer, (void*)address, size);
	return true;
}

//=====[DWORD HANDLER]=====//
DWORD drones;
DWORD wallnames;
DWORD fscrolls;
DWORD fogs;
DWORD tagos1;
DWORD tagos2;
DWORD nocdrevive;
DWORD nocdrevive2;
DWORD lawak1;
DWORD lawak2;
DWORD lawak3;
DWORD autopkAddress = 0x0267BB35; // Replace with the actual address for autopk UPDATED TO? OO LAHAT NG ADDRESS UPDATED
DWORD invibypass1;
DWORD inviaddress = 0x02679931; // Replace with the actual address for invisibility bypass
DWORD petaddress = 0x0267AC2E; // Replace with the actual address for pet
DWORD TAXIHACK;
DWORD SPBPHACK;
DWORD PYTELEHACK;
DWORD SPACEaddress = 0x00AFC3C5; // Replace with the actual address for space
DWORD lrbuss;
DWORD SKILLHEX = 0x0267ABD4; // Replace with the actual address for skill hex	
DWORD skillbypass1 = 0x00617096; // Replace with the actual address for skill bypass 3
DWORD skillbypass2 = 0x00580F17; // Replace with the actual address for skill bypass 2
DWORD skillbypass3 = 0x006170A2; // Replace with the actual address for skill bypass 6
DWORD skillbypass4 = 0x00580F1C; // Replace with the actual address for skill bypass 6
DWORD skillbypass5 = 0x00617099; //  Replace with the actual address for skill bypass 3
DWORD skillbypass6 = 0x00580F25; // eplace with the actual address for skill bypass 6
DWORD skillbypass7 = 0x0061709C; // eplace with the actual address for skill bypass 3 
DWORD skillbypass8 = 0x00580F2E; // eplace with the actual address for skill bypass 6
DWORD skillbypass9 = 0x0061709F; // eplace 00591355 the actual address for skill bypass 3
DWORD skillbypass10 = 0x00580F38; // eplace with the actual address for skill bypass 6
DWORD skillbypass11 = 0x006170F9; // eplace with the actual address for skill bypass 3
DWORD vindmodeaddress = 0x0058AC46; // Replace with the actual address for vindmode
DWORD petcdbyps;
DWORD bankcdbyps;
DWORD bikecdbyps;
DWORD slotcdbyps;
DWORD selftarget1;
DWORD selftarget2;


//=====[BOOL FUNCTION]=====//
static bool mata = false;
static bool wallneym = false;
static bool fscroll = false;
static bool fog = false;
static bool bohay = false;
static bool tagoswall = false;
static bool lawakpepe = false;
static bool autored = false; // Autopk toggle
static bool tago = false;
static bool isFrozen = false; // Flag to check if value is frozen
static bool isTyping = false; // Flag to check if the user is typing
static float frozenValue = 0.0f;  // Store the frozen value that will be updated only by the user
static bool bilispet = false; // Flag to check if pet speed is enabled	
static bool pyheck = false; // Flag to check if pyhack is enabled
static bool space = false; // Flag to check if space is enabled
static bool buslayo = false; // Flag to check if buslayo is enabled
static bool antingongo = false; // Flag to check if antidog is enabled
static bool isOn = false;
static bool isToggled = false;
static bool started = false;
static bool vend = false; // Flag to check if vendmode is enabled
static bool adminUnlocked = false;
static char inputPassword[64] = "";
static bool passwordFetched = false;
static std::string dropboxPassword;
static bool selftarget = false;


void WriteNOPs(DWORD address, int length) {
	BYTE* nops = new BYTE[length];
	memset(nops, 0x90, length);
	WriteBytes(address, nops, length);
	delete[] nops;
}

//=====[SCRIPT CODE]=====//
//AOE//
static float aoeval = 0.0f;
DWORD jmpbackaoe;
void __declspec(naked) AOECave()
{
	__asm
	{
		fsub dword ptr[aoeval]
		fcomp dword ptr[esp + 0x0C]
		fnstsw ax
		jmp[jmpbackaoe]
	}
}
//LR//
static float lrval = 0.0f;
DWORD jmpbacklr;
void __declspec(naked) LRCave()
{
	__asm
	{
		fsqrt
		fsub dword ptr[lrval]
		fstp dword ptr[esp + 0x38]
		jmp[jmpbacklr]
	}
}
//AS//
static float asval1 = 1.0f;
DWORD jmpbackas1;
void __declspec(naked) AS1Cave()
{
	__asm
	{

		mov ecx, esi
		fmul dword ptr [asval1]
		fstp dword ptr [esp+0x10]
		jmp[jmpbackas1]
	}
}


static float asval2 = 0.0f;
DWORD jmpbackas2;
void __declspec(naked) ASCave2()
{
	__asm
	{
		fadd dword ptr[asval2]
		fmul dword ptr[ecx + 0x00008b8C]
		jmp[jmpbackas2]
	}
}
//CS//
static float csval = 0.0f;
DWORD jmpbackcs;
void __declspec(naked) CSCave()
{
	__asm
	{
		fmul dword ptr[esp + 0x10]
		fadd dword ptr[csval]
		mov ecx, esi
		jmp[jmpbackcs]
	}
}


//=======[SCRIPT CAVING]=======//
void ScriptCaving()
{

	////=======[AOE SCRIPT]=======//
	/*DWORD aoeaddr = SigScan((char*)"minia.exe", (char*)"\xD8\x5C\x24\x0C\xDF\xE0\xF6\xC4\x05\x7A\x06\xB8", (char*)"xxxxxxxxxxxx");
	int aoeLength2 = 6;
	jmpbackaoe = aoeaddr + aoeLength2;
	script((void*)aoeaddr, AOECave, aoeLength2);*/

	//=======[LR SCRIPT]=======//
	DWORD lraddr = SigScan((char*)"minia.exe", (char*)"\xD9\xFA\xD9\x5C\x24\x38\xFF", (char*)"xxxxxxx");
	int lrLength2 = 6;
	jmpbacklr = lraddr + lrLength2;
	script((void*)lraddr, LRCave, lrLength2);

	//=======[AS SCRIPT]=======//
	DWORD as1addr = SigScan((char*)"minia.exe", (char*)"\x8B\xCE\xD9\x5C\x24\x10\xE8\xE1", (char*)"xxxxxxxx");
	int asLength2 = 6; //ILANG PITIK
	jmpbackas1 = as1addr + asLength2;
	script((void*)as1addr, AS1Cave, asLength2);

	//=======[CS SCRIPT]=======//
	//DWORD csaddr = SigScan((char*)"minia.exe", (char*)"\xD8\x4C\x24\x10\x8B\xCE", (char*)"xxxxxx");
	//int csLength2 = 6;
	//jmpbackcs = csaddr + csLength2;
	//script((void*)csaddr, CSCave, csLength2);

	//=======[ASVAL2 SCRIPT]=======//
	//DWORD as2addr = SigScan((char*)"minia.exe", (char*)"\xD8\x0D\xB4\xB2\x80\x00\xC3", (char*)"xxxxxxx");
	//int as2part = 6;
	//jmpbackas2 = as2addr + csLength2;
	//script((void*)as2addr, ASCave2, as2part);-
	drones = SigScan((char*)"minia.exe", (char*)"\x75\x09\x8B\x4E\x20\x89", (char*)"xxxxxx");
	wallnames = SigScan((char*)"minia.exe", (char*)"\x74\x23\x8B\x4C\x24\x34\x51\x8D", (char*)"xxxxxxxx");
	lawak1 = SigScan((char*)"minia.exe", (char*)"\xD8\x0D\x9C\xFD\x81\x00\x52\x8B", (char*)"xxxxxxxx");
	lawak2 = SigScan((char*)"minia.exe", (char*)"\x33\xC0\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x83\xEC", (char*)"xxxxxxxxxxxxxxxxx");
	lawak3 = SigScan((char*)"minia.exe", (char*)"\xD8\x05\xEC\xEA\x81\x00\xD9\x5C\x24\x44", (char*)"xxxxxxxxxx");
	fscrolls = SigScan((char*)"minia.exe", (char*)"\x00\x00\xA0\x42\x35\xFA\x0E\x3C", (char*)"xxxxxxxx");
	fogs = SigScan((char*)"minia.exe", (char*)"\x00\x00\x2F\x44\x00\x00\x1B", (char*)"xxxxxxx");
	invibypass1 = SigScan((char*)"minia.exe", (char*)"\xC6\x86\xD1\x8A\x00\x00\x00\xC6\x86\xD2\x8A\x00\x00\x00\x89\xAE", (char*)"xxxxxxxxxxxxxxxx");
	nocdrevive = SigScan((char*)"minia.exe", (char*)"\x20\x41\xC6\x86\xA5\x01", (char*)"xxxxxx");
	nocdrevive2 = SigScan((char*)"minia.exe", (char*)"\x80\x3F\xC7\x86\x98\x01\x00\x00\x00\x00\x20\x41\xC6", (char*)"xxxxxxxxxxxxx");
	tagos1 = SigScan((char*)"minia.exe", (char*)"\xD9\x44\x24\x3C\x8B\x40\x08", (char*)"xxxxxxx");
	tagos2 = SigScan((char*)"minia.exe", (char*)"\xD9\x44\x24\x18\x8B\x43\x10", (char*)"xxxxxxx");
	TAXIHACK = SigScan((char*)"minia.exe", (char*)"\x38\x58\x3C\x75\x34\x53", (char*)"xxxxxx");
	PYTELEHACK = SigScan((char*)"minia.exe", (char*)"\x8A\x48\x3C\x84\xC9\x75\x4D\x6A", (char*)"xxxxxxxx");
	SPBPHACK = SigScan((char*)"minia.exe", (char*)"\x8A\x48\x3C\x84\xC9\x75\x4C\x6A\x00\x68\xC0\x2A\x83\x00\xE8\xE1", (char*)"xxxxxxxxxxxxxxxx");
	lrbuss = SigScan((char*)"minia.exe", (char*)"\x8B\x4C\x24\x14\x8D\x74\x08\x1E", (char*)"xxxxxxxx");




	//=======[PETCD BYPASS]=======//
	DWORD petcdbyps = SigScan((char*)"minia.exe", (char*)"\xD8\x1D\xEC\xEA\x81\x00\xDF\xE0\xF6\xC4\x05\x7A\x3A\x6A\x00", (char*)"xxxxxxxxxxxxxxx");
	BYTE PETCDBP[5] = { 0xD8, 0x1D, 0x1D, 0x00, 0xFD };
	WriteBytes(petcdbyps, PETCDBP, 5);

	////=======[bankCD BYPASS]=======//
	DWORD bankcdbyps = SigScan((char*)"minia.exe", (char*)"\xD8\x1D\xEC\xEA\x81\x00\xDF\xE0\xF6\xC4\x01", (char*)"xxxxxxxxxxx");
	BYTE BANKCDBP[5] = { 0xD8, 0x1D, 0x1D, 0x00, 0xFD };
	WriteBytes(bankcdbyps, BANKCDBP, 5);

	////=======[VEHICECD BYPASS]=======//
	DWORD bikecdbyps = SigScan((char*)"minia.exe", (char*)"\xD8\x1D\xEC\xEA\x81\x00\xDF\xE0\xF6\xC4\x05\x7A\x2E\x6A", (char*)"xxxxxxxxxxxxxx");
	BYTE BIKECDBP[5] = { 0xD8, 0x1D, 0x1D, 0x00, 0xFD };
	WriteBytes(bikecdbyps, BIKECDBP, 5);


	////=======[SLOTCD BYPASS]=======//
	DWORD slotcdbyps = SigScan((char*)"minia.exe", (char*)"\x01\x75\x28\x56\x8B\xCB\xE8", (char*)"xxxxxxx");
	BYTE SLOTCDBP[1] = { 0x00 };
	WriteBytes(slotcdbyps, SLOTCDBP, 1);

	

	WriteNOPs(skillbypass1, 3);
	WriteNOPs(skillbypass2, 2);
	WriteNOPs(skillbypass3, 6);
	WriteNOPs(skillbypass4, 6);
	WriteNOPs(skillbypass5, 3);
	WriteNOPs(skillbypass6, 6);
	WriteNOPs(skillbypass7, 3);
	WriteNOPs(skillbypass8, 6);
	WriteNOPs(skillbypass9, 3);
	WriteNOPs(skillbypass10, 6);
	WriteNOPs(skillbypass11, 3);

}


// CHECK BOX
void BooleanFunct()
{

	if (antingongo)
	{
		if (!started) {
			started = true;

			std::thread([]() {
				DWORD addresses[] = {
					0x02678E54, 0x02678EFC, 0x02678FA4, 0x0267904C, 0x026790F4,
					0x0267919C, 0x02679244, 0x02679244, 0x02679394, 0x0267943C,
					0x026794E4, 0x0267958C, 0x02679634, 0x026796DC ,0x02679784
				};

				std::vector<std::array<BYTE, 4>> targetValues = {
					{ 0x38, 0x00, 0x26, 0x00 }, // dog hex
					{ 0x21, 0x00, 0x4B, 0x00 }, // extreme dance
					{ 0x2B, 0x00, 0x13, 0x00 }, // assassin box
					{ 0x38, 0x00, 0x19, 0x00 }, // rad
					{ 0x38, 0x00, 0x15, 0x00 }, // debuffs
					{ 0x2F, 0x00, 0x12, 0x00 }, //MAGICIAN BOX
				};

				BYTE replacementValue[4] = { 0xFF, 0xFF, 0xFF, 0xFF };

				while (true) {
					for (DWORD address : addresses) {
						BYTE buffer[4];
						memcpy(buffer, (void*)address, 4);

						for (const auto& target : targetValues) {
							if (memcmp(buffer, target.data(), 4) == 0) {
								memcpy((void*)address, replacementValue, 4);
								break;
							}
						}
					}
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
				}).detach();
		}
	}




	////======[BUSLAYO]======//
	if (buslayo)
	{
		BYTE busbytes[4] = { 0x90,0x90,0x90,0x90 };
		WriteBytes(lrbuss, busbytes, 4);
	}
	else
	{
		BYTE busbytes[4] = { 0x8b,0x4c,0x24,0x14 };
		WriteBytes(lrbuss, busbytes, 4);
	}


	////======[SPACE]======//
	if (space)
	{
		BYTE spacebytes[1] = { 0x32 };
		WriteBytes(SPACEaddress, spacebytes, 1);
	}


	////======[PYHACK]======//
	if (pyheck)
	{
		BYTE pewayhack[3] = { 0x90,0x90,0x90 };
		WriteBytes(TAXIHACK, pewayhack, 3);

		BYTE peway2hack[3] = { 0x90,0x90,0x90 };
		WriteBytes(SPBPHACK, peway2hack, 3);

		BYTE peway3hack[3] = { 0x90,0x90,0x90 };
		WriteBytes(PYTELEHACK, peway3hack, 3);
	}
	else
	{
		BYTE pewayhack[3] = { 0x38,0x58,0x3C };
		WriteBytes(TAXIHACK, pewayhack, 3);

		BYTE peway2hack[3] = { 0x8a,0x48,0x3c };
		WriteBytes(SPBPHACK, peway2hack, 3);

		BYTE peway3hack[3] = { 0x8a,0x48,0x3c };
		WriteBytes(PYTELEHACK, peway3hack, 3);

	}
	//======[thruwall]======//	

	if (tagoswall)
	{
		BYTE wall1thr[7] = { 0x90,0x90,0x90,0x90,0x90,0x90,0x90 };
		WriteBytes(tagos1, wall1thr, 7);

		BYTE wall2thr[7] = { 0x90,0x90,0x90,0x90,0x90,0x90,0x90 };
		WriteBytes(tagos2, wall2thr, 7);

	}
	else
	{
		BYTE wall1thr[7] = { 0xD9,0x44,0x24,0x3c,0x8B,0x40,0x08 };
		WriteBytes(tagos1, wall1thr, 7);

		BYTE wall2thr[7] = { 0xD9,0x44,0x24,0x18,0x8B,0x43,0x10 };
		WriteBytes(tagos2, wall2thr, 7);

	}

	////======[revive]======//
	if (bohay)
	{
		BYTE revbytes[2] = { 0x80 , 0xBF };
		WriteBytes(nocdrevive, revbytes, 2);
		BYTE revbytes2[2] = { 0x80 , 0xBF };
		WriteBytes(nocdrevive2, revbytes2, 2);



	}
	else
	{
		BYTE revbytes[2] = { 0x20 , 0x41 };
		WriteBytes(nocdrevive, revbytes, 2);
		BYTE revbytes2[2] = { 0x80, 0x3F };
		WriteBytes(nocdrevive2, revbytes2, 2);
	}

	//======[VENDMODE]======//

	if (vend)
	{
		BYTE vind[1] = { 0xEB };
		WriteBytes(vindmodeaddress, vind, 1);



	}
	else
	{
		BYTE vind[1] = { 0x74 };
		WriteBytes(vindmodeaddress, vind, 1);
	}


	//======[PETSPEED]======//	
	if (bilispet)
	{
		BYTE petbytes[2] = { 0x50,0x46 };
		WriteBytes(petaddress, petbytes, 2);



	}
	else
	{
		BYTE petbytes[2] = { 0xC8,0x42 };
		WriteBytes(petaddress, petbytes, 2);

	}


	//======[INVISIBILITY]======//	

	if (tago)
	{
		BYTE tagoBYTES[7] = { 0x90,0x90,0x90,0x90,0x90,0x90,0x90 };
		WriteBytes(invibypass1, tagoBYTES, 7);

		BYTE tagoBYTES2[1] = { 0x01 };
		WriteBytes(inviaddress, tagoBYTES2, 1);


	}
	else
	{
		BYTE tagoBYTES[7] = { 0xC6,0x86,0xD1,0x8A,0x00,0x00,0x00 };
		WriteBytes(invibypass1, tagoBYTES, 7);
		BYTE tagoBYTES2[1] = { 0x00 };
		WriteBytes(inviaddress, tagoBYTES2, 1);
	}

	//======[AOE]======//
	if (lawakpepe)
	{
		BYTE AOEBYTES[6] = { 0x90,0x90,0x90,0x90,0x90,0x90 };
		WriteBytes(lawak1, AOEBYTES, 6);

		BYTE AOEBYTES2[2] = { 0x90,0x90 };
		WriteBytes(lawak2, AOEBYTES2, 2);

		BYTE AOEBYTES3[6] = { 0x90,0x90,0x90,0x90,0x90,0x90 };
		WriteBytes(lawak3, AOEBYTES3, 6);
	}
	else
	{
		BYTE AOEBYTES[6] = { 0xD8,0x0D, 0x9C, 0xfd, 0x81, 0x00 };
		WriteBytes(lawak1, AOEBYTES, 6);

		BYTE AOEBYTES2[2] = { 0x33, 0xC0 };
		WriteBytes(lawak2, AOEBYTES2, 2);

		BYTE AOEBYTES3[6] = { 0xD8, 0x05, 0xeC, 0xfa, 0x81, 0x00 };
		WriteBytes(lawak3, AOEBYTES3, 6);
	}

	//======[DRONE]======//
	if (mata)
	{
		BYTE DRNBYTES[1] = { 0xEB };
		WriteBytes(drones, DRNBYTES, 1);
	}
	else
	{
		BYTE DRNBYTES[1] = { 0x75 };
		WriteBytes(drones, DRNBYTES, 1);
	}

	//======[WALLNAME]======//
	if (wallneym)
	{
		BYTE WNBYTES[1] = { 0xEB };
		WriteBytes(wallnames, WNBYTES, 1);
	}
	else
	{
		BYTE WNBYTES[1] = { 0x74 };
		WriteBytes(wallnames, WNBYTES, 1);

	}

	//======[FAST SCROLL]======//
	if (fscroll)
	{
		BYTE FSBYTES[4] = { 0x00, 0x00, 0x96, 0x43 };
		WriteBytes(fscrolls, FSBYTES, 4);
	}
	else
	{
		BYTE FSBYTES[4] = { 0x00, 0x00, 0xA0, 0x42 };
		WriteBytes(fscrolls, FSBYTES, 4);
	}

	//======[REMOVE FOG]======//
	if (fog)
	{
		BYTE FGBYTES[4] = { 0x00, 0x24, 0x74, 0x49 };
		WriteBytes(fogs, FGBYTES, 4);
	}
	else
	{
		BYTE FGBYTES[4] = { 0x00, 0x00, 0x2F, 0x44 };
		WriteBytes(fogs, FGBYTES, 4);
	}

	//======[AUTOPK TOGGLE]======//
	if (autored) {
		// If autopk is active, write 0x01 to the address
		BYTE PKBytes[1] = { 0x01 }; // Activating autopk
		WriteBytes(autopkAddress, PKBytes, 1);
	}
	else {
		// If autopk is inactive, write 0x00 to the address
		BYTE PKBytes[1] = { 0x00 }; // Deactivating autopk
		WriteBytes(autopkAddress, PKBytes, 1);
	}



}
