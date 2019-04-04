/*
 * ethip.h
 *
 *  Created on: 3 апр. 2019 г.
 *      Author: Roman
 */

#ifndef ETHIP_H_
#define ETHIP_H_

#include <stdint.h>

typedef struct {
	uint8_t 	*data;
	uint16_t	length;
	uint8_t		close_tcp;
}packet;

typedef struct {
	uint16_t cmd;
	uint16_t length;
	uint32_t handle;
	uint32_t status;
	uint8_t  sender_context[8];
	uint32_t options;
	uint8_t *data;
}encaps_packet;

void get_encaps_packet(packet *inp, encaps_packet *out, uint8_t *res);
void get_answer(encaps_packet *inp, packet *out);


#endif /* ETHIP_H_ */
