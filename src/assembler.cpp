#include <stdio.h>
#include <iostream>
#include <fstream>
#include "../inc/assembler.hpp"

using namespace std;

int Asembler::symbolId = 0;

list<string> Asembler::parsirajString(string str, string delimiter){
  size_t pos = 0;
  std::string token;

  list<string> lista;
  while ((pos = str.find(delimiter)) != std::string::npos) {
    token = str.substr(0, pos);
    str.erase(0, pos + delimiter.length());
    lista.push_back(token);
  }
  lista.push_back(str);
  return lista;
}

bool Asembler::simbolDefinisan(string simbol){
  if (tabelaSimbola.count(simbol) == 1) return true;
  else return false;
}

void Asembler::razresiRelZapis(string simbol, char tip){

  list<ulazRelokacionaTabela> lista;

  for (auto it=relokacionaTabela.begin(); it != relokacionaTabela.end(); it++){
    if (it->first == simbol){

      if(it->second.simbol == "nedefinisan"){
        
        switch (tip)
        {
        case 'l':
          //it->second.offset = locationCounter;
          it->second.simbol = section;
          break;
        case 'g':
          //it->second.offset = locationCounter;
          it->second.simbol = simbol;
        
        default:
          break;
        }

      } else{
        cout << "Greska: Greska pri razresavanju simbola koji je razresen";
      }
    }
  } 
  
}


int Asembler::obradiLabelu(string labela){
  if (simbolDefinisan(labela)){

    ulazTabelaSimbola& ulaz = tabelaSimbola.at(labela);
    
    if (ulaz.tip == 'e') {
      cout << "Greska: Simbol je definisan kao ulazni";
      return -1;
    }

    ulaz.definisan = true;
    ulaz.offset = this->locationCounter;
    ulaz.sekcija = this->section;
    ulaz.sekcijaId = this->sectionId;

    razresiRelZapis(labela, ulaz.tip);
  } else {
    if (this->section == "UND"){
      cout<< "Greska: nije definisana sekcija\n";
      return -1;
    }
    int id = ++Asembler::symbolId;
    ulazTabelaSimbola ulaz = {
      labela,
      id,
      locationCounter,
      sectionId,
      section,
      'l',
      "NOTYP",
      true
    };

    tabelaSimbola.insert({labela, ulaz});
    
    razresiRelZapis(labela,'l');

  }
  return 0;
}

int Asembler::obradiGlobal(string lista_simbola){

    list<string> lista = parsirajString(lista_simbola, ",");

    for (string s: lista){
        if (simbolDefinisan(s)){
          ulazTabelaSimbola &ulaz = tabelaSimbola.at(s);
          if (ulaz.definisan){
            ulaz.tip = 'g';
          }else {
            cout << "Greska: Simbol se nalazi u tabeli ali nije definisan";
            return -1;
          }
        } else {
          int id = ++Asembler::symbolId;
          ulazTabelaSimbola ulaz =
           {  s, 
              id,
              locationCounter,
              sectionId,
              section,
              'g',
              "NOTYP",
              false};
          tabelaSimbola.insert({s, ulaz});
        }
    }

    return 0;
    
}

int Asembler::obradiExtern(string lista_simbola){
   list<string> lista = parsirajString(lista_simbola, ",");
  
  for (string s: lista){
    if (simbolDefinisan(s)){
      ulazTabelaSimbola &ulaz = tabelaSimbola.at(s);
      if (ulaz.tip != 'e'){
        cout << "Greska: Simbol ne sme biti definisan.";
        return -1;
      }
      return 0;
    }else{
      int id = ++Asembler::symbolId;
      ulazTabelaSimbola ulaz = {
        s,
        id,
        locationCounter,
        0,
        "UND",
        'e',
        "NOTYP",
        false
      };
      tabelaSimbola.insert({s, ulaz});
    }
  } 
  return 0; 
}

int Asembler::obradiSekciju(string ime_sekcije){

  if (simbolDefinisan(ime_sekcije)){
    cout << "Greska: Sekcija je prethodno definisana\n";
    return -1;
  }

  ulazTabelaSekcija& trenutniUlaz = tabelaSekcija.at(section);
  trenutniUlaz.duzinaSekcije = locationCounter;
  locationCounter = 0;
  sectionId++;
  section = ime_sekcije;

  vector<char> zapis;
  // zapis.push_back("#." + ime_sekcije);
  ulazTabelaSekcija ulazSek = {
    ime_sekcije,
    sectionId,
    locationCounter,
    zapis
  };
  tabelaSekcija.insert({ime_sekcije, ulazSek});
  
  int id=++Asembler::symbolId;
  ulazTabelaSimbola ulaz = {
    ime_sekcije,
    id,
    locationCounter,
    sectionId,
    section,
    'l',
    "SCTN",
    true
  };
  tabelaSimbola.insert({ime_sekcije, ulaz});
  return 0;
}

