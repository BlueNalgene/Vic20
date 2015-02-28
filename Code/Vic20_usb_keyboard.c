/* 
 *  Vic20_usb_keyboard.c
 *
 *  Source file for interfacing a Commodore VIC-20 keyboard
 *  with USB through a Teensy ATmega based device.
 *
 *  This file contains code that uses the PJRC keyboard USB code
 *  to connect a Commodore VIC-20 keyboard to a Teensy++ 2.0.
 *  This in turn allows the Vic-20 keyboard to appear as a HID keyboard
 *  to a PC or Mac.
 *
 *  This project consists of four files:
 *
 *  Vic20_usb_keyboard.c      top-level source (this file)
 *  usb_keyboard.c           PJRC's USB HID keyboard source for Teensy++ 2.0
 *  usb_keyboard.h           PJRC's USB HID keyboard header file (modified)
 *  Makefile                 PJRC's makefile for building the project (modified)
 *
 *  Heavily cribbed from M100 USB Keyboard project by created by Karl Lunt
 *  (www.seanet.com/~karllunt) and Spaceman Spiff's Commodire 64 USB Keyboard
 *	by Mikkel Holm Olsen (symlink.dk)
 *
 *  I've modified the makefile by renaming the top-level source file from
 *  M100_usb_keyboard.c to Vic20_usb_keyboard.c.
 *
 *  I've modified the usb_keyboard.h file to use personalized USB IDs.
 *
 *  The Teensy++ 2.0 board is wired to the Vic20 keyboard connector so that
 *  the eight matrix columns connect to port C (PC0 through PC7).  The nine
 *  matrix rows connect to port F (PF0 through PF7) and port E (PE1).
 *  Refer to the following table.  Pin numbers on the keyboard connection
 *  can be found in the Commodore Vic-20 technical manual, but the values for one
 *  column are wrong.  
 *
 *  Port pin	Vic keyboard connection
 *  -------		------------------------
 *	PC7			pin 20 (col7)
 *	PC6			pin 19 (col1)
 *	PC5			pin 18 (col2)
 *  PC4			pin 17 (col3)
 *  PC3			pin 16 (col4)
 *  PC2			pin 15 (col5)
 *  PC1			pin 14 (col6)
 *  PC0			pin 13, 3 (col0)
 *  PE1			pin 12  (row0)
 *  PF7			pin 11  (row1)
 *  PF6			pin 10  (row2)
 *  PF5			pin 9  (row7)
 *  PF4			pin 8  (row4)
 *  PF3			pin 7  (row5)
 *  PF2			pin 6  (row6)
 *  PF1			pin 5  (row3)
 *  PF0			pin 1  (row8)
 *	
 *  From SpacemanSpiff's C64key
 *		C64 keyboard matrix
 *		-------------------
 *		
 *		The table below contains the mapping of the C64 keyboard matrix. The table has
 *		been organized based on the row/column numbers used internally in the C64 (and
 *		also in the c64key project), but does not correspond to the wiring used in the
 *		C64 keyboard matrix connector.
 *		
 *		row/col         0       1       2       3       4       5       6       7
 *		   0          NS/DEL    3       5       7       9       +       £       1
 *		   1          RETURN    W       R       Y       I       P       *       <-
 *		   2         CRSR RL    A       D       G       J       L       ;      CTRL
 *		   3            F7      4       6       8       0       -    CLR/HOME   2
 *		   4            F1      Z       C       B       M       .    R shift   SPC
 *		   5            F3      S       F       H       K       :       =       C=
 *		   6            F5      E       T       U       O       @       ^       Q
 *		   7         CRSR DU  L shift   X       V       N       ,       /    RUN/STOP
 *  
 *  Pin 2 is empty on the Commodore keyboard, and pin 4 is unused.
 *
 *  The code reads the keyboard by driving one (and only one) of the
 *  column pins low, then reading the port lines for the rows and
 *  looking for low bits; a low bit means the switch in that row at
 *  the selected column is closed.
 *
 *  For example, if only column 0 (PF0) is low and the row port shows
 *  a 0 in row 7 (PB7), then the L key is pressed.
 */

