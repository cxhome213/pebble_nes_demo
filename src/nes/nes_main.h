#ifndef _NES_MAIN_H_
#define _NES_MAIN_H_
#include "nes/sys.h"    
#include <stdio.h>
#include <string.h>   
#include "nes/6502.h"
#include "nes/PPU.h"
#include "nes/nes_joypad.h"	 
#include "nes/nes_rom.h"
#include <pebble.h>
//#include "guix.h"
//////////////////////////////////////////////////////////////////////////////////	 
//²Î¿¼×ÔÍøÓÑ:xiaowei061µÄNESÄ£ÄâÆ÷£¬ÔÙ´Ë¶Ôxiaowei061±íÊ¾¸ÐÐ»£¡
//ALIENTEKÕ½½¢STM32¿ª·¢°å
//NESÄ£ÄâÆ÷ ´úÂë	   
//ÕýµãÔ­×Ó@ALIENTEK
//¼¼ÊõÂÛÌ³:www.openedv.com
//ÐÞ¸ÄÈÕÆÚ:2012/10/3
//°æ±¾£ºV1.0		       								  
////////////////////////////////////////////////////////////////////////////////// 	   
		 
							 										  
typedef struct
{
	char filetype[4]; 	//×Ö·û´®¡°NES^Z¡±ÓÃÀ´Ê¶±ð.NESÎÄ¼þ 		 
	u8 romnum;			//16kB ROMµÄÊýÄ¿ 						 
	u8 vromnum;			//8kB VROMµÄÊýÄ¿				 
	u8 romfeature;		//D0£º1£½´¹Ö±¾µÏñ£¬0£½Ë®Æ½¾µÏñ 
						// D1£º1£½ÓÐµç³Ø¼ÇÒä£¬SRAMµØÖ·$6000-$7FFF
						// D2£º1£½ÔÚ$7000-$71FFÓÐÒ»¸ö512×Ö½ÚµÄtrainer 
						// D3£º1£½4ÆÁÄ»VRAM²¼¾Ö 
						//  D4£­D7£ºROM MapperµÄÍ4» 	  
	u8 rommappernum;	// D0£­D3£º±£Áô£¬±ØÐëÊÇ0£¨×¼±¸×÷Îª¸±MapperºÅ^_^£©
						// D4£­D7£ºROM MapperµÄ¸ß4Î» 		    
	//u8 reserve[8];	// ±£Áô£¬±ØÐëÊÇ0 					    
	//OM¶ÎÉýÐòÅÅÁÐ£¬Èç¹û´æÔÚtrainer£¬ËüµÄ512×Ö½Ú°ÚÔÚROM¶ÎÖ®Ç° 
	//VROM¶Î, ÉýÐòÅÅÁÐ 
}NesHeader;																		 

u8 nes_main(GContext *ctx);
void NesFrameCycle(GContext *ctx);
void NES_ReadJoyPad(u8 JoyPadNum);


//PPUÊ¹ÓÃ
extern u8 *NameTable;			//2KµÄ±äÁ¿
extern u32	*Buffer_scanline;	//ÐÐÏÔÊ¾»º´æ,ÉÏÏÂ±êÔ½½ç×î´óÎª7£¬ÏÔÊ¾Çø 7 ~ 263  0~7 263~270 Îª·ÀÖ¹Òç³öÇø
//CPUÊ¹ÓÃ
extern u8 *ram6502;  			//RAM  2K×Ö½Ú,ÓÉmallocÉêÇë

u8 nes_mem_creat(void);		//¿ª±ÙnesÔËÐÐËùÐèµÄRAM.
void nes_mem_delete(void);	//É¾³ýnesÔËÐÐÊ±ÉêÇëµÄRAM	
										 
#endif











