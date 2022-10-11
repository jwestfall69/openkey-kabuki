#include <avr/sleep.h>

// PIN 8 (PIN_B1) <=> Kabuki PIN 23 (BUSACK)
#define SET_BUSACK    PORTB.OUTSET = PIN1_bm
#define CLR_BUSACK    PORTB.OUTCLR = PIN1_bm

// PIN 9 (PIN_B0) <=> Kabuki PIN 24 (WAIT)
#define SET_WAIT      PORTB.OUTSET = PIN0_bm
#define CLR_WAIT      PORTB.OUTCLR = PIN0_bm

// PIN 10 (PIN_A0) used for UPDI (ATtiny programming)

// PIN 11 (PIN_A1) <=> Kabuki PIN 25 (BUSREQ)
#define SET_BUSREQ    PORTA.OUTSET = PIN1_bm
#define CLR_BUSREQ    PORTA.OUTCLR = PIN1_bm

// PIN 12 (PIN_A2) <=> Kabuki PIN 26 (RESET)
#define SET_RESET     PORTA.OUTSET = PIN2_bm
#define CLR_RESET     PORTA.OUTCLR = PIN2_bm

// PIN 13 (PIN_A3) <=> Kabuki PIN 27 (M1)
#define SET_M1        PORTA.OUTSET = PIN3_bm
#define CLR_M1        PORTA.OUTCLR = PIN3_bm

// First 4 fields come from mame @
// https://github.com/mamedev/mame/blob/master/src/mame/capcom/kabuki.cpp#L73
// mem_mask field comes from Eduardo Cruz (ArcadeHacker). 
// https://github.com/ArcadeHacker/ArcadeHacker_Kabuki/blob/master/ArcadeHacker_Kabuki.ino#L33
typedef struct game_key_t {
  uint32_t swap_key1;
  uint32_t swap_key2;
  uint16_t address_key;
   uint8_t xor_key;
  uint16_t mem_mask;
} game_key;

#define MAX_GAME_NUM   20
game_key game_keys[MAX_GAME_NUM + 1] = {
  // mitchell games                                      dsw
  //   skey1       skey2     addr    xor   mask         12345
  { 0x02461357, 0x64207531, 0x0002, 0x01, 0x7000 },  // 00000 [BLx] Block Block (BLE, BLJ)
  { 0x01234567, 0x76543210, 0x6548, 0x24, 0x7000 },  // 00001 [CBJ] Capcom Baseball
  { 0x04152637, 0x40516273, 0x5751, 0x43, 0x7000 },  // 00010 [CW]  Capcom World
  { 0x76543210, 0x01234567, 0xaa55, 0xa5, 0x7000 },  // 00011 [D2]  Dokaben 2
  { 0x76543210, 0x01234567, 0xaa55, 0xa5, 0x7000 },  // 00100 [DB]  Dokaben
  { 0x76543210, 0x01234567, 0xaa55, 0xa5, 0x7000 },  // 00101 [MG2] Mahjong Gakuen 2 Gakuen-chou no Fukushuu
  { 0x54321076, 0x54321076, 0x4854, 0x4f, 0x7000 },  // 00110 [MG3] Super Marukin-Ban
  { 0x76543210, 0x01234567, 0xaa55, 0xa5, 0x7000 },  // 00111 [PKx] Poker Ladies (PK, PKO)
  { 0x01234567, 0x76543210, 0x6548, 0x24, 0xf000 },  // 01000 [PWx] Pang / Buster Bros / Pomping World (PWE, PWJ, PWU)
  { 0x45670123, 0x45670123, 0x5751, 0x43, 0x7000 },  // 01001 [Q2]  Adventure Quiz 2 Hatena ? no Dai-Bouken
  { 0x12345670, 0x12345670, 0x1111, 0x11, 0x7000 },  // 01010 [Q3]  Quiz Tonosama no Yabou
  { 0x23456701, 0x23456701, 0x1828, 0x18, 0x7000 },  // 01011 [Q4]  Quiz Sangokushi
  { 0x45670123, 0x45670123, 0x5852, 0x43, 0x7000 },  // 01100 [SPE] Super Pang World
  { 0x45123670, 0x67012345, 0x55aa, 0x5a, 0x7000 },  // 01101 [SPJ] Super Pang Japan
  { 0x45670123, 0x45670123, 0x2130, 0x12, 0x7000 },  // 01110 [SPU] Super Buster Bros
  { 0x00000000, 0x00000000, 0x0000, 0x00, 0x0000 },  // 01111 [???] Ashita Tenki ni Naare (unknown)

  // cps1.5 games                                        dsw
  //   skey1       skey2     addr    xor   mask         12345
  { 0x76543210, 0x24601357, 0x4343, 0x43, 0xff00 },  // 10000 [CDx] Cadillacs and Dinosaurs (CDE, CDJ, CDT, CDU)
  { 0x54321076, 0x65432107, 0x3131, 0x19, 0xff00 },  // 10001 [MBx] Slam Masters (MBDE, MBDJ, MBE, MBJ, MBU)
  { 0x67452103, 0x75316024, 0x2222, 0x22, 0xff00 },  // 10010 [PSx] Punisher (PS, PSE, PSH, PSU)
  { 0x01234567, 0x54163072, 0x5151, 0x51, 0xff00 },  // 10011 [TKx] Warrior of Fate (TK2A, TK2E, TK2J, TK2U)

  // bad dsw value / zero key
  { 0x00000000, 0x00000000, 0x0000, 0x00, 0x0000 }
};

