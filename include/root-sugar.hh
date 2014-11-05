#pragma once
#include "TFile.h"

template <typename T>
T* retrieve(const char* fname,const char* objname){
  //This probably leaks like a sieve 
  TFile* file = TFile::Open(fname);
  return dynamic_cast<T*>(file->Get(objname));
}
