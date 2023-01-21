#pragma once

const int MEM_SIZE = 0x10000;

enum SIZES{
  BYTE = 1,
  WORD = 2
};

enum INSTRUCTIONS{
  HALT = 0,
  INT = 1,
  IRET = 2,
  CALL = 3,
  RET = 4,
  JUMP = 5,
  XCHG = 6,
  ARIT = 7,
  LOG = 8,
  SHIFT = 9,
  LDR = 10,
  STR = 11
};

enum JUMPS{
  JMP = 0,
  JEQ = 1,
  JNE = 2,
  JGT = 3
};

enum ARITHMETICS{
  ADD = 0,
  SUB = 1,
  MUL = 2,
  DIV = 3,
  CMP = 4
};

enum LOGICS{
  NOT = 0,
  AND = 1,
  OR = 2,
  XOR = 3,
  TEST = 4
};

enum SHIFTS{
  SHL = 0,
  SHR = 1
};

enum ADDR{
  neposredno = 0,
  reg_dir = 1,
  reg_dir_pom = 5,
  reg_ind = 2,
  reg_ind_pom = 3,
  mem = 4
};

enum FLAGS{
  Z_FLAG = 0,
  O_FLAG = 2,
  C_FLAG = 4,
  N_FLAG = 8,
  Tr_FLAG = 1 << 13,
  Tl_FLAG = 1 << 14,
  I_FLAG = 1<< 15
};

class Emulator{
  public:
  int initMemorija(string input);
  int initPcPswSp();
  int emuliraj();
  int fetch();
  int execute();

  int procitajMemoriju(unsigned short adr, SIZES size);
  int upisiuMemoriju(unsigned short adr, SIZES size, unsigned short value);
  int skociNaAdresu();
  int obradiPrekide();


  private:
  unsigned short regs[9];
  char memorija[MEM_SIZE];
  unsigned short* pc = &regs[7];
  unsigned short* sp = &regs[6];
  unsigned short* psw = &regs[8];

  unsigned char instrDescr;
  unsigned char regsDescr;
  unsigned char addrMode;
  unsigned char dataHigh;
  unsigned char dataLow;
  string insName;
  int insSize;

  bool running = false;
  bool greska_instrukcija = false;
  bool greska_timer = false;
  bool greska_terminal = false;
  bool greska_ostali = false;

};