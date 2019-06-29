/*
 * fsmc_flash.c
 *
 *  Created on: 13 џэт. 2019 у.
 *      Author: User
 */

#include "fsmc_flash.h"
#include "stm32f4xx_hal.h"


static volatile unsigned char _Sys_Flash_Buzy=0;

extern NAND_HandleTypeDef hnand1;

unsigned short bad_blocks[MAX_BAD_BLOCKS_CNT];
unsigned char bad_cnt = 0;

static volatile unsigned char spare[64];

unsigned char isFlashBusy(void) {
	return _Sys_Flash_Buzy;
}

static unsigned char test_block(unsigned short bl_n) {
	NAND_AddressTypeDef NAND_Address;
	if(check_block(bl_n)==0) return 0;

	NAND_Address.Plane = 0;
	NAND_Address.Block = bl_n;
	NAND_Address.Page = 0;
	spare[0] = 0;
	HAL_NAND_Read_SpareArea_8b(&hnand1, &NAND_Address,(uint8_t*)spare,1);

	if(spare[0]==0xFF) return 1;
	add_bad_block(bl_n);
	return 0;
}

void scan_bad_blocks(void) {
	unsigned short i = 0;
	bad_cnt = 0;
	for(i=0;i<MAX_BAD_BLOCKS_CNT;i++)  {
		bad_blocks[i]=0xFFFF;
	}
	for(i=0;i<1024;i++) {
		test_block(i);
		if(bad_cnt>=MAX_BAD_BLOCKS_CNT) break;
	}
}

unsigned char read_page(unsigned short bl_n, unsigned short page_n, unsigned char *ptr) {
	NAND_AddressTypeDef NAND_Address;
	HAL_StatusTypeDef res;

	NAND_Address.Plane = 0;
	NAND_Address.Block = bl_n;
	NAND_Address.Page = page_n;

	_Sys_Flash_Buzy = 1;
	res = HAL_NAND_Read_Page(&hnand1, &NAND_Address, ptr, 1);
	_Sys_Flash_Buzy = 0;

	if(res==HAL_OK) return FLASH_OK;
	if(res==HAL_BUSY) return FLASH_BUSY;
	return FLASH_ERROR;
}

unsigned char write_page(unsigned short bl_n, unsigned short page_n, unsigned char *ptr) {
	NAND_AddressTypeDef NAND_Address;
	HAL_StatusTypeDef res;

	NAND_Address.Plane = 0;
	NAND_Address.Block = bl_n;
	NAND_Address.Page = page_n;

	_Sys_Flash_Buzy = 1;
	res = HAL_NAND_Write_Page(&hnand1, &NAND_Address, ptr, 1);
	_Sys_Flash_Buzy = 0;

	if(res==HAL_TIMEOUT) {
		hnand1.State = HAL_NAND_STATE_RESET;
		return FLASH_ERROR;
	}
	if(res==HAL_OK) return FLASH_OK;
	if(res==HAL_BUSY) return FLASH_BUSY;
	return FLASH_ERROR;
}

unsigned char erase_block(unsigned short bl_n) {
	NAND_AddressTypeDef NAND_Address;
	HAL_StatusTypeDef res;

	NAND_Address.Plane = 0;
	NAND_Address.Block = bl_n;
	NAND_Address.Page = 0;

	_Sys_Flash_Buzy = 1;
	res = HAL_NAND_Erase_Block(&hnand1, &NAND_Address);
	_Sys_Flash_Buzy = 0;

	if(res==HAL_TIMEOUT) {
		hnand1.State = HAL_NAND_STATE_RESET;
		return FLASH_ERROR;
	}
	if(res==HAL_OK) return FLASH_OK;
	if(res==HAL_BUSY) return FLASH_BUSY;
	return FLASH_ERROR;
}

unsigned char check_block(unsigned short bl_n) {
	unsigned char i=0;
	for(i=0;i<MAX_BAD_BLOCKS_CNT;i++) {
		if(bad_blocks[i]==bl_n) return 0;
	}
	return 1;
}

void add_bad_block(unsigned short bl_n) {
	if(bad_cnt<MAX_BAD_BLOCKS_CNT) {bad_blocks[bad_cnt++] = bl_n;}
}

void mark_bad_block(unsigned short bl_n) {
	NAND_AddressTypeDef NAND_Address;
	unsigned char i = 0;

	NAND_Address.Plane = 0;
	NAND_Address.Block = bl_n;
	NAND_Address.Page = 0;
	for(i=0;i<64;i++) spare[i] = 0;

	_Sys_Flash_Buzy = 1;
	HAL_NAND_Write_SpareArea_8b(&hnand1, &NAND_Address,(uint8_t*)spare,1);
	_Sys_Flash_Buzy = 0;
}

void reset_flash() {
	HAL_NAND_Reset(&hnand1);
}