int Asembler::obradiWord(string argument){

    if (section == "UND"){
      cout << "Greska: direktiva .word je van sekcije\n";
      return -1;
    }

    list<string> lista = parsirajString(argument, ",");

    ulazTabelaSekcija& ulaz = tabelaSekcija.at(section);

    smatch sm;
    for (string s: lista){
      if (regex_match(s, sm, literal_dec)){
        int v = stoi(s);

        int v1 = (v & 0xff);
        char c1 = 0 + v1;

        int v2 = ((v & 0xff00)>> 8);
        char c2 = 0 + v2;
        
        ulaz.zapis.push_back(c1);
        ulaz.zapis.push_back(c2);
        
      } else if (regex_match(s, sm, literal_hex)){
        int v = stoi(s, nullptr, 16);
        
        int v1 = (v & 0xff);
        char c1 = 0 + v1;

        int v2 = ((v & 0xff00)>> 8);
        char c2 = 0 + v2;
        
        ulaz.zapis.push_back(c1);
        ulaz.zapis.push_back(c2);
      } else if (regex_match(s, sm, reg_simbol)){
        
        if (simbolDefinisan(s)){
          ulazTabelaSimbola &ulazSim = tabelaSimbola.at(s);

          if (ulazSim.definisan){
            if (ulazSim.tip == 'l'){
              int v = ulazSim.offset;
              
              int v1 = (v & 0xff);
              char c1 = 0 + v1;

              int v2 = ((v & 0xff00)>> 8);
              char c2 = 0 + v2;
        
              ulaz.zapis.push_back(c1);
              ulaz.zapis.push_back(c2);

              ulazRelokacionaTabela ulazRT = {
                section,
                locationCounter,
                "TYPE_16",
                ulazSim.sekcija,
                0
              };

              relokacionaTabela.insert({s, ulazRT});
            } else if (ulazSim.tip == 'g'){

              ulaz.zapis.push_back(0x00);
              ulaz.zapis.push_back(0x00);

              ulazRelokacionaTabela ulazRT = {
                section,
                locationCounter,
                "TYPE_16",
                s,
                0
              };

              relokacionaTabela.insert({s, ulazRT});
            }
          } else {
            if (ulazSim.tip == 'e'){
              ulaz.zapis.push_back(0x00);
              ulaz.zapis.push_back(0x00);

              ulazRelokacionaTabela ulazRT = {
                section,
                locationCounter,
                "TYPE_16",
                s,
                0
              };

              relokacionaTabela.insert({s, ulazRT});
            } else {

              // ovo je ako se simbol nalazi ali nije definisan 
              ulaz.zapis.push_back(0x00);
              ulaz.zapis.push_back(0x00);
          
              ulazRelokacionaTabela ulazRT = {
              section,
              locationCounter,
              "TYPE_16",
              "nedefinisan",
              0
              };

              relokacionaTabela.insert({s, ulazRT});
            }
          }
        }else{

          // ovo je ako se uopste ne nalazi simbol u tabeli simbola
          ulaz.zapis.push_back(0x00);
          ulaz.zapis.push_back(0x00);
          
          ulazRelokacionaTabela ulazRT = {
              section,
              locationCounter,
              "TYPE_16",
              "nedefinisan",
              0
          };
          relokacionaTabela.insert({s, ulazRT});            
        }
      }
      locationCounter += 2;
    }

    return 0;
}

int Asembler::obradiSkip(string argument){

  smatch sm;
  int v;

  ulazTabelaSekcija& ulaz = tabelaSekcija.at(this->section);

  if (regex_match(argument, sm, literal_dec)){
        v = stoi(argument);
        
  } else if (regex_match(argument, sm, literal_hex)){
        v = stoi(argument, nullptr, 16);
  } 

  for (int i=0; i<v; i++){
    ulaz.zapis.push_back(0x00);
  }

  locationCounter += v;
  return 0;
}