// doing bit shifting takes a variable amount
// of time depending on the how far the shift
// is.  Using the below lookup table to keep
// a consistent clock period when masking a
// bit to send. 
uint32_t masks[32] = {
  0x00000001, 0x00000002, 0x00000004, 0x00000008,
  0x00000010, 0x00000020, 0x00000040, 0x00000080,
  0x00000100, 0x00000200, 0x00000400, 0x00000800,
  0x00001000, 0x00002000, 0x00004000, 0x00008000,
  0x00010000, 0x00020000, 0x00040000, 0x00080000,
  0x00100000, 0x00200000, 0x00400000, 0x00800000,
  0x01000000, 0x02000000, 0x04000000, 0x08000000,
  0x10000000, 0x20000000, 0x40000000, 0x80000000,
};

void setup() {
  
  // wait a bit for other IC's on the board to get
  // in their inital states
  delay(100);

  // wait for the WAIT pin on the kabuki to go high
  // before we do anything
  while(!PORTB.IN & PIN0_bm) { }
  
  // set the pins to kabuki as outputs
  PORTA.DIR = PIN1_bm | PIN2_bm | PIN3_bm;
  PORTB.DIR = PIN0_bm | PIN1_bm;

  // stage #1: set initial output states
  // https://www.youtube.com/watch?v=9t-9hQEOjLI&t=754s
  CLR_RESET;
  CLR_WAIT;
  CLR_BUSREQ;
  SET_M1;
  SET_BUSACK;

  // enable pull-ups on dsw pins
  PORTA.PIN4CTRL |= PORT_PULLUPEN_bm;  // PIN 2 (PIN_PA4) <=> DSW5
  PORTA.PIN5CTRL |= PORT_PULLUPEN_bm;  // PIN 3 (PIN_PA5) <=> DSW4
  PORTA.PIN6CTRL |= PORT_PULLUPEN_bm;  // PIN 4 (PIN_PA6) <=> DSW3
  PORTA.PIN7CTRL |= PORT_PULLUPEN_bm;  // PIN 5 (PIN_PA7) <=> DSW2
  PORTB.PIN3CTRL |= PORT_PULLUPEN_bm;  // PIN 6 (PIN_PB3) <=> DSW1

  // stage #2: door knock #1
  // https://www.youtube.com/watch?v=9t-9hQEOjLI&t=871s
  for(int i = 0;i < 3;i++) {
    SET_BUSREQ;
    CLR_BUSREQ;
  }

  // stage #3: unlock key
  // https://www.youtube.com/watch?v=9t-9hQEOjLI&t=1274s
  send_data(0xb7a45, 20, -1);

  // stage #4: door knock #2
  // https://www.youtube.com/watch?v=9t-9hQEOjLI&t=1433s
  for(int i = 0;i < 10;i++) {
    CLR_M1;
    SET_M1;
  }

  uint8_t game_num = read_dsw();
  if(game_num > MAX_GAME_NUM) {
    game_num = MAX_GAME_NUM;
  }

  // stage #5: 108-bit programming
  // https://www.youtube.com/watch?v=9t-9hQEOjLI&t=1518s
  send_data(0x0, 20, -1);
  send_data(game_keys[game_num].address_key, 16, -1);

  // mame swap keys are 32bit, but the actual swap key is 24bit.
  // the conversion process is to drop the top bit from each nibble.
  // below we are looping over each nibble and only sending the
  // lower 3 bits.
  for(int start_bit = 30; start_bit > 0; start_bit -= 4) {
    send_data(game_keys[game_num].swap_key2, 3, start_bit);
  }

  for(int start_bit = 30; start_bit > 0; start_bit -= 4) {
    send_data(game_keys[game_num].swap_key1, 3, start_bit);
  }

  send_data(game_keys[game_num].xor_key, 8, -1);
  send_data(game_keys[game_num].mem_mask, 16, -1);

  // set all pins to be inputs
  PORTA.DIR = 0;
  PORTB.DIR = 0;

  // power down
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_cpu();
}

void loop() {
  while(1) {}
}

// send bits of data starting at start_bit and going MSB to LSB.
// if start_bit is -1, send the last LSB num_bits bits of the data
void send_data(uint32_t data, int8_t num_bits, int8_t start_bit) {
  num_bits--;

  if(start_bit == -1) {
    start_bit = num_bits;
  }

  while(num_bits >= 0) {
    uint32_t bit = data & masks[start_bit];
    if(bit) {
      SET_M1;
    } else {
      CLR_M1;
    }
    CLR_BUSACK;
    SET_BUSACK;
    num_bits--;
    start_bit--;
  }
}

// DSW1 = PB3 = bit4
// DSW2 = PA7 = bit3
// DSW3 = PA6 = bit2
// DSW4 = PA5 = bit1
// DSW5 = PA4 = bit0
uint8_t read_dsw() {
  uint8_t dsw = 0;

  // negate the bits since we are using
  // pull-ups, "on"s will be low
  uint8_t porta = PORTA.IN ^ 0xff;
  uint8_t portb = PORTB.IN ^ 0xff;

  dsw = porta >> 4;
  dsw = dsw | ((portb & 0b0001000) << 1);
  return dsw;
}
