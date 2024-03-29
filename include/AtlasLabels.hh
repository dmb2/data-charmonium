//
//   @file    AtlasLabels.h         
//   
//   @author M.Sutton
// 
//   Copyright (C) 2010 Atlas Collaboration
//
//   $Id$


#ifndef __ATLASLABELS_H
#define __ATLASLABELS_H
#include "TColor.h"
#include "TString.h"
/*
void ATLASLabelAutoScale(Double_t x,Double_t y,bool Preliminary=false,Color_t color=1); 

void ATLASLabelOld(Double_t x,Double_t y,bool Preliminary=false,Color_t color=1); 
*/
void ATLASLabel(Double_t x,Double_t y,bool Preliminary,Color_t color,const TString & conditions); 


void ATLASVersion(char* version=NULL,Double_t x=0.88,Double_t y=0.975,Color_t color=1); 

void myText(Double_t x,Double_t y,Color_t color,char *text); 

void myBoxText(Double_t x, Double_t y,Double_t boxsize,Int_t mcolor,char *text); 

void myMarkerText(Double_t x,Double_t y,Int_t color,Int_t mstyle,char *text); 

#endif // __ATLASLABELS_H
