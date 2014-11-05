#pragma once
#include "TFile.h"

template <typename T>
T* retrieve(TFile* file,const char* objname){
  return dynamic_cast<T*>(file->Get(objname));
}
template <typename T>
T* retrieve(const char* fname,const char* objname){
  //This probably leaks like a sieve 
  TFile* file = TFile::Open(fname);
  return retrieve<T>(file,objname);
}
