#include <iostream>
#include <fstream>
#include <list>
#include <iomanip>

#include "../inc/linker.hpp"

int Linker::symbolId = 0;

void Linker::otvoriBinarniFajl(string type, list<string> inputFiles){
  for (auto it = inputFiles.begin(); it!=inputFiles.end(); it++){
    string input = "./";
    input.append(it->c_str());

    ifstream in;
    in.open(input);

    if(!in.is_open()){
      cout << "Greska pri otvaranju fajla: " << input << endl;
    }

    // cout << input << endl;
    int section_cnt;
	  in.read((char *)&section_cnt, sizeof(section_cnt));    
    map<string, ulazTabelaSekcija> tabelaSekcija;

    for (int i = 0; i < section_cnt; i++){
      unsigned len;
		  in.read((char *)&len, sizeof(len));

      string ime;
      ime.resize(len);
      in.read((char *)ime.c_str(), len);

      int duzina_sekcije;
      in.read((char *)&duzina_sekcije, sizeof(duzina_sekcije));

      int id_sekcije;
      in.read((char*)&id_sekcije, sizeof(id_sekcije));

      vector<char> zapis;

      for(int i=0; i <duzina_sekcije; i++){
        char c;
        in.read((char*)&c, sizeof(char));
        zapis.push_back(c);
      }
      
      ulazTabelaSekcija ulaz = {
        ime, id_sekcije, duzina_sekcije, zapis, 0
      };

      tabelaSekcija.insert({ime, ulaz});
      // cout << len << endl;
      // cout << ime << endl;
      // cout << duzina_sekcije << endl;
      // cout << id_sekcije << endl;

      // for(unsigned char c: zapis){
      //   cout << (c + 0) << " ";
      // }
    }
    zajednickaTabelaSekcija.insert({it->c_str(), tabelaSekcija});

    int symbol_table_count;
    in.read((char*)&symbol_table_count, sizeof(symbol_table_count));
    map<string, ulazTabelaSimbola> tabelaSimbola;

    for (int i=0; i < symbol_table_count; i++){
      unsigned len;
		  in.read((char *)&len, sizeof(len));

      string ime;
      ime.resize(len);
      in.read((char *)ime.c_str(), len);

      int simbolId;
      in.read((char*)&simbolId, sizeof(simbolId));

      int offset;
      in.read((char*)&offset, sizeof(offset));

      int sekcijaId;
      in.read((char*)&sekcijaId, sizeof(sekcijaId));

      unsigned szsection;
      in.read((char*)&szsection, sizeof(szsection));
      string sekcija;
      sekcija.resize(szsection);
      in.read((char*)sekcija.c_str(), szsection);

      char tip;
      in.read((char*)&tip, sizeof(tip));

      unsigned szvrsta;
      in.read((char*)&szvrsta, sizeof(szvrsta));
      string vrsta;
      vrsta.resize(szvrsta);
      in.read((char*)vrsta.c_str(), szvrsta);

      bool definisan;
      in.read((char*)&definisan, sizeof(definisan));

      ulazTabelaSimbola ulaz = {
        ime, simbolId, offset, sekcijaId,
         sekcija, tip, vrsta, definisan, it->c_str()
      };

      tabelaSimbola.insert({ime, ulaz});
      // cout << len << ime << simbolId << "|"
      // << offset << "|" << sekcijaId << szsection
      // << sekcija<< tip << szvrsta << vrsta << definisan << endl;
    }
    zajednickaTabelaSimbola.insert({it->c_str(), tabelaSimbola});

    unsigned rel_tab_size;
    in.read((char*)&rel_tab_size, sizeof(rel_tab_size));

    multimap <string, ulazRelokacionaTabela> relokacionaTabela;
    for (int i=0; i<rel_tab_size; i++){

      unsigned symbolsz;
      in.read((char*)&symbolsz, sizeof(symbolsz));
      string symbol;
      symbol.resize(symbolsz);
      in.read((char*)symbol.c_str(), symbolsz);

      unsigned szsection;
      in.read((char*)&szsection, sizeof(szsection));
      string sekcija;
      sekcija.resize(szsection);
      in.read((char*)sekcija.c_str(), szsection);

      int offset;
      in.read((char*)&offset, sizeof(offset));

      unsigned sztip;
      in.read((char*)&sztip, sizeof(sztip));
      string tip;
      tip.resize(sztip);
      in.read((char*)tip.c_str(), sztip);

      unsigned szsimbol;
      in.read((char*)&szsimbol, sizeof(szsimbol));
      string simbol;
      simbol.resize(szsimbol);
      in.read((char*)simbol.c_str(), szsimbol);

      int addend;
      in.read((char*)&addend, sizeof(addend));
      

      ulazRelokacionaTabela ulaz={
        sekcija, offset, tip, simbol, addend, it->c_str() 
      };

      // if (symbol == "mathAdd"){
      //   cout << "MATH: " << offset << endl;
      // }

      relokacionaTabela.insert({symbol, ulaz});
      // cout << symbol << szsection << sekcija << offset << "|"
      // << sztip << tip << szsimbol << simbol << addend << it->c_str() << endl;
    }
    zajednickaRelokacionaTabela.insert({it->c_str(), relokacionaTabela});
    in.close();
  }
}

