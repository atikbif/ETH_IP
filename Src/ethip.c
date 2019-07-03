/*
 * ethip.c
 *
 *  Created on: 3 апр. 2019 г.
 *      Author: Roman
 */

#include "ethip.h"
#include <string.h>

#define HEADER_SIZE	(2+2+4+4+8+4)

#define REG_SESSION		0x0065
#define UNREG_SESSION	0x0066
#define SEND_RR_DATA	0x006F

uint8_t answer_9c[74] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
	0x00, 0x00
};

uint8_t answer_90[246] = {
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x01, 0x00, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfd,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfd,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe,
  0x00, 0xfe, 0x00, 0xfe, 0x00, 0xfe, 0x00, 0x00,
  0x01, 0x01, 0x00, 0x00, 0x00, 0x10, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20
};

uint8_t answer_91[28] = {
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
  0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
  0xaa, 0xaa, 0xaa, 0xaa
};

uint8_t answer_92[14] = {
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa
};

uint8_t answer_93[14] = {
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa,
	0xaa, 0xaa, 0xaa, 0xaa, 0xaa, 0xaa
};

uint8_t answer_94[28] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

uint8_t answer_95[128] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0b,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t answer_96[16] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t answer_97[128] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t answer_98[128] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

uint8_t answer_99[4] = {
	0x00, 0x00, 0x00, 0x00
};

uint8_t answer_9a[34] = {
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00
};

uint8_t answer_9b[2] = {
	0x80, 0x00
};

uint8_t answer_time[4] = {0,0,0,0};

uint8_t ob_99_upd = 0;
uint8_t ob_9a_upd = 0;
uint8_t ob_time_upd = 0;
uint8_t eth_ip_state = 2;
uint16_t eth_ip_tmr = 0;

static uint32_t get_new_session_handle(void) {
	static uint32_t session_handle = 0x20021300 - 1;
	session_handle++;
	//eth_ip_state = 0;
	return session_handle;
}

static uint8_t check_rr_request_header(uint8_t *inp) {
	uint32_t interface_handle;
	//uint16_t timeout;
	interface_handle = inp[3];interface_handle <<= 8;
	interface_handle |= inp[2];interface_handle <<= 8;
	interface_handle |= inp[1];interface_handle <<= 8;
	interface_handle |= inp[0];
	//timeout = inp[5];timeout <<= 8;
	//timeout |= inp[4];
	if(interface_handle==0x00) { // CIP interface
		uint16_t item_count = inp[7];item_count <<= 8;
		item_count |= inp[6];
		if(item_count==2) {
			// address
			uint16_t id = inp[9];id <<= 8;
			id |= inp[8];
			if(id==0x00) {
				uint16_t length = inp[11];length <<= 8;
				length |= inp[10];
				if(length==0) {
					// data item
					id = inp[13];id <<= 8;
					id |= inp[12];
					if(id==0x00B2) { // unconnected message
						length = inp[15];length <<= 8;
						length |= inp[14];
						return length;
					}
				}
			}
		}
	}
	return 0;
}

static uint16_t create_rr_data_reply_header(uint8_t *out, uint16_t timeout, uint16_t addr, uint16_t length) {
	// interface handle (CIP)
	out[0] = 0x00; out[1] = 0x00; out[2] = 0x00; out[3] = 0x00;
	// timeout
	out[4] = timeout & 0xFF;
	out[5] = timeout >> 8;
	// item count
	out[6] = 0x02;
	out[7] = 0x00;
	// address item
	out[8] = addr & 0xFF; out[9] = addr >> 8;	// addr id
	out[10] = 0x00;	out[11] = 0x00;				// addr length
	// data item
	out[12] = 0xB2;	out[13] = 0x00;	// unconnected message
	out[14] = length & 0xFF; out[15] = length >> 8;
	return 16;
}

static uint16_t identity_get_attribute_single(uint8_t* out, uint8_t attr_num) {
	if(attr_num==1) {
		uint16_t offset = create_rr_data_reply_header(out,1,0,6);
		// data CIP
		// response
		out[offset++] = 0x8E;
		// reserved shal be zero
		out[offset++] = 0x00;
		// status (success)
		out[offset++] = 0x00;
		// size of additional status
		out[offset++] = 0x00;
		// vendor ID
		out[offset++] = 0x5A;
		out[offset++] = 0x00;
		return offset;
	}
	return 0;
}