int Asembler::obradiHalt(){

  if (this->section == "UND"){
    cout << "Greska: instrukcija HALT je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x00);
  locationCounter++;
  return 0;
}

int Asembler::obradiInt(string registar){

  int value = stoi(registar);

   if (this->section == "UND"){
    cout << "Greska: instrukcija INT je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x10);
  ulaz.zapis.push_back((char)((value << 4)|0xf));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiIret(){
  if (this->section == "UND"){
    cout << "Greska: instrukcija IRET je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x20);
  locationCounter++;
  return 0;
}

int Asembler::obradiCall(string operand){
  if (this->section == "UND"){
    cout << "Greska: instrukcija CALL je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x30);
  locationCounter++;

  int ret = obradiOperand(false, operand, "noreg");

  return ret;
}

int Asembler::obradiRet(){
  if (this->section == "UND"){
    cout << "Greska: instrukcija IRET je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x40);
  locationCounter++;
  return 0;
}

int Asembler::obradiJmp(string operand){
  if (this->section == "UND"){
    cout << "Greska: instrukcija JMP je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x50);
  locationCounter++;

  int ret = obradiOperand(false, operand, "noreg");
  return ret;
}

int Asembler::obradiJeq(string operand){
  if (this->section == "UND"){
    cout << "Greska: instrukcija JEQ je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x51);
  locationCounter++;

  int ret = obradiOperand(false, operand, "noreg");
  return ret;
}

int Asembler::obradiJne(string operand){
  if (this->section == "UND"){
    cout << "Greska: instrukcija JNE je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x52);
  locationCounter++;

  int ret = obradiOperand(false, operand, "noreg");
  return ret;
}

int Asembler::obradiJgt(string operand){
  if (this->section == "UND"){
    cout << "Greska: instrukcija JNE je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x53);
  locationCounter++;

  int ret = obradiOperand(false, operand, "noreg");
  return ret;
}

int Asembler::obradiPush(string registar){
  int value = stoi(registar);

   if (this->section == "UND"){
    cout << "Greska: instrukcija PUSH je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0xb0);
  ulaz.zapis.push_back((char)((value << 4)|0x6));
  ulaz.zapis.push_back(0x12);

  locationCounter += 3;
  return 0;
}

int Asembler::obradiPop(string registar){
  int value = stoi(registar);

   if (this->section == "UND"){
    cout << "Greska: instrukcija PUSH je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0xa0);
  ulaz.zapis.push_back((char)((value << 4)|0x6));
  ulaz.zapis.push_back(0x42);

  locationCounter += 3;
  return 0;
}

int Asembler::obradiXchg(string reg1, string reg2){
  
  if (this->section == "UND"){
    cout << "Greska: instrukcija XCHG je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x60);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiAdd(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija ADD je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x70);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiSub(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija SUB je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x71);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiMul(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija MUL je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x72);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiDiv(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija DIV je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x73);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiCmp(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija CMP je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x74);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiNot(string reg1){
  if (this->section == "UND"){
    cout << "Greska: instrukcija NOT je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  int r1 = stoi(reg1);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x80);
  ulaz.zapis.push_back((char)(r1<<4|0xf));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiAnd(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija AND je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x81);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiOr(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija OR je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x82);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiXor(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija XOR je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x83);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiTest(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija TEST je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x84);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiShl(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija SHL je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x90);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiShr(string reg1, string reg2){
  if (this->section == "UND"){
    cout << "Greska: instrukcija SHR je pozvana u nedefinisanoj sekciji!\n";
  }

  reg1 = reg1.substr(1, 1);
  reg2 = reg2.substr(1, 1);

  int r1 = stoi(reg1);
  int r2 = stoi(reg2);
 
  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  ulaz.zapis.push_back(0x91);
  ulaz.zapis.push_back((char)(r1<<4|r2));

  locationCounter += 2;
  return 0;
}

int Asembler::obradiLdr(string reg, string operand){
  if (this->section == "UND"){
    cout << "Greska: instrukcija LDR je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);
  reg = reg.substr(1, 1);

  ulaz.zapis.push_back(0xa0);
  locationCounter++;

  int ret = obradiOperand(true, operand, reg);
  return ret;
}

int Asembler::obradiStr(string reg, string operand){
  if (this->section == "UND"){
    cout << "Greska: instrukcija LDR je pozvana u nedefinisanoj sekciji!\n";
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);
  reg = reg.substr(1, 1);

  ulaz.zapis.push_back(0xb0);
  locationCounter++;

  int ret = obradiOperand(true, operand, reg);
  return ret;
}

int Asembler::obradiAscii(string str){
  int ret = 0;
  if (this->section == "UND"){
    cout << "Greska: instrukcija ASCII je pozvana u nedefinisanoj sekciji!\n";
    ret = -1;
  }

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);

  for (int i=0; i< str.size(); i++){
    ulaz.zapis.push_back(str[i]);
  }

  locationCounter+= str.size();
  return ret;
}

int Asembler::obradiOperand(bool data,string operand, string reg){

  ulazTabelaSekcija &ulaz = tabelaSekcija.at(this->section);
  smatch sm;
  //cout << data << " " << operand << " " << reg << "\n";
  if (!data){
    if (regex_match(operand, sm, jump_operand_neposredno)){

      ulaz.zapis.push_back(0xff);
      ulaz.zapis.push_back(0x00);
      if (regex_match(operand, sm, literal_dec)){

        int v = stoi(operand);
        ulaz.zapis.push_back((char)(v>>8));
        ulaz.zapis.push_back((char)v);

      } else if (regex_match(operand, sm, literal_hex)){
      
        int v = stoi(operand, nullptr, 16);
        ulaz.zapis.push_back((char)(v>>8));
        ulaz.zapis.push_back((char)v);

      } else if (regex_match(operand, sm, reg_simbol)){
      
        ulaz.zapis.push_back(0x00);
        ulaz.zapis.push_back(0x00);

        ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();
        
        map<string, ulazTabelaSimbola>::iterator itrr;
        for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
            if (itrr->first == operand){
              *ulazTS = itrr->second;
            }
        }

        if (ulazTS->simbolId == 0){
          int id = ++Asembler::symbolId;
          
          ulazTabelaSimbola noviUlaz = {
            operand,
            id,
            0,
            0,
            "UND",
            'l',
            "NOTYP",
            false
          };      

          tabelaSimbola.insert({operand, noviUlaz});
        }
       
        ulazRelokacionaTabela ulazRT;

        ulazRT.addend = 0;
        ulazRT.offset = locationCounter + 5 - 2 - 1;
        ulazRT.sekcija = this->section;
        ulazRT.simbol = "nedefinisan";
        ulazRT.tip = "TYPE_16";

        relokacionaTabela.insert({operand, ulazRT});
      }
      locationCounter += 4;
    } else if (regex_match(operand, sm, jump_operand_pc_relativno)){
      
        ulaz.zapis.push_back(0xf7); // 7 je jer je pc reg7
        ulaz.zapis.push_back(0x05);

        ulaz.zapis.push_back(0x00);
        ulaz.zapis.push_back(0x00);

        ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();

        operand = operand.substr(1,operand.size() - 1);
        
        map<string, ulazTabelaSimbola>::iterator itrr;
        for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
            if (itrr->first == operand){
              *ulazTS = itrr->second;
            }
        }

        if (ulazTS->simbolId == 0){
          int id = ++Asembler::symbolId;
          
          ulazTabelaSimbola noviUlaz = {
            operand,
            id,
            0,
            0,
            "UND",
            'l',
            "NOTYP",
            false
          };      

          tabelaSimbola.insert({operand, noviUlaz});
        }
        
        ulazRelokacionaTabela ulazRT;

        ulazRT.addend = 0;
        ulazRT.offset = locationCounter + 5 - 2 -1;
        ulazRT.sekcija = this->section;
        ulazRT.simbol = "nedefinisan";
        ulazRT.tip = "TYPE_PC_16";

        relokacionaTabela.insert({operand, ulazRT});
        locationCounter += 4;
    } else if (regex_match(operand, sm, jump_operand_registarsko)){
      operand = operand.substr(2, operand.size() -1 );
      int reg = stoi(operand);

      ulaz.zapis.push_back(0xf0|reg);
      ulaz.zapis.push_back(0x01);

      locationCounter += 2;

    }else if(regex_match(operand, sm, jump_operand_reg_ind)){
      operand = operand.substr(3, 1);
      //cout << operand;

      int v = stoi(operand);

      ulaz.zapis.push_back(0xf0|v);
      ulaz.zapis.push_back(0x02);

      locationCounter += 2;
    } else if(regex_match(operand, sm, jump_operand_reg_pom)){
      
      string registar = operand.substr(3,1);
      int r = stoi(registar);

      ulaz.zapis.push_back(0xf0|r);
      ulaz.zapis.push_back(0x03);


      string opr = operand.substr(7, operand.size() - 8);
      
      if(regex_match(opr, sm, literal_dec)){
        int v = stoi(opr);

        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
      } else if(regex_match(opr, sm, literal_hex)) {

        int v = stoi(opr, nullptr, 16);
        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
        
      } else if (regex_match(opr, sm, reg_simbol)){

        ulaz.zapis.push_back(0x00);
        ulaz.zapis.push_back(0x00);

        ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();
      
        map<string, ulazTabelaSimbola>::iterator itrr;
        for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
          if (itrr->first == opr){
            *ulazTS = itrr->second;
          }
        }
        
        if (ulazTS->simbolId == 0){
          int id = ++Asembler::symbolId;
          
          ulazTabelaSimbola noviUlaz = {
            opr,
            id,
            0,
            0,
            "UND",
            'l',
            "NOTYP",
            false
          };      

          tabelaSimbola.insert({operand, noviUlaz});
        }
      
        ulazRelokacionaTabela ulazRT;

        ulazRT.addend = 0;
        ulazRT.offset = locationCounter + 5 - 2 -1;
        ulazRT.sekcija = this->section;
        ulazRT.simbol = "nedefinisan";
        ulazRT.tip = "TYPE_16";

        relokacionaTabela.insert({opr, ulazRT});
             
      }
      locationCounter +=4;  
    }else if (regex_match(operand,sm, jump_operand_memorijsko)){
      ulaz.zapis.push_back(0xff);
      ulaz.zapis.push_back(0x04);

      operand = operand.substr(1,operand.size() - 1);

      if(regex_match(operand, sm, literal_dec)){
        int v = stoi(operand);

        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
      } else if(regex_match(operand, sm, literal_hex)) {

        int v = stoi(operand, nullptr, 16);
        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
        
      } else if (regex_match(operand, sm, reg_simbol)){
        
        ulaz.zapis.push_back(0x00);
        ulaz.zapis.push_back(0x00);

        ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();
      
        map<string, ulazTabelaSimbola>::iterator itrr;
        for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
          if (itrr->first == operand){
            *ulazTS = itrr->second;
          }
        }

        if (ulazTS->simbolId == 0){
          int id = ++Asembler::symbolId;
          
          ulazTabelaSimbola noviUlaz = {
            operand,
            id,
            0,
            0,
            "UND",
            'l',
            "NOTYP",
            false
          };      

          tabelaSimbola.insert({operand, noviUlaz});
        }
      
        ulazRelokacionaTabela ulazRT;

        ulazRT.addend = 0;
        ulazRT.offset = locationCounter + 5 - 2 - 1;
        ulazRT.sekcija = this->section;
        ulazRT.simbol = "nedefinisan";
        ulazRT.tip = "TYPE_16";

        relokacionaTabela.insert({operand, ulazRT});    
      }
      locationCounter += 4;  
    }  else {
      cout << "Greska: Operand nije sintaksno dobro napisan";
      return -1;
    }
  } else {
    if (regex_match(operand, sm, data_operand_neposredno)){
      int r = stoi(reg);

      ulaz.zapis.push_back((r<<4)|0xf); // regsdescr
      ulaz.zapis.push_back(0x00); // addrmode

      operand = operand.substr(1, operand.size() -1);
      if (regex_match(operand, sm, literal_dec)){

        int v = stoi(operand);
        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
        

      } else if (regex_match(operand, sm, literal_hex)){
      
        int v = stoi(operand, nullptr, 16);
        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
        

      } else if (regex_match(operand, sm, reg_simbol)){
      
        ulaz.zapis.push_back(0x00);
        ulaz.zapis.push_back(0x00);

        ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();
        
        map<string, ulazTabelaSimbola>::iterator itrr;
        for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
            if (itrr->first == operand){
              *ulazTS = itrr->second;
            }
        }

        if (ulazTS->simbolId == 0){
          int id = ++Asembler::symbolId;
          
          ulazTabelaSimbola noviUlaz = {
            operand,
            id,
            0,
            0,
            "UND",
            'l',
            "NOTYP",
            false
          };      

          tabelaSimbola.insert({operand, noviUlaz});
        }
       
        ulazRelokacionaTabela ulazRT;

        ulazRT.addend = 0;
        ulazRT.offset = locationCounter + 5 - 2 - 1;
        ulazRT.sekcija = this->section;
        ulazRT.simbol = "nedefinisan";
        ulazRT.tip = "TYPE_16";

        relokacionaTabela.insert({operand, ulazRT});
      }
      locationCounter += 4;
    } else if (regex_match(operand, sm, data_operand_registarsko)){
      
      // r1 - reg
      // r2 - operand
      int r1;
      if (reg[0] == 'p'){
        r1 = 8;
      } else {
        r1 = stoi(reg);
      }
      
      int r2;
      if (operand[0] == 'p'){
        r2 = 8;
      } else {
        operand = operand.substr(1, 1);
        r2 = stoi(operand);
      }

      int rd = stoi(reg);
     
      ulaz.zapis.push_back((rd<<4)|r1); //regsdescr
      ulaz.zapis.push_back(0x01); //addrmode

      locationCounter += 2;
    } else if (regex_match(operand, sm, data_operand_memorijsko)){

      int r = stoi(reg);

      ulaz.zapis.push_back((r<<4)|0xf); // regdescr
      ulaz.zapis.push_back(0x04); // addrmode


      if(regex_match(operand, sm, literal_dec)){
        int v = stoi(operand);

        ulaz.zapis.push_back((char)(v>>8));
        ulaz.zapis.push_back((char)v);
      } else if(regex_match(operand, sm, literal_hex)) {

        int v = stoi(operand, nullptr, 16);

        ulaz.zapis.push_back((char)(v>>8));
        ulaz.zapis.push_back((char)v);
      } else if (regex_match(operand, sm, reg_simbol)){
        
        ulaz.zapis.push_back(0x00);
        ulaz.zapis.push_back(0x00);

        ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();
      
        map<string, ulazTabelaSimbola>::iterator itrr;
        for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
          if (itrr->first == operand){
            *ulazTS = itrr->second;
          }
        }

        if (ulazTS->simbolId == 0){
          int id = ++Asembler::symbolId;
          
          ulazTabelaSimbola noviUlaz = {
            operand,
            id,
            0,
            0,
            "UND",
            'l',
            "NOTYP",
            false
          };      

          tabelaSimbola.insert({operand, noviUlaz});
        }
      
        ulazRelokacionaTabela ulazRT;

        ulazRT.addend = 0;
        ulazRT.offset = locationCounter + 5 - 2 - 1;
        ulazRT.sekcija = this->section;
        ulazRT.simbol = "nedefinisan";
        ulazRT.tip = "TYPE_16";

        relokacionaTabela.insert({operand, ulazRT});    
      }
      locationCounter += 4;  
    } else if (regex_match(operand, sm, data_operand_pc_relativno)){
      
      int r = stoi(reg);

      ulaz.zapis.push_back((r<<4)|0x7); // regdescr
      ulaz.zapis.push_back(0x05); // addrmode

      ulaz.zapis.push_back(0x00);
      ulaz.zapis.push_back(0x00);

      ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();

      operand = operand.substr(1,operand.size() - 1);
      
      map<string, ulazTabelaSimbola>::iterator itrr;
      for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
          if (itrr->first == operand){
            *ulazTS = itrr->second;
          }
      }

      if (ulazTS->simbolId == 0){
        int id = ++Asembler::symbolId;
        
        ulazTabelaSimbola noviUlaz = {
          operand,
          id,
          0,
          0,
          "UND",
          'l',
          "NOTYP",
          false
        };      

        tabelaSimbola.insert({operand, noviUlaz});
      }
      
      ulazRelokacionaTabela ulazRT;

      ulazRT.addend = 0;
      ulazRT.offset = locationCounter + 5 - 2 -1;
      ulazRT.sekcija = this->section;
      ulazRT.simbol = "nedefinisan";
      ulazRT.tip = "TYPE_PC_16";

      relokacionaTabela.insert({operand, ulazRT});
      locationCounter += 4;
    } else if (regex_match(operand, sm, data_operand_reg_ind)){
      
      operand = operand.substr(1, 3);
      // r1 - reg
      // r2 - operand

      int r1;
      if (reg[0] == 'p'){
        r1 = 8;
      } else {
        r1 = stoi(reg);
      }
      
      int r2;
      if (operand[0] == 'p'){
        r2 = 8;
      } else {
        operand = operand.substr(1, 1);
        r2 = stoi(operand);
      }
     
      ulaz.zapis.push_back((r1<<4)|r2); //regsdescr
      ulaz.zapis.push_back(0x02); //addrmode

      locationCounter += 2;
    } else if (regex_match(operand, sm, data_operand_reg_pom)){
      //cout << operand << "\n";
      string r = operand.substr(2, 1);
      //operand = operand.substr(6,1);
      string opr = operand.substr(6, operand.size() - 7);

      int r1;
      if (r[0] == 'p'){
        r1 = 8;
      } else {
        r1 = stoi(r);
      }
      
      int r2;
      if (opr[0] == 'p'){
        r2 = 8;
      } else {
        //opr = opr.substr(1, 1);
        r2 = stoi(opr);
      }

      int rd = stoi(reg);
     
      ulaz.zapis.push_back((rd<<4)|r1); //regsdescr
      ulaz.zapis.push_back(0x03); // addrmode

      
      if(regex_match(opr, sm, literal_dec)){
        int v = stoi(opr);
        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
        
      } else if(regex_match(opr, sm, literal_hex)) {

        int v = stoi(opr, nullptr, 16);
        ulaz.zapis.push_back((char)v);
        ulaz.zapis.push_back((char)(v>>8));
        
      } else if (regex_match(opr, sm, reg_simbol)){

        ulaz.zapis.push_back(0x00);
        ulaz.zapis.push_back(0x00);

        ulazTabelaSimbola* ulazTS = new ulazTabelaSimbola();
      
        map<string, ulazTabelaSimbola>::iterator itrr;
        for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
          if (itrr->first == operand){
            *ulazTS = itrr->second;
          }
        }

        if (ulazTS->simbolId == 0){
          int id = ++Asembler::symbolId;
          
          ulazTabelaSimbola noviUlaz = {
            opr,
            id,
            0,
            0,
            "UND",
            'l',
            "NOTYP",
            false
          };      

          tabelaSimbola.insert({operand, noviUlaz});
        }
      
        ulazRelokacionaTabela ulazRT;

        ulazRT.addend = 0;
        ulazRT.offset = locationCounter + 5 - 2 - 1;
        ulazRT.sekcija = this->section;
        ulazRT.simbol = "nedefinisan";
        ulazRT.tip = "TYPE_16";

        relokacionaTabela.insert({opr, ulazRT});
             
      }
      locationCounter +=4;  
    } else {
      cout << "Greska: Operand(Podatak) nije sintaksno dobro napisan";
      return -1;
    }
  }
  return 0;
}



void Asembler::makeBinarniFajl(string output){
  ofstream* file = new ofstream(output, ios::out | ios::binary);
  ofstream* file2 = new ofstream("assembler.txt", ios::out);

  if (!file->is_open()){
    cout << "Greska pri otvranju izlaznog fajla!\n" ;
  }

  if (!file2->is_open()){
    cout << "Greska pri otvranju izlaznog fajla!\n" ;
  }

  unsigned section_tables_count = tabelaSekcija.size();
  file->write((char*)&section_tables_count, sizeof(section_tables_count));
  file2->write((char*)&section_tables_count, sizeof(section_tables_count));
  

  for (auto x: tabelaSekcija){
    ulazTabelaSekcija ulaz = x.second;
    unsigned szname = ulaz.ime.size();


    file->write((char*)&szname, sizeof(szname));
    file2->write((char*)&szname, sizeof(szname));
    file->write(ulaz.ime.c_str(), ulaz.ime.length());
    file2->write(ulaz.ime.c_str(), ulaz.ime.length());

    file->write((char*)&ulaz.duzinaSekcije, sizeof(ulaz.duzinaSekcije));
    file2->write((char*)&ulaz.duzinaSekcije, sizeof(ulaz.duzinaSekcije));
    file->write((char*)&ulaz.sectionId, sizeof(ulaz.sectionId));
    file2->write((char*)&ulaz.sectionId, sizeof(ulaz.sectionId));

    for (char c: ulaz.zapis){
      file->write((char*)&c, sizeof(c));
      file2->write((char*)&c, sizeof(c));
    }
  }

  unsigned symbol_table_count = tabelaSimbola.size();
  file->write((char*)&symbol_table_count, sizeof(symbol_table_count));
  file2->write((char*)&symbol_table_count, sizeof(symbol_table_count));

  for (auto x: tabelaSimbola){
    string simbol = x.first;
    ulazTabelaSimbola ulaz = x.second;

    unsigned szname = ulaz.ime.length();
    file->write((char*)&szname, sizeof(szname));
    file2->write((char*)&szname, sizeof(szname));
    file->write(ulaz.ime.c_str(), szname);
    file2->write(ulaz.ime.c_str(), szname);

    file->write((char*)&ulaz.simbolId, sizeof(ulaz.simbolId));
    file2->write((char*)&ulaz.simbolId, sizeof(ulaz.simbolId));
    file->write((char*)&ulaz.offset, sizeof(ulaz.offset));
    file2->write((char*)&ulaz.offset, sizeof(ulaz.offset));
    file->write((char*)&ulaz.sekcijaId, sizeof(ulaz.sekcijaId));
    file2->write((char*)&ulaz.sekcijaId, sizeof(ulaz.sekcijaId));
    
    unsigned szsection = ulaz.sekcija.length();
    file->write((char*)&szsection, sizeof(szsection));
    file2->write((char*)&szsection, sizeof(szsection));
    file->write(ulaz.sekcija.c_str(), szsection);
    file2->write(ulaz.sekcija.c_str(), szsection);

    file->write((char*)&ulaz.tip, sizeof(ulaz.tip));
    file2->write((char*)&ulaz.tip, sizeof(ulaz.tip));

    unsigned szvrsta = ulaz.vrsta.length();
    file->write((char*)&szvrsta, sizeof(szvrsta));
    file2->write((char*)&szvrsta, sizeof(szvrsta));
    file->write(ulaz.vrsta.c_str(), szvrsta);
    file2->write(ulaz.vrsta.c_str(), szvrsta);

    file->write((char*)&ulaz.definisan, sizeof(ulaz.definisan));
    file2->write((char*)&ulaz.definisan, sizeof(ulaz.definisan));
  }

  unsigned rel_tab_size = relokacionaTabela.size();
  file->write((char*)&rel_tab_size, sizeof(rel_tab_size));
  file2->write((char*)&rel_tab_size, sizeof(rel_tab_size));

  for (auto x: relokacionaTabela){

    unsigned symbolsz = x.first.length();
    file->write((char*)&symbolsz, sizeof(symbolsz));
    file2->write((char*)&symbolsz, sizeof(symbolsz));
    file->write((char*)x.first.c_str(), symbolsz);
    file2->write((char*)x.first.c_str(), symbolsz);

    ulazRelokacionaTabela ulaz = x.second;

    unsigned szsection = ulaz.sekcija.length();
    file->write((char*)&szsection, sizeof(szsection));
    file2->write((char*)&szsection, sizeof(szsection));
    file->write(ulaz.sekcija.c_str(), szsection);
    file2->write(ulaz.sekcija.c_str(), szsection);

    file->write((char*)&ulaz.offset, sizeof(ulaz.offset));
    file2->write((char*)&ulaz.offset, sizeof(ulaz.offset));

    unsigned sztip = ulaz.tip.length();
    file->write((char*)&sztip, sizeof(sztip));
    file2->write((char*)&sztip, sizeof(sztip));
    file->write(ulaz.tip.c_str(), sztip);
    file2->write(ulaz.tip.c_str(), sztip);

    unsigned szsimbol = ulaz.simbol.length();
    file->write((char*)&szsimbol, sizeof(szsimbol));
    file2->write((char*)&szsimbol, sizeof(szsimbol));
    file->write(ulaz.simbol.c_str(), szsimbol);
    file2->write(ulaz.simbol.c_str(), szsimbol);

    file->write((char*)&ulaz.addend, sizeof(ulaz.addend));
    file2->write((char*)&ulaz.addend, sizeof(ulaz.addend));
  }
  file->close();
  file2->close();
}

void Asembler::dodajAddend(){
  for(auto it = relokacionaTabela.begin(); it != relokacionaTabela.end(); it++){
    ulazTabelaSimbola ulaz = tabelaSimbola.at(it->first);
    if (it->second.simbol == "nedefinisan") it->second.simbol = ulaz.ime;
   
    if (ulaz.tip == 'l'){
      it->second.addend += ulaz.offset;
    }
  }
}

void Asembler::asembliraj(const char* input, const char* output){
  if (input == nullptr || output == nullptr) {
    cout<< "Losi ulazni podaci";
  }

  ifstream in;
  in.open(input);
  cout << output;
  if (!in.is_open()){
    cout << "Greska: Greska pri otvaranju fajla " << input << "\n"  ;
  }else {
    string linija;
    while (getline(in, linija)){
      linija = regex_replace(linija, komentar, "$1");
      linija = regex_replace(linija, spaces, " ");
      linija = regex_replace(linija, tabovi, " ");
      linija = regex_replace(linija, zarez_space, ",");
      linija = regex_replace(linija, labela_space, ":");
      linija = regex_replace(linija, boundary_space, "$2");
      if (linija == "" || linija == " ") {continue;}
      linijeAsm.push_back(linija);
    }

    locationCounter = 0;
    sectionId = 0;
    section = "UND";
    vector<char> v;
    //v.push_back("#.und");
    ulazTabelaSekcija ulaz = {"UND", 0, 0, v};
    tabelaSekcija.insert({"UND", ulaz});
    
    for (string s: linijeAsm){
        smatch sm;

        //obrada labele da li ima naredbu ili ne
        if(regex_match(s, sm, labela)){
          int ret = obradiLabelu(sm.str(1));
          if (ret < 0) return;
          continue;
        }else if (regex_match(s, sm, labela_text)){
          int ret = obradiLabelu(sm.str(1));
          if (ret < 0) return;  
          s = sm.str(2);    
          //cout << s;     
        }
        
        if (regex_match(s, sm, global)){
          int ret = obradiGlobal(sm.str(1));
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, reg_extern)){
          int ret = obradiExtern(sm.str(1));
          if (ret<0) return;
          continue;
        } else if (regex_match(s, sm, sekcija)){
          int ret = obradiSekciju(sm.str(1));
          if (ret<0) return;
          continue;
        } else if (regex_match(s, sm, word)){
          int ret = obradiWord(sm.str(1));
          if (ret<0) return;
          continue;
        } else if (regex_match(s, sm, skip)){
          int ret = obradiSkip(sm.str(1));
          continue;
        } else if (regex_match(s, sm, end)){
          break;
        } else if (regex_match(s, sm, halt)){
          int ret = obradiHalt();
          if (ret<0) return;
          continue;
        } else if (regex_match(s, sm, instrukcija_int)){
          string sub = sm.str(1).substr(1, sm.str(1).size());
          int ret = obradiInt(sub);
          if (ret<0) return;
          continue;
        } else if (regex_match(s, sm, iret)){
          int ret = obradiIret();
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, call)){
          int ret = obradiCall(sm.str(2));
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, ret)){
          int ret = obradiRet();
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, jmp)){
          int ret = obradiJmp(sm.str(2));
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, jeq)){
          int ret = obradiJeq(sm.str(2));
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, jne)){
          int ret = obradiJne(sm.str(2));
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, jgt)){
          int ret = obradiJgt(sm.str(2));
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, push)){
          string sub = sm.str(1).substr(1, sm.str(1).size());
          int ret = obradiPush(sub);
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, pop)){
          string sub = sm.str(1).substr(1, sm.str(1).size());
          int ret = obradiPop(sub);
          if (ret < 0) return;
          continue;
        } else if (regex_match(s, sm, xchg)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiXchg(reg1, reg2);
          if (ret < 0) return;
          continue;          
        } else if (regex_match(s, sm, add)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiAdd(reg1, reg2);
          if (ret < 0) return;
          continue;          
        } else if (regex_match(s, sm, sub)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiSub(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, mul)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiMul(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, div)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiDiv(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, cmp)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiCmp(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, reg_not)){
          string reg1 = sm.str(1);
          int ret = obradiNot(reg1);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, reg_and)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiAnd(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, reg_or)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiOr(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, reg_xor)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiXor(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, reg_test)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiTest(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, shl)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiShl(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, shr)){
          string reg1 = sm.str(1);
          string reg2 = sm.str(2);
          int ret = obradiShr(reg1, reg2);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, ldr)){
          string reg = sm.str(2).substr(0, 2);
          string oper = sm.str(2).substr(3, sm.str(2).size() - 3);
          int ret = obradiLdr(reg, oper);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, str)){
          string reg = sm.str(2).substr(0, 2);
          string oper = sm.str(2).substr(3, sm.str(2).size() - 3);
          int ret = obradiStr(reg, oper);
          if (ret < 0) return;
          continue; 
        } else if (regex_match(s, sm, ascii)){
          string str = sm.str(2);
          int ret = obradiAscii(str);
          if (ret < 0) return;
          continue;
        }
    }

    ulazTabelaSekcija &ulazTS = tabelaSekcija.at(this->section);
    ulazTS.duzinaSekcije = locationCounter;

    dodajAddend();

    makeBinarniFajl(output);

    
    //cout << "\n" << input << "\n";

    //ovo je ispis simbola
    /*map<string, ulazTabelaSimbola>::iterator itrr;
    for (itrr = tabelaSimbola.begin(); itrr != tabelaSimbola.end(); itrr++){
      cout << itrr->second.simbolId << " " << itrr->second.ime << "\t" << itrr->second.tip << "\t" << itrr->second.offset << "\n";
      
    }
    cout << "\n";
    // ovo ispis zapisa sekcija>
    map<string, ulazTabelaSekcija>::iterator it;
    for (it = tabelaSekcija.begin(); it != tabelaSekcija.end(); it++){
      //cout << it->second.zapis << " ";
      // for (string str: it->second.zapis){
      //   cout << str << " ";
      // }
      cout << it->second.ime << "\t" << it->second.sectionId << "\t"
      << it->second.duzinaSekcije << "\n";
      int i = 0;
      for (unsigned char c: it->second.zapis){
        //i++;
        
        cout << std::hex << (c - 0) << " ";
        if ((++i%8) == 0) cout << "\n";
      }
      cout << "\n";
    }

    cout << "\n";
    cout << "RELOKACIONA TABELA" << endl;
    for (auto itr = relokacionaTabela.begin(); itr!=relokacionaTabela.end(); itr++){
      cout << itr->first << "\t" <<
      itr->second.sekcija << "\t" << itr->second.offset << 
      "\t" << itr->second.tip << "\t" <<  itr->second.simbol <<
      "\t" << itr->second.addend << "\n";
    }*/
  }
}

int main(int argc, char* argv[]){

  // ifstream in;
  // in.open(argv[3]);

  // if (!in.is_open()){
  //   cout << "Greska: Greska pri otvaranju fajla " << "\n";
  // }

  // Asembler a;
  // a.asembliraj("../tests/main.s", "main.o");

  if (argc<2){
    cout << "Neodgovarajuc broj argumenata" << endl;
    return -1;
  }
  string prvi = argv[0];
  if (prvi == "./assembler"){
    if (argc<4){
      cout << "Neodgovarajuc broj argumenataa" << endl;
      return -2;
    }else{
      string drugi = argv[1];
      if (drugi != "-o"){
        cout << "Greska u komandnoj liniji" << endl;
        return -3;
      }
      
      Asembler a;
      a.asembliraj(argv[3], argv[2]);
      return 0;
    }
  }  
    
}