/* Keyboard example for Teensy USB Development Board
 * http://www.pjrc.com/teensy/usb_keyboard.html
 * Copyright (c) 2008 PJRC.COM, LLC
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "usb_keyboard.h"


#ifndef  FALSE
#define  FALSE	0
#define  TRUE  !FALSE
#endif

#define  NUM_ROWS				9
#define  NUM_COLS				8


/*
 *  For the Teensy++ 2.0, define port lines that control the on-board
 *  LED, used for status.
 */
#define LED_CONFIG	(DDRD |= (1<<6))
//#define LED_ON		(PORTD &= ~(1<<6))
//#define LED_OFF		(PORTD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))		// original is backwards, LED is active-high
#define LED_ON		(PORTD |= (1<<6))		// original is backwards, LED is active-high

/*
 *  Define macros for accessing the keyboard matrix.
 *
 *  These macros depend on the port assignments and M100 connector
 *  wiring.  If you rewire the cabling between the Teensy and the
 *  M100 keyboard, update these macros.
 */
#define  PORT_COL			PORTC
#define  DDR_COL			DDRC
#define  PIN_COL			PINC
#define  MASK_COL			0xff			/* uses all eight pins in row port */

#define  PORT_ROW_MSB		PORTE
#define  DDR_ROW_MSB		DDRE
#define  PIN_ROW_MSB		PINE
#define  MASK_ROW_MSB		(1<<0)			/* uses only pin 0 in col MSB port */

#define  PORT_ROW_LSB		PORTF
#define  DDR_ROW_LSB		DDRF
#define  PIN_ROW_LSB		PINF
#define  MASK_ROW_LSB		0xff			/* uses all eight pins in col LSB port */


#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))


/*
 *  Map the physical keys to rows and columns of the keyboard matrix.
 *
 *  Note that I have changed some key mappings from the original used by the M100.
 *  This is to provide missing keys (such as tilde and the curly-braces) and to
 *  compensate for errors in the keypad schematic documentation in the reference
 *  manual.
 *		row/col         0       1       2       3       4       5       6       7
 *		   0          NS/DEL    3       5       7       9       +       £       1
 *		   1          RETURN    W       R       Y       I       P       *       <-
 *		   2         CRSR RL    A       D       G       J       L       ;      CTRL
 *		   3            F7      4       6       8       0       -    CLR/HOME   2
 *		   4            F1      Z       C       B       M       .    R shift   SPC
 *		   5            F3      S       F       H       K       :       =       C=
 *		   6            F5      E       T       U       O       @       ^       Q
 *		   7         CRSR DU  L shift   X       V       N       ,       /    RUN/STOP
 *
 *  Key names shown here are the USB key names, not the TRS-80 key function names.
 *  For example, the M100 keyboard has keys labeled PASTE, LABEL, and PRINT.  I
 *  have renamed these keys to provide needed PC-101 keys.  PASTE is now {; if
 *  you shift this key, you get }.  This isn't exactly how things work on a PC-101,
 *  but it's close and doesn't conflict with the M100 physical keycaps.
 */