static uint16_t class0x90_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_90));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_90);i++) out[offset++] = answer_90[i];
	return offset;
}

static uint16_t class0x91_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_91));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_91);i++) out[offset++] = answer_91[i];
	return offset;
}

static uint16_t class0x92_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_92));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_92);i++) out[offset++] = answer_92[i];
	return offset;
}

static uint16_t class0x93_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_93));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_93);i++) out[offset++] = answer_93[i];
	return offset;
}

static uint16_t class0x94_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_94));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_94);i++) out[offset++] = answer_94[i];
	return offset;
}

static uint16_t class0x95_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_95));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_95);i++) out[offset++] = answer_95[i];
	return offset;
}

static uint16_t class0x96_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_96));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_96);i++) out[offset++] = answer_96[i];
	return offset;
}

static uint16_t class0x97_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_97));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_97);i++) out[offset++] = answer_97[i];
	return offset;
}

static uint16_t class0x98_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_98));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_98);i++) out[offset++] = answer_98[i];
	return offset;
}

static uint16_t class0x99_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_99));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_99);i++) out[offset++] = answer_99[i];
	return offset;
}

static uint16_t class0x9a_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_9a));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_9a);i++) out[offset++] = answer_9a[i];
	return offset;
}

static uint16_t class0x9b_get_attribute_single(uint8_t* out) {
	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_9b));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_9b);i++) out[offset++] = answer_9b[i];
	return offset;
}

static uint16_t class0x9c_get_attribute_single(uint8_t* out) {

	static uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out,1,0,4+sizeof(answer_9c));
	// data CIP
	// response
	out[offset++] = 0x8E;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_9c);i++) out[offset++] = answer_9c[i];
	return offset;
}

static uint16_t get_attribute_single(uint8_t* out,uint8_t class_num, uint8_t impl_num, uint8_t attr_num) {
	if(class_num==0x01 && impl_num==0x01) return identity_get_attribute_single(out,attr_num);
	if(class_num==0x90 && impl_num==0x01 && attr_num==0x01) return class0x90_get_attribute_single(out);
	if(class_num==0x91 && impl_num==0x01 && attr_num==0x01) return class0x91_get_attribute_single(out);
	if(class_num==0x92 && impl_num==0x01 && attr_num==0x01) return class0x92_get_attribute_single(out);
	if(class_num==0x93 && impl_num==0x01 && attr_num==0x01) return class0x93_get_attribute_single(out);
	if(class_num==0x94 && impl_num==0x01 && attr_num==0x01) return class0x94_get_attribute_single(out);
	if(class_num==0x95 && impl_num==0x01 && attr_num==0x01) return class0x95_get_attribute_single(out);
	if(class_num==0x96 && impl_num==0x01 && attr_num==0x01) return class0x96_get_attribute_single(out);
	if(class_num==0x97 && impl_num==0x01 && attr_num==0x01) return class0x97_get_attribute_single(out);
	if(class_num==0x98 && impl_num==0x01 && attr_num==0x01) return class0x98_get_attribute_single(out);
	if(class_num==0x99 && impl_num==0x01 && attr_num==0x01) return class0x99_get_attribute_single(out);
	if(class_num==0x9A && impl_num==0x01 && attr_num==0x01) return class0x9a_get_attribute_single(out);
	if(class_num==0x9B && impl_num==0x01 && attr_num==0x01) return class0x9b_get_attribute_single(out);
	if(class_num==0x9C && impl_num==0x01 && attr_num==0x01) {
		eth_ip_state = 1;
		eth_ip_tmr = 0;
		return class0x9c_get_attribute_single(out);
	}
	return 0;
}

static uint16_t time_set_attribute_single(uint8_t* out,uint8_t *data) {
	uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out, 1, 0, 4);
	// data CIP
	// response
	out[offset++] = 0x90;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	ob_time_upd = 1;
	for(i=0;i<sizeof(answer_time);i++) answer_time[i] = data[i];
	return offset;
}

static uint16_t set_attribute_single_99(uint8_t* out,uint8_t *data) {
	uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out, 1, 0, 4);
	// data CIP
	// response
	out[offset++] = 0x90;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_99);i++) answer_99[i] = data[i];
	ob_99_upd = 1;
	return offset;
}

