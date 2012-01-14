/****
 * written by Lukas Middendorf
 *
 * use as desired but do not remove this notice
 */

#include "project.h"

#ifndef INFOMEM_H_
#define INFOMEM_H_

/*
 * This driver allows applications to store data in the information memory flash
 * without interfering with each other (except for the total available memory) or
 * having to care about the characteristics of flash memory.
 *
 * infomem_ready() has to be called before any other function does work (except
 * infomem_init, but use infomem_ready to check if infomem_init is really needed).
 *
 * Single applications should only use the infomem_app_*() functions ary infomem_space(),
 * the rest of the functions should be used only in the global part of the firmware
 * (or in a dedicated application which's function it is to do memory maintenance tasks).
 *
 * Use infomem_app_replace() with count>0 to initialize memory for application. This has
 * to be redone if data of size zero was present and the application header was therefore
 * also removed (replace with zero count, clear, delete with zero offset or modify with
 * zero count and offset).
 *
 * All pointers and addresses have to be word addresses (even numbers) and all counts
 * are given in units of words (two bytes).
 */


//check if infomem is initialized and in sane state, return amount of data present
extern int16_t infomem_ready();
//write infomem data structure
extern int16_t infomem_init(uint16_t start, uint16_t end);
//return amount of free space
extern int16_t infomem_space();
//change start and end address of data storage (can change size)
extern int16_t infomem_relocate(uint16_t start, uint16_t end);
//delete complete data storage (only managed space)
extern int16_t infomem_delete_all(void);

//return how much data for the application is available
extern int16_t infomem_app_amount(uint8_t identifier);
//read count bytes of data with offset for given application into prepared memory
extern int16_t infomem_app_read(uint8_t identifier, uint16_t *data, uint8_t count, uint8_t offset);
//replace all memory content for application by new data
extern int16_t infomem_app_replace(uint8_t identifier, uint16_t *data, uint8_t count);
//delete all memory content for application
extern int16_t infomem_app_clear(uint8_t identifier);
//delete all memory content beginning with offset
extern int16_t infomem_app_delete(uint8_t identifier, uint8_t offset);
//modify given bytes of data
extern int16_t infomem_app_modify(uint8_t identifier, uint16_t *data, uint8_t count, uint8_t offset);



struct infomem {
	uint16_t		*startaddr; //starting address (position of header)
	uint8_t			size;  //size of payload in words
	uint8_t			maxsize;  //maximum size of payload in words
	volatile uint8_t	not_lock;  //memory is not locked for write
	uint8_t			sane;  //sanity check passed
};
// extern struct infomem sInfomem;


#define INFOMEM_IDENTIFIER 0x5a74
#define INFOMEM_TERMINATOR 0xdaf4
#define INFOMEM_SANE 0xda

#define INFOMEM_START 0x1800
#define INFOMEM_D 0x1800
#define INFOMEM_C 0x1880
#define INFOMEM_B 0x1900
#define INFOMEM_A 0x1980
#define INFOMEM_SEGMENT_SIZE 128
#define INFOMEM_SEGMENT_WORDS INFOMEM_SEGMENT_SIZE/2
#define INFOMEM_ERASED_WORD 0xFFFF


#endif /*INFOMEM_H_*/