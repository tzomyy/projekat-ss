#include <iostream>
#include <fstream>
#include <bitset>
#include  <iomanip>


using namespace std;

#include "../inc/emulator.hpp"


int Emulator::initMemorija(string input){
  ifstream in;
  in.open(input);

  if (!in.is_open()){
    cout << "Greska: Greska pri otvaranju fajla " << input << "\n"  ;
  }

  unsigned char c;
  int filesize = 0;
  while(1){
    in.read((char*)&c, sizeof(c));
    if (c == '\n') break;
    //cout << hex << c - 0 << " ";
    filesize++;
  }

  if (filesize < MEM_SIZE - 256){
    in.seekg(0, in.beg);
    in.read((char*)&memorija, filesize);
  }else{
    cout << "Velicina fajla je prevelika" << endl;
    return -1;
  }

  // for (int i=0; i<filesize; i++){
  //   cout << hex << (unsigned char)memorija[i] - 0 << " ";
  // }  
  return 0;
}

int Emulator::initPcPswSp(){
  unsigned char low = memorija[0];
  unsigned char high = memorija[1];

  *pc = (high<<8)|low;
  *sp = 0xff00;
  *psw = 0x00;

  return 0;
}

int Emulator::skociNaAdresu(){

  unsigned short payload;
  if (this->insSize == 5){
    payload = (this->dataHigh<< 8)| this->dataLow; 
  }

  unsigned int sReg = this->regsDescr & 0xf;
  unsigned char nacin_adr = this->addrMode & 0xf;

  switch (nacin_adr)
  {
  case neposredno:
    *pc = payload;
    break;
  case reg_dir:
    *pc = regs[sReg];
    break;
  case reg_dir_pom:
    *pc = regs[sReg] + payload;
    break;
  case reg_ind:
    *pc = procitajMemoriju(regs[sReg], WORD);
    break;
  case reg_ind_pom:
    *pc = procitajMemoriju(regs[sReg] + payload, WORD);
    break;
  case mem:
    *pc = procitajMemoriju(payload, WORD);
    break;
  }
  return 0;
}

int Emulator::procitajMemoriju(unsigned short adr, SIZES size){
  int ret;

  switch (size)
  {
  case BYTE:
    ret = memorija[adr];
    break;
  case WORD:
    ret = (unsigned char)memorija[adr];
    ret |= (unsigned short)(memorija[adr + 1] << 8);
    break;
  default:
    cout << "Nemoguce je procitati toliko podataka" << endl;
    ret = -1;
    break;
  }
  return ret;
}

int Emulator::upisiuMemoriju(unsigned short adr, SIZES size, unsigned short value){
  int ret;

  switch (size)
  {
  case BYTE:
    memorija[adr] = (unsigned char)value;
    break;
  case WORD:
    memorija[adr] = (unsigned char)value;
    memorija[adr + 1] = (unsigned char)(value >> 8);
    //if (adr = 0xfefc) cout << "INSTRUKCIJA: " <<  this->insName << endl;
    // cout << "ADR: " << hex << memorija[adr] -0 << memorija[adr+1] - 0 << endl;
    break;
  default:
    cout << "Nemoguce je procitati toliko podataka" << endl;
    ret = -1;
    break;
  }
  return ret;
}