const uint8_t				keyMapping[NUM_ROWS][NUM_COLS]  PROGMEM  =
{
//	  col 0		col 1		col 2		col 3		col 4		col 5		col 6		col 7
//	-------------------------------------------------------------------------------------------
    {SPC_del, 	KEY_3, 		KEY_5, 		SPC_7, 		SPC_9, 		SPC_plus, 	SPC_pound, 	KEY_1}, // row0
    {KEY_enter, KEY_W, 		KEY_R, 		KEY_Y, 		KEY_I, 		KEY_P, 		SPC_ast, 	KEY_esc}, // row1
    {SPC_crsrlr, KEY_A, 	KEY_D, 		KEY_G, 		KEY_J, 		KEY_L, 		SPC_smcol, 	MOD_LCTRL}, // row2
    {SPC_F7, 	KEY_4, 		SPC_6, 		SPC_8, 		SPC_0, 		SPC_minus, 	SPC_home, 	SPC_2}, // row3
    {SPC_F1, 	KEY_Z, 		KEY_C, 		KEY_B, 		KEY_M, 		KEY_dot, 	MOD_RSHIFT, KEY_spc}, // row4
    {SPC_F3, 	KEY_S, 		KEY_F, 		KEY_H, 		KEY_K, 		SPC_colon, 	SPC_equal, 	MOD_LALT}, // row5
    {SPC_F5, 	KEY_E, 		KEY_T, 		KEY_U, 		KEY_O, 		SPC_at, 	SPC_hat, 	KEY_Q}, // row6
    {SPC_crsrud, MOD_LSHIFT, KEY_X, 	KEY_V, 		KEY_N, 		KEY_comma, 	KEY_slash, 	MOD_RALT}, // row7
    {MOD_RCTRL, 0, 			0, 			0, 			0, 			0, 			0, 			0} // Imaginary row8 is for restore
};
/* M100 keymap
	{KEY_Z,		KEY_X,		KEY_C,		KEY_V,		KEY_B,		KEY_N,		KEY_M,		KEY_L},
	{KEY_A,		KEY_S,		KEY_D,		KEY_F,		KEY_G,		KEY_H,		KEY_J,		KEY_K},
	{KEY_Q,		KEY_W,		KEY_E,		KEY_R,		KEY_T,		KEY_Y,		KEY_U,		KEY_I},
	{KEY_O,		KEY_P,		KEY_LEFT_BRACE,	KEY_SEMICOLON,	KEY_QUOTE,	KEY_COMMA,	KEY_PERIOD},
	{KEY_1,		KEY_2,		KEY_3,		KEY_4,		KEY_5,		KEY_6,		KEY_7,		KEY_8},
	{KEY_9,		KEY_0,		KEY_MINUS,	KEY_EQUAL,	KEY_LEFT,	KEY_RIGHT,	KEY_UP,		KEY_DOWN},
	{KEY_SPACE,	KEY_BACKSPACE,	KEY_TAB,	KEY_ESC,	KEY_F9,		KEY_BACKSLASH,	KEY_PRINTSCREEN,	KEY_ENTER,		0},
	{KEY_F1,	KEY_F2,		KEY_F3,		KEY_F4,		KEY_F5,		KEY_F6,		KEY_F7,		KEY_F8,		0},
	{KEY_SHIFT,	KEY_LEFT_CTRL,	KEY_LEFT_ALT,	KEY_RIGHT_ALT,	KEY_NUM_LOCK,	KEY_CAPS_LOCK,	0,	KEY_TILDE,		0}
	*/


/*
 *  Map of M100 keys to PC-101 keys (shows only unusual or M100-specific keys; see modifyKeyPressForM100())
 *
 *      M100 key		PC-101 key
 *  -----------------	----------
 *		BKSP			backspace
 *		shift-BKSP		delete
 *		GRPH			left-ALT
 *		CODE			right-ALT
 *		CAPS LOCK		Caps Lock
 *		NUM				Num Lock
 *		PASTE			{  (left curly-brace)
 *		shift-PASTE		}  (right curly-brace)
 *		alt-PASTE		F9
 *		[				[  (left bracket)
 *		shift-[			]  (right bracket)
 *		LABEL			\  (backslash)
 *		shift-LABEL		|  (vertical bar)
 *		alt-LABEL		F10
 *		alt-PRINT		F11
 *		PAUSE			`  (back-tick in Forth)
 *		shift-PAUSE		~  (tilde)
 *		alt-PAUSE		F12
 *		shift- <--		Home
 *		shift- -->		End
 *		shift- up-arrow	Page Up
 *		shift- dn-arrow	Page Down
 */


#define  COL_MODIFIERS		7						/* M100 layout places all modifiers in col 8 */
#define  ROW_RIGHT_SHIFT	0						/* row for right shift key */
#define  ROW_LEFT_SHIFT		0						/* both SHIFT keys are wired together */
#define  ROW_LEFT_CTRL		1						/* row for left-side CTRL key */
#define  ROW_LEFT_ALT		2						/* this is actually labeled GRPH on M100 keyboard */
#define  ROW_RIGHT_ALT		3						/* this is actually labeled CODE on M100 keyboard */

#define  MASK_ALL_MODIFIERS  ((1<<ROW_RIGHT_SHIFT) | (1<<ROW_LEFT_SHIFT) | \
							  (1<<ROW_LEFT_CTRL) | (1<<ROW_LEFT_ALT) | (1<<ROW_RIGHT_ALT))


/*
 *  Global variables
 */
