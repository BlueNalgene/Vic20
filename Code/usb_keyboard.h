#ifndef usb_serial_h__
#define usb_serial_h__

#include <stdint.h>

void usb_init(void);			// initialize everything
uint8_t usb_configured(void);		// is the USB port configured

int8_t usb_keyboard_press(uint8_t key, uint8_t modifier);
int8_t usb_keyboard_send(void);
extern uint8_t keyboard_modifier_keys;
extern uint8_t keyboard_keys[6];
extern volatile uint8_t keyboard_leds;

// This file does not include the HID debug functions, so these empty
// macros replace them with nothing, so users can compile code that
// has calls to these functions.
#define usb_debug_putchar(c)
#define usb_debug_flush_output()

enum keycodes {
  KEY__=0,
  KEY_errorRollOver,
  KEY_POSTfail,
  KEY_errorUndefined,
  KEY_A,        // 4
  KEY_B,
  KEY_C,
  KEY_D,
  KEY_E,
  KEY_F,
  KEY_G, 
  KEY_H,
  KEY_I,
  KEY_J,
  KEY_K,
  KEY_L,
  KEY_M,        // 0x10
  KEY_N,
  KEY_O,
  KEY_P,
  KEY_Q, 
  KEY_R,
  KEY_S,
  KEY_T,
  KEY_U,
  KEY_V,
  KEY_W,
  KEY_X,
  KEY_Y,
  KEY_Z,
  KEY_1,
  KEY_2,
  KEY_3,        // 0x20
  KEY_4,
  KEY_5,
  KEY_6,
  KEY_7,
  KEY_8,
  KEY_9,
  KEY_0,        // 0x27
  KEY_enter,
  KEY_esc,
  KEY_bckspc,   // backspace
  KEY_tab,
  KEY_spc,      // space
  KEY_minus,    // - (and _)
  KEY_equal,    // = (and +)
  KEY_lbr,      // [
  KEY_rbr,      // ]  -- 0x30
  KEY_bckslsh,  // \ (and |)
  KEY_hash,     // Non-US # and ~
  KEY_smcol,    // ; (and :)
  KEY_ping,     // ' and "
  KEY_grave,    // Grave accent and tilde
  KEY_comma,    // , (and <)
  KEY_dot,      // . (and >)
  KEY_slash,    // / (and ?)
  KEY_cpslck,   // capslock
  KEY_F1,
  KEY_F2,
  KEY_F3,
  KEY_F4,
  KEY_F5,
  KEY_F6, 
  KEY_F7,       // 0x40
  KEY_F8,
  KEY_F9,
  KEY_F10,
  KEY_F11,
  KEY_F12,
  KEY_PrtScr,
  KEY_scrlck,
  KEY_break,
  KEY_ins,
  KEY_home,
  KEY_pgup,
  KEY_del,
  KEY_end,
  KEY_pgdn,
  KEY_rarr, 
  KEY_larr,     // 0x50
  KEY_darr,
  KEY_uarr,
  KEY_numlock,
  KEY_KPslash,
  KEY_KPast,
  KEY_KPminus,
  KEY_KPplus,
  KEY_KPenter,
  KEY_KP1,
  KEY_KP2,
  KEY_KP3,
  KEY_KP4,
  KEY_KP5,
  KEY_KP6,
  KEY_KP7,
  KEY_KP8,      // 0x60
  KEY_KP9,
  KEY_KP0,
  KEY_KPcomma,
  KEY_Euro2,

  /* These are NOT standard USB HID - handled specially in decoding,
     so they will be mapped to the modifier byte in the USB report */
  KEY_Modifiers,
  MOD_LCTRL,    // 0x01
  MOD_LSHIFT,   // 0x02
  MOD_LALT,     // 0x04
  MOD_LGUI,     // 0x08
  MOD_RCTRL,    // 0x10
  MOD_RSHIFT,   // 0x20
  MOD_RALT,     // 0x40
  MOD_RGUI,     // 0x80
  
