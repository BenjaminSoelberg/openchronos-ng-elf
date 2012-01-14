// MYsimpliciti


#if defined (CONFIG_USEPPT) || defined (CONFIG_EGGTIMER) || (CONFIG_ACCEL)
#define SIMPLICITI_TX_ONLY_REQ
#endif


/*
smplStatus_t SMPL_Link(0);
}

smplStatus_t SMPL_Init(uint8_t (*callback)(linkID_t));

smplStatus_t SMPL_SendOpt(lid, *msg, len,opt);

smplStatus_t SMPL_Receive(lid, *msg, *len);

smplStatus_t SMPL_Ioctl(object, action, *val);
*/

// Include section

// system
#include "project.h"


// forward declarations/prototype section
// used in main.c and rf1a.c
void __delay_cycles(uint16_t cycles);

// used in adc12.c
uint8_t __even_in_range(uint16_t vector, uint8_t unknown);

// used in radio.c
void MRFI_RadioIsr(void); 

//used in rf1a.c
uint16_t __get_interrupt_state(void);
void __disable_interrupt(void);
void __set_interrupt_state(uint16_t int_state);

// used in rfsimpliciti.c
void simpliciti_link(void);
void simpliciti_main_sync(void);
void simpliciti_main_tx_only(void);


// functions
void __delay_cycles(uint16_t cycles) {}
uint8_t __even_in_range(uint16_t vector, uint8_t unknown) {}
void MRFI_RadioIsr(void){}
uint16_t __get_interrupt_state(void) {}
void __disable_interrupt(void) {}
void __set_interrupt_state(uint16_t int_state) {}
void simpliciti_link() {}
void simpliciti_main_sync() {}
void simpliciti_main_tx_only() {}

// forward declarations/prototype section
/*
void reset_rf(void);
void sx_rf(uint8_t line);
void sx_ppt(uint8_t line);
void sx_sync(uint8_t line);
void display_rf(uint8_t line, uint8_t update);
void display_ppt(uint8_t line, uint8_t update);
void display_sync(uint8_t line, uint8_t update);
void send_smpl_data(uint16_t data);
uint8_t is_rf(void);
*/
/*
void reset_rf() {}
void sx_rf(uint8_t line) {}
void sx_ppt(uint8_t line) {}
void sx_sync(uint8_t line){}
void display_rf(uint8_t line, uint8_t update) {}
void display_ppt(uint8_t line, uint8_t update) {}
void display_sync(uint8_t line, uint8_t update) {}
void send_smpl_data(uint16_t data){}
uint8_t is_rf(void) {}
*/