uint16_t			rowData;
uint16_t			colData;
uint8_t				prevRowData[NUM_COLS];				// holds row data from previous scan
uint8_t				currRowData[NUM_COLS];				// holds current row data


/*
 *  Local functions
 */
void				scanKeyboard(void);					// update globals with current scan info
uint8_t				modifyKeyPress(uint8_t  key);	// create psuedo keys for pressing
uint8_t				modifyKeyRelease(uint8_t  key);	// create psuedo keys for releasing
void				processSpecialKeys(uint8_t  key);	// may need to send release action for special keys



int main(void)
{
	uint8_t			n;

	CPU_PRESCALE(0);					// set for 16 MHz clock
	LED_OFF;
	LED_CONFIG;

/*
 *  Configure the row port for input; use internal pullups.
 *  Configure the column ports for output.
 */
	DDR_COL = 0x00;						// set row port as inputs
	PORT_COL = 0xff;					// turn on pullups
	DDR_ROW_MSB = MASK_ROW_MSB;   
	DDR_ROW_LSB = MASK_ROW_LSB;

	// Initialize the USB, and then wait for the host to set configuration.
	// If the Teensy is powered without a PC connected to the USB port,
	// this will wait forever.
	usb_init();
	while (!usb_configured()) /* wait */ ;

	// Wait an extra second for the PC's operating system to load drivers
	// and do whatever it does to actually be ready for input
	_delay_ms(1000);


	for (n=0; n<NUM_COLS; n++)  prevRowData[n] = 0xff;		// begin with no key pressed

	while (1)
	{
		scanKeyboard();
		_delay_ms(40);
	}
}



/*
 *  scanKeyboard      scan the keyboard matrix, determine if any key has changed
 *
 *  This routine steps through each column in the keyboard matrix by pulling
 *  successive bits in the column I/O ports low, then recording the value of
 *  the row input port.  After a scan is done, the array currRowData[] holds
 *  all the scan info.
 *
 *  This routine then determines if a key change has occurred.  If so, the
 *  appropriate key actions are sent as USB packets to the PC.
 */


void  scanKeyboard(void)
{
	uint16_t				n;
	uint8_t					coln;
	uint8_t					rown;
	uint16_t				mask;
	uint8_t					k;
	uint8_t					needToProcess;
	volatile uint16_t		delay;

	needToProcess = FALSE;				// nothing to do yet
	for (n=0; n<NUM_COLS; n++)			// for all columns...
	{
		mask = 0xffff & ~(1<<n);		// get a bit mask for selected column (active low)
		PORT_ROW_LSB = (mask & MASK_ROW_LSB);		// set LSB
		PORT_ROW_MSB = (mask >> 8) & MASK_ROW_MSB;	// set MSB
		for (delay=0; delay<500; delay++)  ;
		currRowData[n] = PIN_COL;		// get the scan result for that column
		if (currRowData[n] != prevRowData[n])	// if there is a difference...
		{
//			if ((n != COL_MODIFIERS) || 
//				((~currRowData[n] | MASK_ALL_MODIFIERS) != MASK_ALL_MODIFIERS))	// and not a modifier...
//			{
				needToProcess = TRUE;
//			}
		}
	}
	    
	PORT_ROW_LSB = 0xff;				// done for now, pull all columns high
	PORT_ROW_MSB = MASK_ROW_MSB;

	if (needToProcess)					// if something to do...
	{
		for (n=0; n<6; n++)  keyboard_keys[n] = 0;	// magic number; clear out all keys in USB buffer

//
//  All of the modifier keys, such as LEFT_CTRL, are in column 8,
//   so check the modifiers first.  Save the state of all modifiers in
//   the global variable keyboard_modifiers_keys, used by the USB library.
//
		mask = currRowData[COL_MODIFIERS];		// reuse mask for brevity
		keyboard_modifier_keys = 0;				// start with no modifiers pressed
		if ((mask & (1<<ROW_LEFT_SHIFT)) == 0)   keyboard_modifier_keys |= MOD_LSHIFT;
		if ((mask & (1<<ROW_RIGHT_SHIFT)) == 0)  keyboard_modifier_keys |= MOD_RSHIFT;
		if ((mask & (1<<ROW_LEFT_CTRL)) == 0)    keyboard_modifier_keys |= MOD_LCTRL;
		if ((mask & (1<<ROW_LEFT_ALT)) == 0)     keyboard_modifier_keys |= MOD_LALT;
		if ((mask & (1<<ROW_RIGHT_ALT)) == 0)    keyboard_modifier_keys |= MOD_RALT;
//		if (keyboard_modifier_keys)  usb_keyboard_send();	// if any modifier changed, update host now

		for (coln=0; coln<NUM_COLS; coln++)		// for all columns...
		{
			for (rown=0; rown<NUM_ROWS; rown++)		// for all rows...
			{
//
//  The following block only sends a key action (pressed or released) for keys
//  that are not in the modifier group.  We don't send key actions for modifiers;
//  current state of the modifier keys is already collected in the keyboard_modifier_keys
//  variable above.
//
				if ((coln == COL_MODIFIERS) && ((1<<rown) & MASK_ALL_MODIFIERS)) continue;  // ignore modifiers

				if ((currRowData[coln] & (1<<rown)) != (prevRowData[coln] & (1<<rown)))	// if key changed...
				{
					k = pgm_read_byte(&keyMapping[coln][rown]);	// get first draft of key
					if (currRowData[coln] & (1<<rown))		// if this key was just released...
					{
						k = modifyKeyRelease(k);		// if needed, modify key and modifiers
						keyboard_keys[0] = k;
						usb_keyboard_send();
						LED_OFF;
					}
					else				  					// key was just pressed...
					{
						k = modifyKeyPress(k);		// if needed, modify key and modifiers
						keyboard_keys[0] = k;
						usb_keyboard_send();
						LED_ON;
					}
					processSpecialKeys(k);					// may need to do extra processing...
				}
			}
		} 
	}
	for (n=0; n<NUM_COLS; n++)  prevRowData[n] = currRowData[n];	// record as previous data
}