void Linker::postaviPocetneAdrese(){
    int start_addr = 0;
    int next_addr = 0;

    for(auto it = inputFiles.begin(); it != inputFiles.end(); it++){
      map<string, ulazTabelaSekcija>& tabela_sekcija = zajednickaTabelaSekcija.at(it->c_str());
      for (auto itts = tabela_sekcija.begin(); itts != tabela_sekcija.end(); itts++){
        if (itts->first == "UND")continue;
        if (tabelaSekcija.count(itts->first) == 0){
            itts->second.pocAdresa = next_addr;
            ulazTabelaSekcija ulaz = {
              itts->second.ime,
              itts->second.sectionId,
              itts->second.duzinaSekcije,
              itts->second.zapis,
              next_addr
            };
                    
            for(auto it2 = it; it2 != inputFiles.end(); it2++){
              if (it2 == it) continue;
              map<string, ulazTabelaSekcija>& tabela_sekcija2 = zajednickaTabelaSekcija.at(it2->c_str());
              if (tabela_sekcija2.count(itts->first) == 1){
                ulazTabelaSekcija& ulaz2 = tabela_sekcija2.at(itts->first);
                ulaz2.pocAdresa += ulaz.pocAdresa + ulaz.duzinaSekcije;
                ulaz.duzinaSekcije += ulaz2.duzinaSekcije;
              }
            }
            next_addr += ulaz.duzinaSekcije;
            tabelaSekcija.insert({itts->first, ulaz});
        }
      }
    }
    // cout << "TABELA SEKCIJA" << endl;
    // for (auto it = tabelaSekcija.begin(); it !=tabelaSekcija.end(); it++){
    //   cout << it->second.ime << it->second.duzinaSekcije << "|" <<it->second.pocAdresa << endl;
    // }
}

int Linker::napraviTabeluSimbola(){
  list<string> externiSimboli;
  for (auto x: tabelaSekcija){
    ulazTabelaSimbola ulaz = {
      x.second.ime,
      ++this->symbolId,
      x.second.pocAdresa,
      x.second.sectionId,
      x.second.ime,
      'g',
      "SCTN",
      true
    };

    tabelaSimbola.insert({x.first, ulaz});
  }

  for (auto it = inputFiles.begin(); it != inputFiles.end(); it++){
    map<string, ulazTabelaSimbola>& tabela_simbola = zajednickaTabelaSimbola.at(it->c_str());
    for (auto itts = tabela_simbola.begin(); itts != tabela_simbola.end(); itts++){
      if (itts->second.tip == 'e'){
        bool found = 
        (std::find(externiSimboli.begin(), externiSimboli.end(), itts->first) != externiSimboli.end());
        if (!found && tabelaSimbola.count(itts->first) == 0){
          externiSimboli.push_back(itts->first);
        }
      } else if (itts->second.tip == 'g'){
        if (tabelaSimbola.count(itts->first) != 0){
          cout << "Dvostruko definisan simbol " << itts->first << endl;
        }else {
          map<string, ulazTabelaSekcija> tabela_sek = zajednickaTabelaSekcija.at(it->c_str());
          ulazTabelaSekcija ulaz_sekcija = tabela_sek.at(itts->second.sekcija);

          int noviOfset = ulaz_sekcija.pocAdresa + itts->second.offset;
          ulazTabelaSimbola ulaz = {
            itts->first, ++this->symbolId, noviOfset, ulaz_sekcija.sectionId,
            itts->second.sekcija, 'g' , itts->second.vrsta, itts->second.definisan,
            it->c_str()
          };
          tabelaSimbola.insert({itts->first, ulaz});     
        }
      } else if (itts->second.tip == 'l'){
        map<string, ulazTabelaSekcija> tabela_sek = zajednickaTabelaSekcija.at(it->c_str()); // tabela sekcija za taj fajl
        ulazTabelaSekcija ulaz_sekcija = tabela_sek.at(itts->second.sekcija); // ulaz za sekciju u kojoj je definisan sim

        int noviOfset = ulaz_sekcija.pocAdresa;
        itts->second.offset += ulaz_sekcija.pocAdresa;

      }
    }
  }

  for (string sim: externiSimboli){
    if (tabelaSimbola.count(sim) == 0){
      cout << "Nedefinisan simbol " << sim << endl;
    }
  }

  // cout << "TABELA SIMBOLA" << endl;
  // for (auto it = tabelaSimbola.begin(); it != tabelaSimbola.end(); it++){
  //   cout << it->second.ime << it->second.simbolId 
  //   << it->second.sekcija << it->second.tip << it->second.offset
  //   << it->second.vrsta << it->second.definisan << endl;
  // }
  return 0;
}

