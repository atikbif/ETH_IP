/*
 * fsmc_flash.h
 *
 *  Created on: 13 џэт. 2019 у.
 *      Author: User
 */

#ifndef FSMC_FLASH_H_
#define FSMC_FLASH_H_

#define		FLASH_OK	1
#define		FLASH_BUSY	2
#define		FLASH_ERROR	3

#define		MAX_BAD_BLOCKS_CNT		50

void scan_bad_blocks(void);
unsigned char read_page(unsigned short bl_n, unsigned short page_n, unsigned char *ptr);
unsigned char write_page(unsigned short bl_n, unsigned short page_n, unsigned char *ptr);
unsigned char erase_block(unsigned short bl_n);
unsigned char check_block(unsigned short bl_n);
void add_bad_block(unsigned short bl_n);
void mark_bad_block(unsigned short bl_n);
void reset_flash();
unsigned char isFlashBusy(void);

#endif /* FSMC_FLASH_H_ */