/*
 *  modifyKeyPressForM100      adjust the value for a pressed key based on context
 *
 *  This routine uses the current key and the current state of the modifier keys to
 *  select, if needed, an alternate key.  This is needed because of the limited number
 *  of keys on the M100 keyboard and also because the M100 keyboard layout is not
 *  compatible with the standard PC-101 keyboard.
 *
 *  For example, the M100 keyboard has the right-brace (right-square-bracket) key
 *  and the left-brace key on the same keycap, whereas the PC-101 layout uses separate
 *  keycaps.
 *
 *  Upon entry, argument key holds the current key value (as in KEY_A) and the global
 *  variable keyboard_modifier_keys holds the current state of all modifier keys.
 *
 *  Upon exit, this routine returns the changed key value (if necessary).  This routine
 *  may also alter the mask in the keyboard_modifier_keys variable, to remove
 *  shift or ctrl key states.
 */
  /* 
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
  Special keys that need to generate different scan-codes for unshifted
   and shifted states, or that need to alter the modifier keys. 
   Since the LGUI and RGUI bits are not used, these signify that the
   left and right shift states should be deleted from report, so
     0x88 means clear both shift flags
     0x00 means do not alter shift states
     0xC8 means clear both shifts and set L_ALT */
/*
const unsigned char spec_keys[23][4] PROGMEM = {
  { KEY_2,       0x00, KEY_ping,    0x00}, // SPC_2 - shift-2 is "
  { KEY_6,       0x00, KEY_7,       0x00}, // SPC_6 - shift-6 is &
  { KEY_7,       0x00, KEY_ping,    0x88}, // SPC_7 - shift-7 is '
  { KEY_8,       0x00, KEY_9,       0x00}, // SPC_8 - shift-8 is (
  { KEY_9,       0x00, KEY_0,       0x00}, // SPC_9 - shift-9 is )
  { KEY_0,       0x00, KEY_0,       0x88}, // SPC_0 - shift-0 is 0
  { KEY_equal,   0x02, KEY_equal,   0x8A}, // SPC_plus 
  { KEY_minus,   0x00, KEY_minus,   0x88}, // SPC_minus - "-" and "-"
  { KEY_grave,   0x02, KEY_grave,   0x8A}, // SPC_pound - "~"
  { KEY_home,    0x80, KEY_end,     0x80}, // SPC_home - home and end
  { KEY_bckspc,  0x00, KEY_del,     0x88}, // SPC_del - backspace and delete
  { KEY_8,       0x02, KEY_8,       0x02}, // SPC_ast - "*" (Asterix)
  { KEY_equal,   0x00, KEY_equal,   0x88}, // SPC_equal - "="
  { KEY_darr,    0x80, KEY_uarr,    0x80}, // SPC_crsrud - cursor down/up
  { KEY_rarr,    0x80, KEY_larr,    0x80}, // SPC_crsrlr - cursor right/left
  { KEY_F1,      0x80, KEY_F2,      0x80}, // SPC_F1 - F1 and F2
  { KEY_F3,      0x80, KEY_F4,      0x80}, // SPC_F3 - F3 and F4
  { KEY_F5,      0x80, KEY_F6,      0x80}, // SPC_F5 - F5 and F6
  { KEY_F7,      0x80, KEY_F8,      0x80}, // SPC_F7 - F7 and F8
  { KEY_6,       0x02, KEY_6,       0x00}, // SPC_hat - "^"
  { KEY_smcol,   0x02, KEY_lbr,     0x88}, // SPC_colon - : and [
  { KEY_smcol,   0x00, KEY_rbr,     0x88}, // SPC_smcol - ; and ]
  { KEY_2,       0x8A, KEY_2,       0x8A}, // SPC_at - @
 };
 */