void Linker::napraviHexFajl(string outputFile){
  ofstream* filehex = new ofstream(outputFile, ios::out | ios::binary);
  ofstream* filetxt = new ofstream("program.txt", ios::out);

  if (!filehex->is_open()){
    cout << "Greska pri otvaranju izlaznog fajla!\n" ;
  }

  if (!filetxt->is_open()){
    cout << "Greska pri otvaranju tekstualnog fajla!" << endl;
  }

  int pocAddr = 0;
  int i = 0;
  bool flag = false;

  for (auto it = zajednickaTabelaSekcija.begin(); it != zajednickaTabelaSekcija.end(); it++){
    repeat:
    map<string , ulazTabelaSekcija> tab_sek = it->second;
    for (auto itts = tab_sek.begin(); itts != tab_sek.end(); itts++){
      if (itts->second.pocAdresa == pocAddr){
        for (unsigned char c: itts->second.zapis){
          if (i % 8 == 0){            
            *filetxt << setfill('0') << std::setw(4) << hex << (int)i << ": "; 
          }
          filehex->write((char*)&c, sizeof(c));
          *filetxt << setfill('0') << std::setw(2) << hex << (int)c << " ";
          if (++i % 8 == 0) {
            *filetxt << endl;
          }
          flag = true;
        }
        
        //it = zajednickaTabelaSekcija.begin();
        //itts = tab_sek.begin();
        //break;
      }
      if (flag){
        flag = false;
        pocAddr += itts->second.duzinaSekcije;
        it = zajednickaTabelaSekcija.begin();
        goto repeat;
      }
    }
  }
  unsigned char c = '\n';
  filehex->write((char*)&c, sizeof(c));

  filehex->close();
  filetxt->close();

}

void Linker::obradiPlace(){
  
  regex place("-place=([a-zA-Z][_A-Za-z0-9]*)@0x([0-9A-Fa-f]+)");
	smatch match;
	for (string elem : this->places)
	{
		if (regex_match(elem, match, place))
		{
			string section = match.str(1);
      string adr = match.str(2);
      adr.substr(2, adr.size()-2);
      cout << adr << endl;
			int address = stoi(adr);
			pocetneAdrese.insert({section, address});
		}
	}
}

void Linker::link(string type, list<string> inputFiles, string outputFile){
  this->inputFiles = inputFiles;
  otvoriBinarniFajl(type, inputFiles);

  obradiPlace();
  postaviPocetneAdrese();
  int ret = napraviTabeluSimbola();
  if (ret < 0) return;
  razresiRelZapis();
  napraviHexFajl(outputFile);

  // for (auto it = zajednickaTabelaSekcija.begin(); it != zajednickaTabelaSekcija.end(); it++){
  //   cout << it->first << endl;
  //   for (auto itts= it->second.begin(); itts != it->second.end(); itts++){
  //     int i = 0;
  //     cout << itts->first << endl;
  //     for (unsigned char c: itts->second.zapis){
  //       cout << std::hex << (c-0) << " ";
  //       if ((++i % 8) == 0) cout << endl;

  //     }
  //     cout << endl;
  //   }
  // }
}