  /* Other keys that need special handling -
     These are looked up in the table spec_keys because they do not
     generate the same scan-code in the shifted and unshifted state,
     and some may need to alter the shift-state to generate the
     correct character code on the PC */
  KEY_Special,
  SPC_2,
  SPC_6,
  SPC_7,
  SPC_8,
  SPC_9,
  SPC_0,
  SPC_plus,
  SPC_minus,
  SPC_pound,
  SPC_home,
  SPC_del,
  SPC_ast,
  SPC_equal,
  SPC_crsrud,
  SPC_crsrlr,
  SPC_F1,
  SPC_F3,
  SPC_F5,
  SPC_F7,
  SPC_hat,
  SPC_colon,
  SPC_smcol,
  SPC_at
};
/*
#define MOD_LCTRL	0x01
#define MOD_LSHIFT	0x02
#define MOD_LALT	0x04
#define MOD_LGUI	0x08
#define MOD_RCTRL	0x10
#define MOD_RSHIFT	0x20
#define MOD_RALT	0x40
#define MOD_RGUI	0x80

#define KEY_A		4
#define KEY_B		5
#define KEY_C		6
#define KEY_D		7
#define KEY_E		8
#define KEY_F		9
#define KEY_G		10
#define KEY_H		11
#define KEY_I		12
#define KEY_J		13
#define KEY_K		14
#define KEY_L		15
#define KEY_M		16
#define KEY_N		17
#define KEY_O		18
#define KEY_P		19
#define KEY_Q		20
#define KEY_R		21
#define KEY_S		22
#define KEY_T		23
#define KEY_U		24
#define KEY_V		25
#define KEY_W		26
#define KEY_X		27
#define KEY_Y		28
#define KEY_Z		29
#define KEY_1		30
#define KEY_2		31
#define KEY_3		32
#define KEY_4		33
#define KEY_5		34
#define KEY_6		35
#define KEY_7		36
#define KEY_8		37
#define KEY_9		38
#define KEY_0		39
#define KEY_enter	40	
#define KEY_esc		41
#define KEY_bckspc	42	
#define KEY_tab		43
#define KEY_spc		44	
#define KEY_minus	45
#define KEY_equal	46
#define KEY_lbr		47
#define KEY_rbr		48
#define KEY_bckslsh	49
#define KEY_hash	50
#define KEY_smcol	51
#define KEY_ping	52
#define KEY_grave	53
#define KEY_comma	54
#define KEY_dot		55
#define KEY_slash	56
#define KEY_cpslck	57
#define KEY_F1		58
#define KEY_F2		59
#define KEY_F3		60
#define KEY_F4		61
#define KEY_F5		62
#define KEY_F6		63
#define KEY_F7		64
#define KEY_F8		65
#define KEY_F9		66
#define KEY_F10		67
#define KEY_F11		68
#define KEY_F12		69
#define KEY_PrtScr	70		
#define KEY_scrlck	71		
#define KEY_break	72
#define KEY_ins		73	
#define KEY_home	74
#define KEY_pgup	75
#define KEY_del		76
#define KEY_end		77
#define KEY_pgdn	78	
#define KEY_rarr	79
#define KEY_larr	80
#define KEY_darr	81
#define KEY_uarr	82
#define KEY_numlock	83
#define KEY_KPslash	84	
#define KEY_KPast	85	
#define KEY_KPminus	86	
#define KEY_KPplus	87	
#define KEY_KPenter	88	
#define KEY_KP1		89
#define KEY_KP2		90
#define KEY_KP3		91	
#define KEY_KP4		92	
#define KEY_KP5		93	
#define KEY_KP6		94	
#define KEY_KP7		95	
#define KEY_KP8		96	
#define KEY_KP9		97	
#define KEY_KP0		98		
#define KEY_PERIOD	99		
#define KEY_KPcomma	100
#define KEY_Euro2	101
//#define  SPC_plus	87
//#define  SPC_minus	56
//#define  SPC_pound	101	
//#define  SPC_home	
//#define  SPC_del	
//#define  SPC_ast	85
//#define  SPC_equal	67
//#define  SPC_crsrud	
//#define  SPC_crsrlr	
//#define  SPC_F1		
//#define  SPC_F3		
//#define  SPC_F5		
//#define  SPC_F7		
//#define  SPC_hat	
//#define  SPC_colon	
//#define  SPC_smcol	
//#define  SPC_at		206
*/


// Everything below this point is only intended for usb_serial.c
#ifdef USB_SERIAL_PRIVATE_INCLUDE
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#define EP_TYPE_CONTROL			0x00
#define EP_TYPE_BULK_IN			0x81
#define EP_TYPE_BULK_OUT		0x80
#define EP_TYPE_INTERRUPT_IN		0xC1
#define EP_TYPE_INTERRUPT_OUT		0xC0
#define EP_TYPE_ISOCHRONOUS_IN		0x41
#define EP_TYPE_ISOCHRONOUS_OUT		0x40

#define EP_SINGLE_BUFFER		0x02
#define EP_DOUBLE_BUFFER		0x06

#define EP_SIZE(s)	((s) == 64 ? 0x30 :	\
			((s) == 32 ? 0x20 :	\
			((s) == 16 ? 0x10 :	\
			             0x00)))

#define MAX_ENDPOINT		4

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

#if defined(__AVR_AT90USB162__)
#define HW_CONFIG() 
#define PLL_CONFIG() (PLLCSR = ((1<<PLLE)|(1<<PLLP0)))
#define USB_CONFIG() (USBCON = (1<<USBE))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#elif defined(__AVR_ATmega32U4__)
#define HW_CONFIG() (UHWCON = 0x01)
#define PLL_CONFIG() (PLLCSR = 0x12)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#elif defined(__AVR_AT90USB646__)
#define HW_CONFIG() (UHWCON = 0x81)
#define PLL_CONFIG() (PLLCSR = 0x1A)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#elif defined(__AVR_AT90USB1286__)
#define HW_CONFIG() (UHWCON = 0x81)
#define PLL_CONFIG() (PLLCSR = 0x16)
#define USB_CONFIG() (USBCON = ((1<<USBE)|(1<<OTGPADE)))
#define USB_FREEZE() (USBCON = ((1<<USBE)|(1<<FRZCLK)))
#endif

// standard control endpoint request types
#define GET_STATUS			0
#define CLEAR_FEATURE			1
#define SET_FEATURE			3
#define SET_ADDRESS			5
#define GET_DESCRIPTOR			6
#define GET_CONFIGURATION		8
#define SET_CONFIGURATION		9
#define GET_INTERFACE			10
#define SET_INTERFACE			11
// HID (human interface device)
#define HID_GET_REPORT			1
#define HID_GET_IDLE			2
#define HID_GET_PROTOCOL		3
#define HID_SET_REPORT			9
#define HID_SET_IDLE			10
#define HID_SET_PROTOCOL		11
// CDC (communication class device)
#define CDC_SET_LINE_CODING		0x20
#define CDC_GET_LINE_CODING		0x21
#define CDC_SET_CONTROL_LINE_STATE	0x22
#endif
#endif
