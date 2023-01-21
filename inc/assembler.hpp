#pragma once

#include <string>
#include <list>
#include <map>
#include <regex>

using namespace std;

struct ulazTabelaSimbola{
  string ime;
  int simbolId; 
  int offset; 
  int sekcijaId;
  string sekcija;
  char tip;
  string vrsta;
  bool definisan;
};

struct ulazTabelaSekcija{
  string ime;
  int sectionId;
  int duzinaSekcije;
  vector<char> zapis;
};

struct ulazRelokacionaTabela{
  string sekcija;
  int offset;
  string tip;
  string simbol;
  int addend;
};

class Asembler{
  public:
  void asembliraj(const char* input, const char* output);

  private:
  int locationCounter;
  int sectionId;
  string section;
  static int symbolId;

  list<string> linijeAsm;
  map<string, ulazTabelaSimbola> tabelaSimbola;
  map<string, ulazTabelaSekcija> tabelaSekcija;
  multimap<string, ulazRelokacionaTabela> relokacionaTabela;

  void parsirajText();
  list<string> parsirajString(string str, string delimiter);

  int obradiLabelu(string labela);
  int obradiGlobal(string lista_simbola);
  int obradiExtern(string lista_simbola);
  int obradiSekciju(string ime_sekcije);
  int obradiWord(string argument);
  int obradiSkip(string bajtovi);
  int obradiHalt();
  int obradiInt(string registar);
  int obradiIret();
  int obradiCall(string operand);
  int obradiOperand(bool data, string operand, string reg);
  int obradiRet();
  int obradiJmp(string operand);
  int obradiJeq(string operand);
  int obradiJne(string operand);
  int obradiJgt(string operand);
  int obradiPush(string registar);
  int obradiPop(string registar);
  int obradiXchg(string reg1, string reg2);
  int obradiAdd(string reg1, string reg2);
  int obradiSub(string reg1, string reg2);
  int obradiMul(string reg1, string reg2);
  int obradiDiv(string reg1, string reg2);
  int obradiCmp(string reg1, string reg2);
  int obradiNot(string reg1);
  int obradiAnd(string reg1, string reg2);
  int obradiOr(string reg1, string reg2);
  int obradiXor(string reg1, string reg2);
  int obradiTest(string reg1, string reg2);
  int obradiShl(string reg1, string reg2);
  int obradiShr(string reg1, string reg2);
  int obradiLdr(string reg, string operand);
  int obradiStr(string reg, string operand);
  int obradiAscii(string str);
  
  bool simbolDefinisan(string simbol);
  void razresiRelZapis(string simbol, char tip);
  void dodajAddend();

  void makeBinarniFajl(string output);


  //regularni izrazi
  regex komentar = regex("^([^#]*)#.*");
  regex tabovi = regex("\\t");
  regex spaces = regex(" {2,}");
  regex zarez_space = regex(" ?, ?");
  regex labela_space = regex(" ?: ?");

  regex labela = regex("^([a-zA-Z][_A-Za-z0-9]*):$");
  regex labela_text = regex("^([a-zA-Z][_A-Za-z0-9]*):(.*)$");
  regex global = regex("^\\.global ([a-zA-Z][_A-Za-z0-9]*(,[a-zA-Z][_A-Za-z0-9]*)*)$");
  regex reg_extern = regex("^\\.extern ([a-zA-Z][_A-Za-z0-9]*(,[a-zA-Z][_A-Za-z0-9]*)*)$");
  regex sekcija = regex("^\\.section ([a-zA-Z][_A-Za-z0-9]*)$");
  regex word = regex("^\\.word (([a-zA-Z][_A-Za-z0-9]*|-?[0-9]+|0x[0-9A-F]+)(,[a-zA-Z][_A-Za-z0-9]*|-?[0-9]+|0x[0-9A-F]+)*)$");
  regex literal_hex = regex("0x[0-9A-F]+");
  regex literal_dec = regex("[0-9]+");
  regex reg_simbol = regex("^([a-zA-Z][_A-Za-z0-9]*)$");
  regex skip = regex("^\\.skip ([0-9]+|^([a-zA-Z][_A-Za-z0-9]*)$)$");
  regex end = regex("^\\.end$");
  
