//
// Created by Stephane Zsoldos on 7/6/22.
//

#include <TH2D.h>

#include "ReconAnalysis.hh"

#include <SnD/RATData.hh>
#include <SnD/Multilateration.hh>
#include <SnD/Recon.hh>
#include <SnD/Map.hh>

#include <ROOT/Utils.hh>

ReconAnalysis::ReconAnalysis(const char *pdfname, const char *histname, const char* perpmthistname ,
							 const double &R, const double &HH,
							 int me, int a, int ms,
							 bool im, const char *mn,
							 bool iv,
							 const char *treename)
	: nMaxEvts(me), algo(a), max_seed(ms), ismap(im), mapname(mn), isverbose(iv){
  //
  hPDF = GetROOTObj<TH2D>(pdfname, histname)->ProjectionX("hPDF");
  std::cout << "Load PDF: " << hPDF->GetName() << std::endl;
  mPDF2D = GetROOTMObj<TH2D>(pdfname, perpmthistname, "TH2D");
  std::transform(
	  mPDF2D.begin(), mPDF2D.end(),
	  std::inserter(mPDF1D, mPDF1D.begin()),
	  [](const std::pair<int, TH2D*>& p){
		return std::make_pair(p.first, p.second->ProjectionX());
	  }
  );
  //
  Cyl = new Cylinder(R, HH);
  //
  Tree = new TTree(treename, treename);
  RT.SetTree(Tree);
  //
  max_seed = max_seed < 0 ? std::numeric_limits<int>::max() : max_seed;
}
ReconAnalysis::~ReconAnalysis(){
  delete Tree;
  delete Cyl;
  delete hPDF;
  for(auto& p : mPDF2D)
	delete p.second;
}

void ReconAnalysis::Do(void *Data) {

  // Get Data
  auto *RData = static_cast<RATData*>(Data);

  // Get centroid seed
  TVector3 Centroid = GetCentroid(RData->vHits);

  // Get time seed
  double TSeed = Cyl->GetTWall(Centroid);

  // Get SnD seeds
  std::vector<PosT> vSeeds = GetVPosTSeeds(RData->vHits, hPDF, Cyl, max_seed);
  vSeeds.emplace_back(Centroid, TSeed);

  // Recon
  FitStruct FS = {RData->vHits, hPDF};
  RT = Recon(&FS, Cyl, vSeeds, GetAlgo(algo), fPosT, {SetBounds, SetPars});
  if(isverbose)
	RT.Print();
  FitMapStruct FMS = {RData->vHits, mPDF1D};
  RT = Recon(&FMS, Cyl, vSeeds, GetAlgo(algo), fPosTPerPMT, {SetBounds, SetPars});
  if(isverbose)
	RT.Print();

  // Map
  if(ismap)
	SaveMap(RData->vHits, hPDF, Cyl, RData->tag.c_str(), mapname.c_str());

  // Fill
  Tree->Fill();
}

#include <TFile.h>
void ReconAnalysis::Export(const char *filename) const {
  TFile f(filename, "RECREATE");
  Tree->Write();
  f.Close();
}