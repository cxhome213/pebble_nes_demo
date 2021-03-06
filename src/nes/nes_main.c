#include "nes/nes_main.h"
#include "nes/nes_rom.h"
#include "stdlib.h"
//////////////////////////////////////////////////////////////////////////////////	 
//参考自网友:xiaowei061的NES模拟器，再此对xiaowei061表示感谢！
//ALIENTEK战舰STM32开发板
//NES模拟器 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/10/3
//版本：V1.0		       								  
////////////////////////////////////////////////////////////////////////////////// 	   

extern Layer *s_canvas_layer;

u8 nesruning = 0;		//初始化为真
u8 frame_cnt = 0;		 	//统计帧数

//NES 帧周期循环 
void NesFrameCycle(GContext *ctx)
{
 	int	clocks;	//CPU执行时间	 
	//启动模拟器循环，检查VROM设置，若卡带为0，初始化VROM
	//if ( NesHeader.byVRomSize == 0)
	//设置VROM存储区位置。
	//frame_cnt=0;
	nesruning=1;
	//while(nesruning)
	{
 		//scanline: 0~19 VBANK 段，若PPU使能NMI，将产生NMI 中断
		frame_cnt++;//帧计数器		    
		SpriteHitFlag=FALSE;		
		for(PPU_scanline=0; PPU_scanline<20; PPU_scanline++)
		{ 
			exec6502(CLOCKS_PER_SCANLINE);
 		}
		//scanline: 20, PPU加载设置，执行一次空的扫描时间 
		exec6502(CLOCKS_PER_SCANLINE);
		//NesHBCycle();  //水平周期
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

			if(PPU_Reg.NES_R1 & (R1_BG_VISIBLE | R1_SPR_VISIBLE))//若为假，关闭显示
			{
				if(SpriteHitFlag == FALSE)
				NES_GetSpr0HitFlag(PPU_scanline - SCAN_LINE_DISPALY_START_NUM);//查找Sprite #0 碰撞标志
			}

			//if((frame_cnt%3)==0)//每2帧显示一次
			{
				NES_RenderLine(PPU_scanline - SCAN_LINE_DISPALY_START_NUM, ctx);//水平同步与显示一行
			}	
			
		}

		//layer_mark_dirty(s_canvas_layer);

		//scanline: 262 完成一帧 
		exec6502(CLOCKS_PER_SCANLINE);
		PPU_Reg.NES_R2 |= R2_VBlank_Flag;//设置VBANK 标志
		//若使能PPU VBANK中断，则设置VBANK 
		if(PPU_Reg.NES_R0 & R0_VB_NMI_EN)
		{
			NMI_Flag = SET1;//完成一帧扫描，产生NMI中断
		}  
	   	//设置帧IRQ标志，同步计数器，APU等 		   
		//A mapper function in V-Sync 存储器切换垂直VBANK同步 
		//MapperVSync();  
		//读取控制器JoyPad状态,更新JoyPad控制器值
		//NES_JoyPadUpdateValue();	 //systick 中断读取按键值   
	}

} 
//返回值:0,执行OK;
//	  其他,错误代码
u8 nes_main(GContext *ctx)
{	
	u16 offset=0;
	u8 res;
	NesHeader *neshreader = (NesHeader *) rom_file;
 	//由于NES文件头类型以0x1A结束，（0x1a是Ctrl+Z,这是模拟文件结束的符号）
	//所以使用strcncmp比较前3个字节。   
	if(strncmp(neshreader->filetype, "NES", 3) != 0)
	{
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n文件未加载或文件类型错误, NES模拟器退出。");
		return 1;
	}else
	{
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n文件加载完成。");
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  16kB  ROM 的数目: %d", neshreader->romnum);
		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n   8kB VROM 的数目: %d", neshreader->vromnum);
		if((neshreader->romfeature & 0x01) == 0){
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  水平镜像");
		}else{
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  垂直镜像");
		}

		if((neshreader->romfeature & 0x02) == 0){
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  无记忆SRAM");
		}else{
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  有记忆SRAM");
		}

		if((neshreader->romfeature & 0x04) == 0)
		{
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  无512字节的trainer($7000-$71FF)");
		}else
		{
			offset=512;
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  有512字节的trainer(ROM格式暂不支持)");
		}

		if((neshreader->romfeature & 0x08) == 0){
			APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  2屏幕VRAM布局");
		}else{
	 		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  4屏幕VRAM布局(暂不支持)");
		}

		APP_LOG(APP_LOG_LEVEL_INFO, "\r\n  iNES Mapper Numbers: %d", (neshreader->rommappernum & 0xF0)|( neshreader->romfeature >> 4));
	}	  
	res=nes_mem_creat();//申请内存
	if(res==0)//申请成功了.则运行游戏
	{  
		//初始化nes 模拟器		 
		init6502mem(0,						//exp_rom 
					0,						//sram 由卡类型决定, 暂不支持 
					((u8*)&rom_file[offset+0x10]),	//prg_rombank, 存储器大小 由卡类型决定 
					neshreader->romnum 	
					);//初始化6502存储器镜像
		reset6502();//复位
		PPU_Init(((u8*)&rom_file[offset+0x10] + (neshreader->romnum * 0x4000)), (neshreader->romfeature & 0x01));//PPU_初始化 	
		NES_JoyPadInit();  
		NesFrameCycle(ctx);//模拟器循环执行
	}
	nes_mem_delete();//释放内存
 	return res;	 		
}	 
//开辟nes运行所需的RAM.
//返回值:0,成功;
//    其他,错误代码.
u8 nes_mem_creat(void)
{
	ram6502=(u8*)malloc(2048);	//申请2K内存
	if(ram6502==NULL)return 1;			//申请失败
	NameTable=(u8*)malloc(2048);//申请2K内存
	if(NameTable==NULL)return 2;
	Buffer_scanline=(u32*)malloc((8+256+8)*4);
	if(Buffer_scanline==NULL)return 3;
	return 0;	
}	 
//删除nes运行时申请的RAM		
void nes_mem_delete(void)
{
	free(ram6502);
	free(NameTable);
	free(Buffer_scanline);
} 		












