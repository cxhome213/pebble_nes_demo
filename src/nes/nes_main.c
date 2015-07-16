#include "nes/nes_main.h"
#include "nes/nes_rom.h"
#include "stdlib.h"
//////////////////////////////////////////////////////////////////////////////////	 
//�ο�������:xiaowei061��NESģ�������ٴ˶�xiaowei061��ʾ��л��
//ALIENTEKս��STM32������
//NESģ���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/10/3
//�汾��V1.0		       								  
////////////////////////////////////////////////////////////////////////////////// 	   

extern Layer *s_canvas_layer;

u8 nesruning = 0;		//��ʼ��Ϊ��
u8 frame_cnt = 0;		 	//ͳ��֡��

//NES ֡����ѭ�� 
void NesFrameCycle(GContext *ctx)
{
 	int	clocks;	//CPUִ��ʱ��	 
	//����ģ����ѭ�������VROM���ã�������Ϊ0����ʼ��VROM
	//if ( NesHeader.byVRomSize == 0)
	//����VROM�洢��λ�á�
	//frame_cnt=0;
	nesruning=1;
	//while(nesruning)
	{
 		//scanline: 0~19 VBANK �Σ���PPUʹ��NMI��������NMI �ж�
		frame_cnt++;//֡������		    
		SpriteHitFlag=FALSE;		
		for(PPU_scanline=0; PPU_scanline<20; PPU_scanline++)
		{ 
			exec6502(CLOCKS_PER_SCANLINE);
 		}
		//scanline: 20, PPU�������ã�ִ��һ�οյ�ɨ��ʱ�� 
		exec6502(CLOCKS_PER_SCANLINE);
		//NesHBCycle();  //ˮƽ����
		PPU_scanline++;	  //20++
		PPU_Reg.NES_R2 &= ~R2_SPR0_HIT;
		//scanline: 21~261 	
		
		for(; PPU_scanline < SCAN_LINE_DISPALY_END_NUM; PPU_scanline++)
		{
			
			if((SpriteHitFlag == TRUE) && ((PPU_Reg.NES_R2 & R2_SPR0_HIT) == 0))
			{
				clocks = sprite[0].x * CLOCKS_PER_SCANLINE / NES_DISP_WIDTH;
				exec6502(clocks);
				PPU_Reg.NES_R2 |= R2_SPR0_HIT;
				exec6502(CLOCKS_PER_SCANLINE - clocks);
			}else exec6502(CLOCKS_PER_SCANLINE); 

			if(PPU_Reg.NES_R1 & (R1_BG_VISIBLE | R1_SPR_VISIBLE))//��Ϊ�٣��ر���ʾ
			{
				if(SpriteHitFlag == FALSE)
				NES_GetSpr0HitFlag(PPU_scanline - SCAN_LINE_DISPALY_START_NUM);//����Sprite #0 ��ײ��־
			}

			//if((frame_cnt%3)==0)//ÿ2֡��ʾһ��
			{
				NES_RenderLine(PPU_scanline - SCAN_LINE_DISPALY_START_NUM, ctx);//ˮƽͬ������ʾһ��
			}	
			
		}

		//layer_mark_dirty(s_canvas_layer);

		//scanline: 262 ���һ֡ 
		exec6502(CLOCKS_PER_SCANLINE);
		PPU_Reg.NES_R2 |= R2_VBlank_Flag;//����VBANK ��־
		//��ʹ��PPU VBANK�жϣ�������VBANK 
		if(PPU_Reg.NES_R0 & R0_VB_NMI_EN)
		{
			NMI_Flag = SET1;//���һ֡ɨ�裬����NMI�ж�
		}  
	   	//����֡IRQ��־��ͬ����������APU�� 		   
		//A mapper function in V-Sync �洢���л���ֱVBANKͬ�� 
		//MapperVSync();  
		//��ȡ������JoyPad״̬,����JoyPad������ֵ
		//NES_JoyPadUpdateValue();	 //systick �ж϶�ȡ����ֵ   
	}

} 
//����ֵ:0,ִ��OK;
//	  ����,�������
u8 nes_main(GContext *ctx)
{	
	u16 offset=0;
	u8 res;
	NesHeader *neshreader = (NesHeader *) rom_file;
 	//����NES�ļ�ͷ������0x1A��������0x1a��Ctrl+Z,����ģ���ļ������ķ��ţ�
	//����ʹ��strcncmp�Ƚ�ǰ3���ֽڡ�   
	if(strncmp(neshreader->filetype, "NES", 3) != 0)
	{
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n�ļ�δ���ػ��ļ����ʹ���, NESģ�����˳���");
		return 1;
	}else
	{
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n�ļ�������ɡ�");
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  16kB  ROM ����Ŀ: %d", neshreader->romnum);
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n   8kB VROM ����Ŀ: %d", neshreader->vromnum);
		if((neshreader->romfeature & 0x01) == 0){
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  ˮƽ����");
		}else{
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  ��ֱ����");
		}

		if((neshreader->romfeature & 0x02) == 0){
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  �޼���SRAM");
		}else{
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  �м���SRAM");
		}

		if((neshreader->romfeature & 0x04) == 0)
		{
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  ��512�ֽڵ�trainer($7000-$71FF)");
		}else
		{
			offset=512;
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  ��512�ֽڵ�trainer(ROM��ʽ�ݲ�֧��)");
		}

		if((neshreader->romfeature & 0x08) == 0){
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  2��ĻVRAM����");
		}else{
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  4��ĻVRAM����(�ݲ�֧��)");
		}

		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  iNES Mapper Numbers: %d", (neshreader->rommappernum & 0xF0)|( neshreader->romfeature >> 4));
	}	  
	res=nes_mem_creat();//�����ڴ�
	if(res==0)//����ɹ���.��������Ϸ
	{  
		//��ʼ��nes ģ����		 
		init6502mem(0,						//exp_rom 
					0,						//sram �ɿ����;���, �ݲ�֧�� 
					((u8*)&rom_file[offset+0x10]),	//prg_rombank, �洢����С �ɿ����;��� 
					neshreader->romnum 	
					);//��ʼ��6502�洢������
		reset6502();//��λ
		PPU_Init(((u8*)&rom_file[offset+0x10] + (neshreader->romnum * 0x4000)), (neshreader->romfeature & 0x01));//PPU_��ʼ�� 	
		NES_JoyPadInit();  
		NesFrameCycle(ctx);//ģ����ѭ��ִ��
	}
	nes_mem_delete();//�ͷ��ڴ�
 	return res;	 		
}	 
//����nes���������RAM.
//����ֵ:0,�ɹ�;
//    ����,�������.
u8 nes_mem_creat(void)
{
	ram6502=(u8*)malloc(2048);	//����2K�ڴ�
	if(ram6502==NULL)return 1;			//����ʧ��
	NameTable=(u8*)malloc(2048);//����2K�ڴ�
	if(NameTable==NULL)return 2;
	Buffer_scanline=(u32*)malloc((8+256+8)*4);
	if(Buffer_scanline==NULL)return 3;
	return 0;	
}	 
//ɾ��nes����ʱ�����RAM		
void nes_mem_delete(void)
{
	free(ram6502);
	free(NameTable);
	free(Buffer_scanline);
} 		












