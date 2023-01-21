#pragma once

#include <list>
#include <iostream>
#include <regex>

using namespace std;

struct ulazTabelaSekcija{
  string ime;
  int sectionId;
  int duzinaSekcije;
  vector<char> zapis;
  int pocAdresa;
};

struct ulazTabelaSimbola{
  string ime;
  int simbolId; 
  int offset; 
  int sekcijaId;
  string sekcija;
  char tip;
  string vrsta;
  bool definisan;
  string inputFile;
};

struct ulazRelokacionaTabela{
  string sekcija;
  int offset;
  string tip;
  string simbol;
  int addend;
  string inputFile; 
};

class Linker{
  public:
  static int symbolId;
  list<string> places;
  map<string, int> pocetneAdrese;

  map<string, map<string, ulazTabelaSekcija>> zajednickaTabelaSekcija;
  map<string, map<string, ulazTabelaSimbola>> zajednickaTabelaSimbola;
  map<string, multimap<string, ulazRelokacionaTabela>> zajednickaRelokacionaTabela;

  map<string, ulazTabelaSekcija> tabelaSekcija;
  map<string, ulazTabelaSimbola> tabelaSimbola;

  list<string> inputFiles;

  void link(string type, list<string> inputFiles, string outputFile);

  void obradiPlace();
  void otvoriBinarniFajl(string type, list<string> inputFiles);
  void postaviPocetneAdrese();
  int napraviTabeluSimbola();
  void razresiRelZapis();
  void napraviHexFajl(string output);
  
  ulazTabelaSimbola nadjiSimbol(string ime, string sekcija);
};