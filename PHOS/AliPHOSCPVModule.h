#ifndef ALIPHOSCPVMODULE_H
#define ALIPHOSCPVMODULE_H
/* Copyright(c) 1998-1999, ALICE Experiment at CERN, All rights reserved. *
 * See cxx source for full Copyright notice                               */

/* $Id$ */

////////////////////////////////////////////////
//  Manager class for one CPV module          //
//                                            //
//  Author: Yuri Kharlov, IHEP, Protvino      //
//  e-mail: Yuri.Kharlov@cern.ch              //
//  Last modified: 7 November 2000            //
////////////////////////////////////////////////
 
#include <assert.h> 

// --- ROOT system ---
#include <TClonesArray.h> 
#include <TLorentzVector.h>

// --- galice header files ---

class AliPHOSCPVModule : public TObject {

public:

  virtual ~AliPHOSCPVModule(void);
           AliPHOSCPVModule(void);
           AliPHOSCPVModule(const AliPHOSCPVModule & cpv);
  
  AliPHOSCPVModule & operator = (const AliPHOSCPVModule  & rvalue);

  void     Copy(AliPHOSCPVModule &module) const;
  void     Clear(Option_t *opt="");
  void     Print(Option_t *opt="");
  void     AddHit(Int_t shunt, Int_t track, TLorentzVector p, Float_t *xy, Int_t ipart);
  void     MakeBranch    (char *title, Int_t i, char *file=0);
  void     SetTreeAddress(char *title, Int_t i);
  
  TClonesArray *Hits         (void) {return fHits;}

private:
  
  TClonesArray *fHits;              // List of hits in the Module per one current track

  ClassDef(AliPHOSCPVModule,1)      // CPV Module
};
 
#endif
