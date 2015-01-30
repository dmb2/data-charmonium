#pragma once
#include "TFile.h"
#include <iostream>

#define DEBUG_LEVEL 1

#define MSG(message) std::cout<<""<<message<<""<<std::endl
#define MSG_ERR(message) std::cerr<<"\033[31m"<<message<<"\033[0m"<<std::endl
#define MSG_DEBUG(message) if(DEBUG_LEVEL > 0){std::cout<<"\033[32m"<<message<<"\033[0m"<<std::endl;}


template <typename T>
T* retrieve(TFile* file,const char* objname){
  T* obj=NULL;
  try{
    file->GetObject(objname,obj);
    if(obj==NULL){
      throw -2;
    }
  }
  catch(int e){
    obj = new T;
    MSG_ERR("Could not retrieve object "<<obj->ClassName()<<"::"<<objname);
    delete obj;
    exit(e);
  }
  return obj;
}

//A real solution should keep a map<string,TFile*> and search it to
//see if the file is open, if not add it and open it to avoid
//leaking. This would also provide a poor man's garbage collector for
//TFile objects... 

//A really real solution would keep a hash table of TObjects keyed by
//their name, and actually garbage collect them depending on their
//state and ownership...
template <typename T>
T* retrieve(const char* fname,const char* objname){
  //This probably leaks like a sieve 
  try{
    TFile* file = TFile::Open(fname);
    if(file==NULL) throw -1;
    return retrieve<T>(file,objname);
  }
  catch(int e){
    if(e==-1){
      MSG_ERR("Error: Could not open file, "<<fname);
      exit(e);
    }
  }
  return NULL;
}