  regex halt = regex ("^(halt)$");
  regex instrukcija_int = regex ("^int (r[0-7]|psw)$");
  regex iret = regex ("^(iret)$");
  regex call = regex ("^(call) (.*)$");
  regex ret = regex ("^(ret)$");
  regex jmp = regex("^(jmp) (.*)");
  regex jeq = regex("^(jeq) (.*)");
  regex jne = regex("^(jne) (.*)");
  regex jgt = regex("^(jgt) (.*)");
  regex push = regex ("^push (r[0-7]|psw)$");
  regex pop = regex ("^pop (r[0-7]|psw)$");
  regex xchg = regex ("^xchg (r[0-7]|psw),(r[0-7]|psw)");
  regex add = regex("^add (r[0-7]|psw),(r[0-7]|psw)");
  regex sub = regex("^sub (r[0-7]|psw),(r[0-7]|psw)");
  regex mul = regex("^mul (r[0-7]|psw),(r[0-7]|psw)");
  regex div = regex("^div (r[0-7]|psw),(r[0-7]|psw)");
  regex cmp = regex("^cmp (r[0-7]|psw),(r[0-7]|psw)");
  regex reg_not = regex("^not (r[0-7]|psw)");
  regex reg_and = regex("^and (r[0-7]|psw),(r[0-7]|psw)");
  regex reg_or = regex("^or (r[0-7]|psw),(r[0-7]|psw)");
  regex reg_xor = regex("^xor (r[0-7]|psw),(r[0-7]|psw)");
  regex reg_test = regex("^test (r[0-7]|psw),(r[0-7]|psw)");
  regex shl = regex("^shl (r[0-7]|psw),(r[0-7]|psw)");
  regex shr = regex("^shr (r[0-7]|psw),(r[0-7]|psw)");
  regex ldr = regex("^(ldr) (.*)");
  regex str = regex("^(str) (.*)");
  regex ascii = regex("^(.ascii) (.*)");

  regex jump_operand_neposredno = regex("^(-?[0-9]+|0x[0-9A-F]+|[a-zA-Z][_A-Za-z0-9]*)$");
  regex jump_operand_pc_relativno = regex("^%([a-zA-Z][_A-Za-z0-9]*)$");
  regex jump_operand_memorijsko = regex("^\\*(-?[0-9]+|0x[0-9A-F]+|[a-zA-Z][_A-Za-z0-9]*)$");
  regex jump_operand_registarsko = regex("^\\*(r[0-7]|psw)$");
  regex jump_operand_reg_ind = regex("^\\*\\[(r[0-7]|psw)\\]$");
  regex jump_operand_reg_pom = regex("^\\*\\[(r[0-7]|psw) \\+ (-?[0-9]+|0x[0-9A-F]+|[a-zA-Z][_A-Za-z0-9]*)\\]$");
  
  regex data_operand_neposredno = regex("^\\$(-?[0-9]+|0x[0-9A-F]+|[a-zA-Z][_A-Za-z0-9]*)$");
  regex data_operand_memorijsko = regex("^(-?[0-9]+|0x[0-9A-F]+|[a-zA-Z][_A-Za-z0-9]*)$");
  regex data_operand_pc_relativno = regex("^%([a-zA-Z][_A-Za-z0-9]*)$");
  regex data_operand_registarsko = regex("^(r[0-7]|psw)$");
  regex data_operand_reg_ind = regex("^\\[(r[0-7]|psw)\\]$");
  regex data_operand_reg_pom = regex("^\\[(r[0-7]|psw) \\+ (-?[0-9]+|0x[0-9A-F]+|[a-zA-Z][_A-Za-z0-9]*)\\]$");


  regex boundary_space = regex("^( *)([^ ].*[^ ])( *)$");
};