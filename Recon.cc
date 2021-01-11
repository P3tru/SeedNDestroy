//
// Created by zsoldos on 1/5/21.
//

#include <iostream>
#include <string>
#include <vector>

#include <TROOT.h>
#include <TH1D.h>
#include <TH2D.h>

#include <Wrapper.hh>
#include <ProgressBar.hpp>
#include "include/Recon.hh"

#include "Recon.hh"
#include "DrawNLLSpace.hh"

int main(int argc, char *argv[]){

  // ######################################## //
  // Create TApp
  TApplication theApp("App", &argc, argv);
  gROOT->SetBatch(true);


  // ######################################## //
  // Parse arguments
  // Simple struct containing filename and verbosity level
  Args args;
  ProcessArgs(theApp, args);
  const bool isVerbose = args.isVerbose;
  const std::string input = args.filename;
  const std::string pdf = args.pdfname;
  const std::string output = args.outname;


  // ######################################## //
  // Load PDF
  auto hPDF = GetRootHisto<TH2D>(pdf.c_str(), "hCTVSTResPDF");
  TH1D *hPDF_TRes = hPDF->ProjectionX();


  // ######################################## //
  // Create wrapper object
  wRAT w_rat(input);
  const unsigned long int nEvts = args.nEvts > 0 ? args.nEvts : w_rat.GetNEvts();
  const unsigned int wPower = args.wPower;


  // ######################################## //
  // Create structure holding data
  DataStruct1D ds = {hPDF_TRes, std::vector<Hit>(), wPower};
  TTree tree("T", "A zara tree");
  Event evt;
  SetTTree(tree, evt);


  // ######################################## //
  // Handle gen shift of ANNIE
  auto ANNIEShift = [](const TVector3& v){
	TVector3 vShifted;
	vShifted.SetX(v.X());
	vShifted.SetY(-1*(v.Z()-1724));
	vShifted.SetZ(v.Y()+133.3);
	return vShifted;
  };
  auto ANNIEDirShift = [](const TVector3& v){
	TVector3 vShifted;
	vShifted.SetX(v.X());
	vShifted.SetY(-1*v.Z());
	vShifted.SetZ(v.Y());
	return vShifted;
  };

  // ANNIE Boundaries
  Bnds ANNIEBnds = {2.e3/*mm*/, 10./*ns*/};


  // ######################################## //
  // Loop and get vector of NHits
  ProgressBar progress_bar(nEvts, 70);
  for(auto iEvt=0; iEvt<nEvts; iEvt++){

	// Record the tick
	++progress_bar;

	// Point to evt
	w_rat.SetEvt(iEvt);

	// Get True info to build PDFs
	const auto PosTrue = ANNIEShift(w_rat.GetPosTrue(0));
	const auto DirTrue = ANNIEDirShift(w_rat.GetDirTrue(0));
	const auto TTrue = w_rat.GetTTrue(0);

	evt.MCPos = Vec(PosTrue);
	evt.MCDir = Vec(DirTrue);
	evt.MCT = TTrue;

	// Get number of trigger associated with an event
	// i.e, number of EV inside the rat DS
	auto nTriggers = w_rat.GetNTriggers();

	for(auto iTrigger=0; iTrigger<nTriggers; iTrigger++){

	  // Get vector of hits
	  std::vector<Hit> vHits = w_rat.GetVHits(iTrigger);

	  // DO STUFF
	  auto PosTSeed = GetSeed(vHits, hPDF_TRes, ANNIEBnds.Pos, wPower);

	  ds.vHits.clear();
	  ds.vHits = vHits;

	  auto x = ReconPosTime(ds, ANNIEBnds, PosTSeed.Pos, PosTSeed.T);

	  evt.RecPos = Vec(x);
	  evt.RecT = x[3]*1.e-2;

	  tree.Fill();

	  // ...

	}

	if(isVerbose)
	  progress_bar.display();

  }

  if(isVerbose)
	progress_bar.done();

  TFile fOut(output.c_str(), "RECREATE");
  tree.Write();
  fOut.Close();

  return EXIT_SUCCESS;
}