void Linker::razresiRelZapis(){
  for(string s: inputFiles){
    multimap<string, ulazRelokacionaTabela> rel_tab = zajednickaRelokacionaTabela.at(s);
    for (auto itrt = rel_tab.begin(); itrt != rel_tab.end(); itrt++){
      if (tabelaSimbola.count(itrt->first) == 1){
        // global 
        ulazTabelaSimbola ulaz = tabelaSimbola.at(itrt->first);
        // sekcija u kojoj se vrsi relokacija
        ulazTabelaSekcija& ulaz_tab_sek = zajednickaTabelaSekcija.at(itrt->second.inputFile).at(itrt->second.sekcija);
        
        if (itrt->second.tip == "TYPE_16"){
          ulaz_tab_sek.zapis[itrt->second.offset] = (char)ulaz.offset;
          ulaz_tab_sek.zapis[itrt->second.offset+1] = (ulaz.offset >> 8);
        }else {
          unsigned short pc = ulaz_tab_sek.pocAdresa + itrt->second.offset + 2;
          unsigned short rel_adr = ulaz.offset - pc;
          ulaz_tab_sek.zapis[itrt->second.offset+1] = (rel_adr >> 8);
          ulaz_tab_sek.zapis[itrt->second.offset] = (char)rel_adr;
        }
      }else {
        
        // lokalni 
        // treba mi sekcija u kojoj se nalazi taj simbol
        ulazTabelaSimbola ulaz;
        for (string s:inputFiles){
          map<string, ulazTabelaSimbola> tab_sim = zajednickaTabelaSimbola.at(s);
          for (auto it = tab_sim.begin(); it != tab_sim.end(); it++){
            if (it->second.ime == itrt->second.simbol) ulaz = it->second;
          }
        }

        ulazTabelaSekcija& ulaz_tab_sek = zajednickaTabelaSekcija.at(itrt->second.inputFile).at(itrt->second.sekcija);
        unsigned short vrednost_simbola = ulaz.offset + itrt->second.addend;
        
        if (itrt->second.tip == "TYPE_16"){
          ulaz_tab_sek.zapis[itrt->second.offset + 1] = (vrednost_simbola >> 8);
          ulaz_tab_sek.zapis[itrt->second.offset] = (char)vrednost_simbola;
        }else{
          unsigned short pc = ulaz_tab_sek.pocAdresa + itrt->second.offset + 2;
          unsigned short rel_adr = vrednost_simbola - pc;
          ulaz_tab_sek.zapis[itrt->second.offset + 1] = (rel_adr >> 8);
          ulaz_tab_sek.zapis[itrt->second.offset] = (char)rel_adr;
        }     
      }
    }
  }
}


int main(int argc, char* argv[]){

  regex place = regex("-place=([a-zA-Z][_A-Za-z0-9]*)@0x([0-9A-Fa-f]+)");
  regex input = regex("^([a-zA-Z][_A-Za-z0-9]*)\\.o$");
  regex output = regex("^([a-zA-Z][_A-Za-z0-9]*)\\.hex$");

  list<string> inputFiles;
  string outputFile;
  string type;
  Linker l;
  
  string arg = argv[1];
  if (arg == "-hex"){
    type = "hex";
  }else if (arg == "-relocatable"){
    type = "relocatable";
  } else {
    printf("Pogresan prvi argument pri pozivu linkera");
    return -1;
  }
  
  // int i = 2;
  // arg = argv[i];
  // while(regex_search(arg,place)){
  //   l.places.push_back(arg);
  //   i++;
  //   arg = argv[i];
  // }

  for (int i=2; i<argc; i++){
    arg = argv[i];
    if(arg == "-o"){
      i++;
      for (; i<argc; i++){
        arg = argv[i];
        smatch sm;
        if (regex_match(arg, sm, place)){
          l.places.push_back(arg);
        }else if (regex_match(arg, sm, input)){
          inputFiles.push_back(arg);
        } else if (regex_match(arg, sm, output)){
          outputFile = arg;
        }
      }
    }
  }
  //cout << "LINKER\n";
  // cout << endl;
  // cout << outputFile << endl;
  // for (string s: inputFiles){
  //   cout << s << endl;
  // }
 
  l.link(type, inputFiles, outputFile);
}