int Emulator::fetch(){
  int ret = 0;

 
  this->instrDescr = procitajMemoriju(*pc, BYTE);
  (*pc)++;
  
  unsigned char opCode = this->instrDescr >> 4;
  unsigned char modifikator = this->instrDescr & 0xf;
  
  switch (opCode)
  {
  case HALT:{   
    if (modifikator != 0) {
      cout << modifikator - 0 << endl;
      this->greska_instrukcija = true;
      cout << "Instrukcija HALT sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }
    this->insName = "halt";
    this->insSize = 1;
    break;
  }
  case INT:{

    if (modifikator != 0) {
      this->greska_instrukcija = true;
      cout << "Instrukcija INT sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }

    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    unsigned char sourceReg = (this->regsDescr << 4) >> 4;

    if (sourceReg != 0xF){
      this->greska_instrukcija = true;
      cout << "Instrukcija INT sa ovim izvornim registrom ne postoji" << endl;
      ret = -1;
    }

    this->insName = "int";
    this->insSize = 2;
    break;
  }
  case IRET:{

    if (modifikator != 0) {
      this->greska_instrukcija = true;
      cout << "Instrukcija IRET sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }
    this->insName = "iret";
    this->insSize = 1;
    break;
  }
  case CALL:{

    if (modifikator != 0) {
      this->greska_instrukcija = true;
      cout << "Instrukcija CALL sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }

    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    unsigned char destReg = this->regsDescr >> 4;

    if (destReg != 0xF){
      this->greska_instrukcija = true;
      cout << "Instrukcija CALL sa ovim destinacionim registrom ne postoji" << endl;
      ret = -1;
    }
    
    this->addrMode = procitajMemoriju(*pc, BYTE);
    (*pc)++;

    this->insName = "call";
    this->insSize = 3;
    break;
  }
  case RET:{

    if (modifikator != 0) {
      this->greska_instrukcija = true;
      cout << "Instrukcija RET sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }
    this->insName = "ret";
    this->insSize = 1;
    break;
  }
  case JUMP:{
    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    
    unsigned char destReg = this->regsDescr >> 4;
    

    if (destReg != 0xF){
      this->greska_instrukcija = true;
      cout << "Instrukcija skoka sa ovim destinacionim registrom ne postoji" << endl;
      ret = -1;
    }
    
    this->addrMode = procitajMemoriju(*pc, BYTE);
    (*pc)++;

    switch (modifikator)
    {
    case JMP:
      this->insName = "jmp";
      break;
    case JEQ: 
      this->insName = "jeq";
      break;
    case JNE:
      this->insName = "jne";
      break;
    case JGT:
      this->insName = "jgt";
    
    default:
      this->greska_instrukcija = true;
      cout << "Instrukcija skoka sa modifikatorom ne postoji" << endl;
      ret = -1;
      break;
    } 
    this->insSize = 3;
    break;
  }
  case XCHG:{

    if (modifikator != 0) {
      this->greska_instrukcija = true;
      cout << "Instrukcija XCHG sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }
    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    this->insName = "xchg";
    this->insSize = 2;
    break;
  }
  case ARIT:{
    switch (modifikator)
    {
    case ADD:
      this->insName = "add";
      break;
    case SUB:
      this->insName = "sub";
      break;
    case MUL:
      this->insName = "mul";
      break;
    case DIV:
      this->insName = "div";
      break;
    case CMP:
      this->insName = "cmp";
      break;
    default:
      this->greska_instrukcija = true;
      cout << "Aritmeticka instrukcija sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
      break;
    }
    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    this->insSize = 2;
    break;
  }
  case LOG:{
    switch (modifikator)
    {
    case NOT:
      this->insName = "not";
      break;
    case AND:
      this->insName = "and";
      break;
    case OR:
      this->insName = "or";
      break;
    case XOR:
      this->insName = "xor";
      break;
    case TEST:
      this->insName = "test";
      break;  
    default:
      this->greska_instrukcija = true;
      cout << "Logicka instrukcija sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
      break;
    }
    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    this->insSize = 2;
    break;
  }
  case SHIFT:{
    switch (modifikator)
    {
    case SHL:
      this->insName= "shl";
      break;
    case SHR:
      this->insName= "shr";
      break;
    
    default:
      this->greska_instrukcija = true;
      cout << "Pomeracka instrukcija sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
      break;
    }
    break;
  }
  case LDR:{
    if (modifikator != 0) {
      this->greska_instrukcija = true;
      cout << "Instrukcija LDR sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }

    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    this->addrMode = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    this->insName = "ldr";
    this->insSize = 3;
    break;
  }
  case STR:{
    if (modifikator != 0) {
      this->greska_instrukcija = true;
      cout << "Instrukcija LDR sa ovim modifikatorom ne postoji" << endl;
      ret = -1;
    }

    this->regsDescr = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    this->addrMode = procitajMemoriju(*pc, BYTE);
    (*pc)++;
    this->insName = "str";
    this->insSize = 3;
    break;
  }
  default:
    this->greska_instrukcija = true;
    cout << "Ne postoji instruckija sa datim operacionim kodom" << endl;
    cout << hex << this->instrDescr - 0 << endl;
    break;
  }
  
  if (this->insSize == 3){
    unsigned char nacin_adr = this->addrMode & 0xf;
    if (nacin_adr == neposredno || nacin_adr == reg_dir_pom ||
     nacin_adr == reg_ind_pom || nacin_adr == mem){
      this->insSize = 5;
      this->dataLow = procitajMemoriju(*pc, BYTE);
      (*pc)++;
      this->dataHigh = procitajMemoriju(*pc, BYTE);
      (*pc)++;
    }
  }
  //cout << this->insName << hex << *pc << endl;
  return ret;
}

int Emulator::execute(){
  
  unsigned char opCode = this->instrDescr >> 4;
  unsigned char modifikator = this->instrDescr & 0xf;

  unsigned int sReg = this->regsDescr & 0xf;
  unsigned int dReg = this->regsDescr >> 4;

  switch (opCode)
  {
  case HALT:{
    this->running = false;
    break;
  }
  case INT:{
    (*sp)-=2;
    upisiuMemoriju(*sp, WORD, *pc);
    (*sp)-=2;
    upisiuMemoriju(*sp, WORD, *psw);

    //unsigned int destReg = this->regsDescr >> 4;
    *pc = procitajMemoriju((regs[dReg]%8)*2, WORD);
    (*psw) |= (I_FLAG | Tl_FLAG | Tr_FLAG);
    break;
  }  
  case IRET:{
    *psw = procitajMemoriju(*sp,WORD);
    (*sp)+=2;
    *pc = procitajMemoriju(*sp, WORD);
    (*sp)+=2;
    break;
  }
  case CALL:{
    (*sp)-=2;
    upisiuMemoriju(*sp, WORD, *pc);
    skociNaAdresu();
    break;
  }
  case RET:{
    *pc = procitajMemoriju(*sp, WORD);
   /* cout << "SP " << hex << *sp << endl;
    cout << "PC " << hex << *pc << endl;*/
    (*sp)+=2;
    break;
  }
  case JUMP:{    

    switch (modifikator)
    {
    case JMP:
      skociNaAdresu();
      break;
    case JEQ:
      if (*psw & Z_FLAG){
        skociNaAdresu();
      }
      break;
    case JNE:
      if (!(*psw & Z_FLAG)){
        skociNaAdresu();
      } 
      break;
    case JGT:
      if (!(*psw & Z_FLAG) && 
      (((*psw & N_FLAG) > 0 && (*psw & O_FLAG) > 0) ||
      ((*psw & N_FLAG) == 0 && (*psw & O_FLAG) == 0))){
        skociNaAdresu();
      }
      break;
    
    default:
      break;
    }
    break;
  }
  case XCHG:{
    
    unsigned short temp = regs[dReg];
    regs[dReg] = regs[sReg];
    regs[sReg] = temp;
    break; 
  }
  case ARIT:{
    
    switch (modifikator)
    {
    case ADD:
      regs[dReg] = regs[dReg] + regs[sReg];
      //cout << regs[dReg] << endl;
      break;
    case SUB:
      regs[dReg] -= regs[sReg];
      //cout << regs[dReg] << endl;
      break;
    case MUL:
      regs[dReg] *= regs[sReg];
      break;
    case DIV:
      regs[dReg] /= regs[sReg];
      break;
    case CMP:{
      short temp = ((signed short)regs[dReg] - (signed short)regs[sReg]);
      if (temp == 0){
        *psw |= Z_FLAG;
      }else{
        *psw &= ~Z_FLAG;
      }

      short tempD = (signed short)regs[dReg];
      short tempS = (signed short)regs[sReg];

      if ((tempD > 0 && tempS < 0 && (tempD-tempS)<0 ) || 
      (tempD < 0 && tempS > 0 && (tempD - tempS)>0)){
        *psw |= O_FLAG;
      }else{
        *psw &= ~O_FLAG;
      }

      if (regs[dReg] < regs[sReg]){
        *psw |= C_FLAG;
      }else{
        *psw &= ~C_FLAG;
      }

      if (temp < 0){
        *psw |= N_FLAG;
      }else{
        *psw &= ~N_FLAG;
      }        
      break;
    }
    default:
      break;
    }
    break;
  }
  case LOG:{
    switch (modifikator)
    {
    case NOT:
      regs[dReg] = ~regs[dReg];
      break;
    case AND:
      regs[dReg] &= regs[sReg];
      break;
    case OR:
      regs[dReg] |= regs[sReg];
      break;
    case XOR:
      regs[dReg] ^= regs[sReg];
      break;
    case TEST:{
      short temp = regs[dReg] ^ regs[sReg];
      if (temp == 0){
        *psw |= Z_FLAG;
      }else{
        *psw &= ~Z_FLAG;
      }

      if (temp < 0){
        *psw |= N_FLAG;
      }else{
        *psw &= ~N_FLAG;
      }        
      break;
    }
    default:
      break;
    }
    break;
  }
  case SHIFT:{
    short tempD = (signed short)regs[dReg];
    short tempS = (signed short)regs[sReg];

    switch (modifikator)
    {
    case SHL:{
      regs[dReg] <<= regs[sReg];
      signed short temp = regs[dReg];

      if (temp == 0){
        *psw |= Z_FLAG;
      }else{
        *psw &= ~Z_FLAG;
      }
      
      if (temp < 0){
        *psw |= N_FLAG;
      }else{
        *psw &= ~N_FLAG;
      }  

      if (tempS < 16 && ((tempD >> (16 - tempD)) & 1)){
        *psw |= C_FLAG;
      } else {
        *psw &= ~C_FLAG;
      }
      break;
    }
    case SHR:{
      regs[dReg] >>= regs[sReg];
      signed short temp = regs[dReg];

      if (temp == 0){
        *psw |= Z_FLAG;
      }else{
        *psw &= ~Z_FLAG;
      }
      
      if (temp < 0){
        *psw |= N_FLAG;
      }else{
        *psw &= ~N_FLAG;
      }  

      if ((tempD >> (tempS - 1))&1){
        *psw |= C_FLAG;
      } else {
        *psw &= ~N_FLAG;
      }
      break;
    }
    default:
      break;
    }
    break;
  }
  case LDR:{

  unsigned short payload;
  if (this->insSize == 5){
    payload = (this->dataHigh<< 8)| this->dataLow; 
  }
  
  
  unsigned char nacin_adr = this->addrMode & 0xf;

  switch (nacin_adr){
    case neposredno:
      regs[dReg] = payload;
      break;
    case reg_dir:
      regs[dReg] = regs[sReg];
      break;
    case reg_dir_pom:
      regs[dReg] = regs[sReg] + payload;
      break;
    case reg_ind:
      regs[dReg] = procitajMemoriju(regs[sReg], WORD);
      break;
    case reg_ind_pom:
      regs[dReg] = procitajMemoriju(regs[sReg] + payload, WORD);
      break;
    case mem:
      regs[dReg] = procitajMemoriju(payload, WORD);
      break;
    default:
      break;
  }

  unsigned char update = this->addrMode >> 4;

  if (update == 4){
    regs[sReg]+=2;
    //cout << "POPEEEEEEE" << endl;
  }
  break;
  }
  case STR:{
    unsigned short payload;
    if (this->insSize == 5){
      payload = (this->dataHigh<< 8) | this->dataLow; 
    }

    unsigned char update = this->addrMode >> 4;

    if (update == 1){
      regs[sReg]-=2;
    }

    unsigned char nacin_adr = this->addrMode & 0xf;

    switch (nacin_adr)
    {
    case reg_dir:
      regs[sReg] = regs[dReg];
      break;
    case reg_ind:
      upisiuMemoriju(regs[sReg], WORD, regs[dReg]);
      break;
    case reg_ind_pom:
      upisiuMemoriju(regs[sReg]+payload, WORD, regs[dReg]);
      break;
    case mem:
      upisiuMemoriju(payload, WORD, regs[dReg]);
      break;    
    default:
      break;
    }
    break;
  }
  
  default:
    break;
  }
  return 0;
}

int Emulator::obradiPrekide(){

  if (this->greska_instrukcija){
    (*sp)--;
    upisiuMemoriju(*sp, WORD, *pc);
    (*sp)--;
    upisiuMemoriju(*sp, WORD, *psw);
    (*psw) |= (I_FLAG | Tl_FLAG | Tr_FLAG);
    *pc = procitajMemoriju(2, WORD);
    this->greska_instrukcija = false;
  }
  return 0;
}

int Emulator::emuliraj(){
  this->running = true;

  while(this->running){

    int ret = fetch();
    if (ret < 0) {
      return ret;
    }

    ret = execute();

    obradiPrekide();
  }
  cout << "------------------------------------------------" << endl;
  cout << "Emulated processor executed halt instruction" << endl;
  cout << "Emulated processor state: psw=0b" << std::bitset<16>(regs[8]) << endl;
  
  cout << "r0=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[0] << " " ;
  cout << "r1=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[1] << " " ;
  cout << "r2=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[2] << " " ;
  cout << "r3=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[3] << " " ;
  cout << endl;
  cout << "r4=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[4] << " " ;
  cout << "r5=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[5] << " " ;
  cout << "r6=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[6] << " " ;
  cout << "r7=0x" << std::setfill('0') << std::setw(4) << hex <<  regs[7] << " " ;

  //cout << this->insName << this->insSize << "|" << *pc << endl;
  return 0;
}

int main(int argc, char* argv[]){

  if (argc < 2){
    cout << "Neodgovarajuc broj argumenata" << endl;
    return -1;
  }
  
  string input = argv[1];
  Emulator e;
  int ret = e.initMemorija(input);
  if (ret > 0) return ret;

  ret = e.initPcPswSp();
  if (ret > 0) return ret;
  
  ret = e.emuliraj();
  if (ret > 0) return ret;

  

}