static uint16_t set_attribute_single_9a(uint8_t* out,uint8_t *data) {
	uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out, 1, 0, 4);
	// data CIP
	// response
	out[offset++] = 0x90;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_9a);i++) answer_9a[i] = data[i];
	ob_9a_upd = 1;
	return offset;
}

static uint16_t set_attribute_single_9c(uint8_t* out,uint8_t *data) {
	uint16_t i = 0;
	uint16_t offset = create_rr_data_reply_header(out, 1, 0, 4);
	// data CIP
	// response
	out[offset++] = 0x90;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	for(i=0;i<sizeof(answer_9c);i++) answer_9c[i] = data[i];
	return offset;
}

static uint16_t ok_set_attribute_single(uint8_t* out,uint8_t *data) {
	uint16_t offset = create_rr_data_reply_header(out, 1, 0, 4);
	// data CIP
	// response
	out[offset++] = 0x90;
	// reserved shal be zero
	out[offset++] = 0x00;
	// status (success)
	out[offset++] = 0x00;
	// size of additional status
	out[offset++] = 0x00;
	return offset;
}

static uint16_t set_attribute_single(uint8_t* out,uint8_t class_num, uint8_t impl_num, uint8_t attr_num, uint8_t *data) {
	if(class_num==0x9D && impl_num==0x01 && attr_num==0x01) return time_set_attribute_single(out, data);
	if(class_num==0x9C && impl_num==0x01 && attr_num==0x01) return set_attribute_single_9c(out, data);
	if(class_num==0x99 && impl_num==0x01 && attr_num==0x01) return set_attribute_single_99(out, data);
	if(class_num==0x9A && impl_num==0x01 && attr_num==0x01) return set_attribute_single_9a(out, data);
	if(class_num>=0x90 && class_num<=0x9C && impl_num==0x01 && attr_num==0x01) return ok_set_attribute_single(out,data);
	return 0;
}

static void get_error_answer(encaps_packet *inp, packet *out, uint32_t err_code) {
	uint8_t i = 0;
	out->data[0] = inp->cmd & 0xFF;
	out->data[1] = inp->cmd >> 8;
	// length
	out->data[2] = 0x00;
	out->data[3] = 0x00;
	// handle
	out->data[4] = inp->handle & 0xFF;
	out->data[5] = (inp->handle >> 8) & 0xFF;
	out->data[6] = (inp->handle >> 8) & 0xFF;
	out->data[7] = (inp->handle >> 8) & 0xFF;
	// status (invalid length)
	out->data[8] = err_code & 0xFF;
	out->data[9] = (err_code >>8) & 0xFF;
	out->data[10] = (err_code >>16) & 0xFF;
	out->data[11] = (err_code >>24) & 0xFF;
	// sender context
	for(i=0;i<8;i++) out->data[12+i] = inp->sender_context[i];
	// options
	out->data[20] = 0x00;
	out->data[21] = 0x00;
	out->data[22] = 0x00;
	out->data[23] = 0x00;
	out->length = 24;
}

static void create_header(uint8_t *out, encaps_packet *inp) {
	uint8_t i = 0;
	out[0] = inp->cmd & 0xFF;
	out[1] = inp->cmd >> 8;
	out[2] = inp->length & 0xFF;
	out[3] = inp->length >> 8;
	out[4] = inp->handle & 0xFF;
	out[5] = (inp->handle >> 8) & 0xFF;
	out[6] = (inp->handle >> 16) & 0xFF;
	out[7] = (inp->handle >> 24) & 0xFF;
	out[8] = inp->status & 0xFF;
	out[9] = (inp->status >> 8) & 0xFF;
	out[10] = (inp->status >> 16) & 0xFF;
	out[11] = (inp->status >> 24) & 0xFF;
	for(i=0;i<8;i++) out[12+i] = inp->sender_context[i];
	// options
	out[20] = 0x00;
	out[21] = 0x00;
	out[22] = 0x00;
	out[23] = 0x00;
}