uint8_t  modifyKeyPress(uint8_t  k)
{
	switch(k)
	{
		case  KEY_2:
		if (keyboard_modifier_keys & (MOD_LSHIFT|MOD_RSHIFT))	// shift-backspace = delete
		{
			k = KEY_ping;
			keyboard_modifier_keys &= ~(MOD_LSHIFT|MOD_RSHIFT);	// must remove shift modifiers!
		}
		break;

		case  KEY_6:
		if (keyboard_modifier_keys & (MOD_LSHIFT|MOD_RSHIFT))	// shift-up = page-up
		{
			k = KEY_pgup;							// pretend we have a page-up key
			keyboard_modifier_keys &= ~(MOD_LSHIFT|MOD_RSHIFT);	// must remove shift modifiers!
		}
		break;

		case  KEY_darr:
		if (keyboard_modifier_keys & (MOD_LSHIFT|MOD_RSHIFT))	// shift-down = page-down
		{
			k = KEY_pgdn;							// pretend we have a page-down key
			keyboard_modifier_keys &= ~(MOD_LSHIFT|MOD_RSHIFT);	// must remove shift modifiers!
		}
		break;

		case  KEY_rarr:
		if (keyboard_modifier_keys & (MOD_LSHIFT|MOD_RSHIFT))	// shift-right = end
		{
			k = KEY_end;
			keyboard_modifier_keys &= ~(MOD_LSHIFT|MOD_RSHIFT);	// must remove shift modifiers!
		}
		break;

		case  KEY_larr:
		if (keyboard_modifier_keys & (MOD_LSHIFT|MOD_RSHIFT))	// shift-left = home
		{
			k = KEY_home;
			keyboard_modifier_keys &= ~(MOD_LSHIFT|MOD_RSHIFT);	// must remove shift modifiers!
		}
		break;

		case  KEY_lbr:
		if (keyboard_modifier_keys & (MOD_LSHIFT|MOD_RSHIFT))	// shift-[ = ]
		{
			k = KEY_rbr;
			keyboard_modifier_keys &= ~(MOD_LSHIFT|MOD_RSHIFT);	// must remove shift modifiers!
		}
		break;

		case  KEY_F9:										// "PASTE", 1st key in 3rd function key group
		if (keyboard_modifier_keys & (MOD_LSHIFT|MOD_RSHIFT))	// shift-PASTE = }
		{
			k = KEY_rbr;							// this would be SHIFT-] on PC-101 kbd
		}
		else if (keyboard_modifier_keys &(MOD_LALT|MOD_RALT))	// ALT-PASTE = F9
		{
			k = KEY_F9;
			keyboard_modifier_keys &= ~(MOD_LALT|MOD_RALT);	// must remove ALT modifiers!
		}
		else												// not shifted, PASTE becomes {
		{
			k = KEY_lbr;
			keyboard_modifier_keys |= MOD_LSHIFT;		// this would be SHIFT-[ on PC-101 kbd
		}
		break;

		case  KEY_bckslsh:								// "LABEL", 2nd key in 3rd function key group
		if (keyboard_modifier_keys &(MOD_LALT|MOD_RALT))	// ALT-LABEL = F10
		{
			k = KEY_F10;
			keyboard_modifier_keys &= ~(MOD_LALT|MOD_RALT);	// must remove ALT modifiers!
		}
		break;

		case  KEY_PrtScr:								// "PRINT", 3rd key in 3rd function key group
		if (keyboard_modifier_keys &(MOD_LALT|MOD_RALT))	// ALT-PRINT = F11
		{
			k = KEY_F11;
			keyboard_modifier_keys &= ~(MOD_LALT|MOD_RALT);	// must remove ALT modifiers!
		}
		break;

		case  KEY_grave:									// "BREAK", 4th key in 3rd function key group
		if (keyboard_modifier_keys &(MOD_LALT|MOD_RALT))	// ALT-BREAK = F12
		{
			k = KEY_F12;
			keyboard_modifier_keys &= ~(MOD_LALT|MOD_RALT);	// must remove ALT modifiers!
		}
		break;

		default:
		break;
	}
	return  k;
}


