//Author:        Anders Strand Vestbo
//Last Modified: 28.6.01

#include "AliL3HoughEval.h"
#include "AliL3HoughTransformer.h"
#include "AliL3DigitData.h"
#include "AliL3HoughTrack.h"
#include "AliL3Transform.h"
#include "AliL3Histogram.h"
#include "AliL3Defs.h"

ClassImp(AliL3HoughEval)

AliL3HoughEval::AliL3HoughEval()
{
    
}

AliL3HoughEval::AliL3HoughEval(AliL3HoughTransformer *transformer)
{
  
  fHoughTransformer = transformer;
  fTransform = new AliL3Transform();
  
  fSlice = fHoughTransformer->GetSlice();
  fPatch = fHoughTransformer->GetPatch();
  fNrows = NRows[fPatch][1] - NRows[fPatch][0] + 1;
  fNEtaSegments = fHoughTransformer->GetNEtaSegments();
  fEtaMin = fHoughTransformer->GetEtaMin();
  fEtaMax = fHoughTransformer->GetEtaMax();
  fRemoveFoundTracks = kFALSE;
  fNumOfPadsToLook = 1;
  fNumOfRowsToMiss = 1;
  GenerateLUT();
}

AliL3HoughEval::~AliL3HoughEval()
{
  if(fTransform)
    delete fTransform;
  if(fRowPointers)
    delete [] fRowPointers;
}

void AliL3HoughEval::GenerateLUT()
{
  //Generate a LUT, to limit the access to raw data
  
  fRowPointers = new AliL3DigitRowData*[fNrows];

  AliL3DigitRowData *tempPt = (AliL3DigitRowData*)fHoughTransformer->GetDataPointer();
  if(!tempPt)
    printf("AliL3HoughEval::GenerateLUT : Zero data pointer\n");
  
  for(Int_t i=NRows[fPatch][0]; i<=NRows[fPatch][1]; i++)
    {
      Int_t prow = i - NRows[fPatch][0];
      fRowPointers[prow] = tempPt;
      fHoughTransformer->UpdateDataPointer(tempPt);
    }
  
}

Bool_t AliL3HoughEval::LookInsideRoad(AliL3HoughTrack *track,Int_t eta_index,Bool_t remove)
{
  //Look at rawdata along the road specified by the track candidates.
  //If track is good, return true, if not return false.
    
  Int_t sector,row;
  
  Int_t nrow=0,npixs=0;
  Float_t xyz[3];
  
  Double_t etaslice = (fEtaMax - fEtaMin)/fNEtaSegments;
  for(Int_t padrow = NRows[fPatch][0]; padrow <= NRows[fPatch][1]; padrow++)
    {
      Int_t prow = padrow - NRows[fPatch][0];
      
      if(!track->GetCrossingPoint(padrow,xyz))  
	{
	  printf("AliL3HoughEval::LookInsideRoad : Track does not cross line!!\n");
	  continue;
	}
      
      fTransform->Slice2Sector(fSlice,padrow,sector,row);
      fTransform->Local2Raw(xyz,sector,row);
      npixs=0;
      
      //Get the timebins for this pad
      AliL3DigitRowData *tempPt = fRowPointers[prow];
      if(!tempPt) 
	{
	  printf("AliL3HoughEval::LookInsideRoad : Zero data pointer\n");
	  continue;
	}
      
      //Look at both sides of the pad:
      for(Int_t p=(Int_t)xyz[1]-fNumOfPadsToLook; p<=(Int_t)xyz[1]+fNumOfPadsToLook; p++)
	{
	  AliL3DigitData *digPt = tempPt->fDigitData;
	  for(UInt_t j=0; j<tempPt->fNDigit; j++)
	    {
	      UChar_t pad = digPt[j].fPad;
	      if(pad < p) continue;
	      if(pad > p) break;
	      UShort_t time = digPt[j].fTime;
	      Double_t eta = fTransform->GetEta(row,pad,time);
	      Int_t pixel_index = (Int_t)(eta/etaslice);
	      if(pixel_index > eta_index) continue;
	      if(pixel_index != eta_index) break;
	      if(remove)
		digPt[j].fCharge = 0; //Delete the track from image
	      npixs++;
	    }
	}
            
      if(npixs > 0)
	{
	  nrow++;
	}	  
    }
  if(remove)
    return kTRUE;
  
  if(nrow >= fNrows - fNumOfRowsToMiss)//this was a good track
    {
      track->SetEtaIndex(eta_index);
      if(fRemoveFoundTracks)
	LookInsideRoad(track,eta_index,kTRUE);
      return kTRUE;
    }
  else
    return kFALSE;
}

void AliL3HoughEval::DisplayEtaSlice(Int_t eta_index,AliL3Histogram *hist)
{
  //Display the current raw data inside the slice

  if(!hist)
    {
      printf("AliL3HoughEval::DisplayEtaSlice : No input histogram!\n");
      return;
    }
  
  Double_t etaslice = (fEtaMax - fEtaMin)/fNEtaSegments;
  for(Int_t padrow = NRows[fPatch][0]; padrow <= NRows[fPatch][1]; padrow++)
    {
      Int_t prow = padrow - NRows[fPatch][0];
                  
      AliL3DigitRowData *tempPt = fRowPointers[prow];
      if(!tempPt) 
	{
	  printf("AliL3HoughEval::DisplayEtaSlice : Zero data pointer\n");
	  continue;
	}
      
      AliL3DigitData *digPt = tempPt->fDigitData;
      for(UInt_t j=0; j<tempPt->fNDigit; j++)
	{
	  UChar_t pad = digPt[j].fPad;
	  UChar_t charge = digPt[j].fCharge;
	  UShort_t time = digPt[j].fTime;
	  if(charge < fHoughTransformer->GetThreshold()) continue;
	  Float_t xyz[3];
	  Int_t sector,row;
	  fTransform->Slice2Sector(fSlice,padrow,sector,row);
	  fTransform->Raw2Local(xyz,sector,row,pad,time);
	  Double_t eta = fTransform->GetEta(xyz);
	  Int_t pixel_index = (Int_t)(eta/etaslice);
	  if(pixel_index != eta_index) continue;
	  hist->Fill(xyz[0],xyz[1],charge);
	}
    }
  
}