static void get_reg_session_answer(encaps_packet *inp, packet *out) {
	if(inp->length!=4) {get_error_answer(inp,out,0x65);return;} // invalid length
	uint16_t version = inp->data[1];version <<= 8;
	version |= inp->data[0];
	if(version==0x01) {
		inp->handle = get_new_session_handle();
		inp->length = 4;
		inp->status = 0; // success
		create_header(&out->data[0],inp);
		// command specific data
		// supported protocol version
		out->data[24] = 0x01;
		out->data[25] = 0x00;
		// options flags
		out->data[26] = 0x00;
		out->data[27] = 0x00;
		out->length = 28;
	}else {
		inp->handle = 0;
		inp->length = 4;
		inp->status = 0x69; // unsupported protocol version
		create_header(&out->data[0],inp);
		// command specific data
		// supported protocol version
		out->data[24] = 0x01;
		out->data[25] = 0x00;
		// options flags
		out->data[26] = 0x00;
		out->data[27] = 0x00;
		out->length = 28;
	}
}

static void get_unreg_session_answer(encaps_packet *inp, packet *out) {
	out->close_tcp = 1;
}

static void get_rr_answer(encaps_packet *inp, packet *out) {
	uint16_t length = check_rr_request_header(&inp->data[0]);
	uint8_t service = inp->data[16];
	if(length) {
		uint8_t path = inp->data[17];
		uint8_t class_format = inp->data[18];
		uint8_t class_num = inp->data[19];
		uint8_t impl_format = inp->data[20];
		uint8_t impl_num = inp->data[21];
		uint8_t attr_format = inp->data[22];
		uint8_t attr_num = inp->data[23];
		if(path==3 && class_format==0x20 && impl_format==0x24 && attr_format==0x30) {
			if(service==0x0E) {
				uint16_t answer_body_length = get_attribute_single(&out->data[HEADER_SIZE],class_num,impl_num,attr_num);
				if(answer_body_length) {
					// add header
					inp->length = answer_body_length;
					inp->status = 0x00; // unsupported protocol version
					create_header(&out->data[0],inp);
					out->length = HEADER_SIZE + answer_body_length;
				}
			}else if(service==0x10) { // set attribute single
				uint16_t answer_body_length = set_attribute_single(&out->data[HEADER_SIZE],class_num,impl_num,attr_num,&inp->data[24]);
				if(answer_body_length) {
					// add header
					inp->length = answer_body_length;
					inp->status = 0x00; // unsupported protocol version
					create_header(&out->data[0],inp);
					out->length = HEADER_SIZE + answer_body_length;
				}
			}
		}
	}
}

static void clear_encaps_pckt(encaps_packet *epckt) {
	uint8_t i = 0;
	epckt->cmd = 0x0000;
	epckt->length = 0x0000;
	epckt->handle = 0x00000000;
	epckt->status = 0x00000000;
	for(i=0;i<8;i++) epckt->sender_context[i] = 0x00;
	epckt->options = 0x00000000;
	epckt->data = 0;
}

void get_encaps_packet(packet *inp, encaps_packet *out, uint8_t *res) {
	uint8_t i = 0;
	clear_encaps_pckt(out);
	out->length = inp->data[3];out->length <<= 8;
	out->length |= inp->data[2];

	if(inp->length!=out->length + HEADER_SIZE) {*res = 0;return;}
	out->cmd = inp->data[1];out->cmd <<= 8;
	out->cmd |= inp->data[0];
	out->handle = inp->data[7];out->handle <<= 8;
	out->handle |= inp->data[6];out->handle <<= 8;
	out->handle |= inp->data[5];out->handle <<= 8;
	out->handle |= inp->data[4];
	out->status = inp->data[11];out->status <<= 8;
	out->status |= inp->data[10];out->status <<= 8;
	out->status |= inp->data[9];out->status <<= 8;
	out->status |= inp->data[8];
	for(i=0;i<8;i++) out->sender_context[i] = inp->data[12+i];
	out->options = inp->data[23];out->options <<= 8;
	out->options |= inp->data[22];out->options <<= 8;
	out->options |= inp->data[21];out->options <<= 8;
	out->options |= inp->data[20];
	out->data = &(inp->data[24]);
	*res = 1;
}

void get_answer(encaps_packet *inp, packet *out) {
	out->close_tcp = 0;
	out->length = 0;
	if(inp->status) return;
	if(inp->options) return;
	if(inp->cmd == REG_SESSION) {get_reg_session_answer(inp,out);return;}
	if(inp->cmd == UNREG_SESSION) { get_unreg_session_answer(inp,out);return;}
	if(inp->cmd == SEND_RR_DATA) {get_rr_answer(inp,out);return;}
	get_error_answer(inp,out,0x01); // unsupported command
}