/*
 *  modifyKeyReleaseForM100      adjust the value of a released key based on context
 *
 *  This routine alters (if needed) the value of the key that is being released.  This
 *  is required because two keys, CAPS and NUM, are push-on/push-off keys on the
 *  M100 keyboard, unlike the PC-101 keyboard, where these are soft keys.
 *
 *  This means that, for these two keys, the code must send the key press on every
 *  state change of the physical key.  For example, this code must send a KEY_NUM_LOCK
 *  every time the NUM key is pressed, whether that means the key is down or up.
 *
 *  However, because these are hardware keys, it is possible for the host and the
 *  M100 keyboard to get out of sync.  For example, if you power up the M100 with the
 *  NUM key locked down, then release it, the host will get a KEY_NUM_LOCK packet and
 *  will assume the NUM-LOCK key has just been pressed, putting the keyboard in NUM-LOCK
 *  state.  This will be backwards from the physical state of the M100 keyboard, so
 *  subsequent keys may be misrepresented.  The only cure will be to disconnect the
 *  M100 keyboard, put the NUM key in its up (unlocked) position, then reconnect
 *  the M100 keyboard.
 *
 *  Upon entry, variable key holds the value of the key involved and global variable
 *  keyboard_modifier_keys holds the current state of the modifier keys (SHIFT, CTRL).
 *
 *  Upon exit, this routine will return the new value of key, which is typically 0
 *  for all released keys.  For special keys, however, this code will alter the value
 *  in key and may also alter the mask stored in keyboard_modifier_keys.
 */
uint8_t  modifyKeyRelease(uint8_t  key)
{
	switch(key)
	{
		case  KEY_cpslck:	// send this key, even on release
		case  KEY_numlock:		// send this key, even on release
		break;

		default:				// the normal action on release is not to send anything
		key = 0;				// return a value of 0 to surpress sending a key packet
		break;
	}
	return  key;
}



/*
 *  processSpecialKeys      post-processing, used only for certain keys
 *
 *  Some keys, such as the CAPS and NUM keys, require additional processing
 *  after the first key action has been sent.  In these two cases, the key
 *  action is to send the key (say, KEY_CAPS_LOCK) when the key changes state.
 *  This will be interpreted by the PC as a key press.  However, the key
 *  will remain pressed, as far as the PC is concerned.  We need to send a
 *  second key action that tells the host the key has been released.  If we
 *  don't, the host will assume you are holding down the key.  (Remember, on
 *  a PC-101 keyboard, CAPS-LOCK and NUM-LOCK are soft keys, not push-on/
 *  push-off, as on the M100 keyboard.)
 *
 *  Upon entry, argument key holds the key code for the key in question.
 */
void  processSpecialKeys(uint8_t  key)
{
	switch (key)
	{
		case  KEY_cpslck:
		case  KEY_numlock:
		keyboard_keys[0] = 0;
		usb_keyboard_send();
		break;

		default:
		break;
	}
}





