//
// Created by zsoldos on 1/5/21.
//

#include <iostream>
#include <string>
#include <vector>

#include <TH1D.h>
#include <TH2D.h>

#include <Wrapper.hh>
#include <ProgressBar.hpp>

#include "include/Recon.hh"
#include "Centroid.hh"
#include "Multilateration.hh"
#include "TriggerTimeMap.hh"

#include "DebugRecon.hh"
#include "Output.hh"
#include "RATDSCopier.hh"

int main(int argc, char *argv[]){

  // ######################################## //
  // Create TApp
  TApplication theApp("App", &argc, argv);


  // ######################################## //
  // Parse arguments
  // Simple struct containing filename and verbosity level
  Args args;
  ProcessArgs(theApp, args);
  const bool isVerbose = args.isVerbose;
  const std::string pdf = args.pdfname;
  const std::string output = args.outname;


  // ######################################## //
  // Load PDF
  auto hPDF = GetRootHisto<TH2D>(pdf.c_str(), Form("hCTVSTResPDF_TTOF_QW%d", args.wPower));
  TH1D *hPDF_TRes = hPDF->ProjectionX();
  if(args.isVerbose)
    std::cout << "PDF LOADED: " << hPDF_TRes->GetName() << std::endl;


  // ######################################## //
  // Create wrapper object
  wRAT w_rat(args.filename);
  const unsigned long int nEvts = args.nEvts > 0 ? args.nEvts : w_rat.GetNEvts();
  const unsigned int wPower = args.wPower;
  if(args.isVerbose)
	std::cout << "INPUT FILE LOADED: " << args.filename.front() << std::endl;


  // ######################################## //
  // Create structure holding data
  DataStruct1D ds = {hPDF_TRes, wPower};


  // ######################################## //
  // DET Boundaries
  const std::vector<double> DetBnds = {args.bnds.x(), args.bnds.y(), args.bnds.z()};
  const std::vector<double> TBnds = {-10., args.bnds.Mag() / SOL};
  Bnds bnds = {DetBnds, TBnds};


  // ######################################## //
  // LOAD TTPDF
  TrigTimePDFLetterBox TimePDF;
  TimePDF.Load(pdf);
  if(args.isVerbose)
	std::cout << "TimePDF LOADED" << std::endl;

  // ######################################## //
  // OUTPUT Tree
  wRATCP wratcp(args.filename.front(), output, args.isVerbose);
  if(args.isVerbose)
	std::cout << "COPYING RATDS obj for output" << std::endl;

  // ######################################## //
  // Loop and get vector of NHits
  ProgressBar progress_bar(nEvts, 70);
  for(auto iEvt=0; iEvt<nEvts; iEvt++){

	// Record the tick
	++progress_bar;

	// Point to evt
	w_rat.SetEvt(iEvt);

	// Prepare copy obj
	wratcp.PointDSToEvt(iEvt);

	// Get number of trigger associated with an event
	// i.e, number of EV inside the rat DS
	auto nTriggers = w_rat.GetNTriggers();

	for(auto iTrig=0; iTrig<nTriggers; iTrig++){

	  // Prep copy
	  wratcp.ClearDS(iTrig);

	  // Get ID tag
	  const std::string tag = Form("Evt%dTrig%d", iEvt, iTrig);

	  // Get vector of hits
	  std::vector<Hit> vHits = w_rat.GetVHits(iTrig);
	  if(vHits.empty())
	    continue;
	  std::sort(vHits.begin(), vHits.end());

	  //
	  // DO STUFF
	  //

	  // Prep Recon
	  ds.vHits.clear();
	  ds.vHits = vHits;


	  //
	  // #### #### #### TIME SEEDING #### #### #### //
	  //

	  auto CentroidSeed = GetCentroidSeed(vHits, bnds, 2);
	  if(!bnds.IsIn(CentroidSeed))
		std::cerr << "NASTY Centroid seed" << std::endl;

	  std::vector<double> TBounds(2);
	  const double TSeed = TimePDF.GetTrigTime(CentroidSeed, TBounds);

	  //
	  // #### #### #### POS SEEDING #### #### #### //
	  //

	  const std::size_t MaxSeeds = 5;
	  std::vector<TVector3> vSeeds = GetVSeeds(vHits, hPDF_TRes, -TSeed, bnds, wPower, MaxSeeds);

	  //
	  // #### #### #### Create local boundaries for fit #### #### #### //
	  //

	  Bnds localbnds = {bnds.Pos, TBounds};

	  //
	  // #### #### #### Time to fit some sinsemilia #### #### #### //
	  //

	  std::vector< std::vector<double> > vX;
	  vX.reserve(vSeeds.size());

	  for(auto &Seed: vSeeds){

		// Prep Recon
		ds.Reset();

		// Recon
		// X = {XRec, YRec, ZRec, TRec, NLL, NLOPT::Results}
		auto x = ReconPosTime(ds, localbnds, Seed, -TSeed);
		vX.emplace_back(x);

	  }

	  //
	  // #### #### #### Sort fits by NLL #### #### #### //
	  //

	  std::sort(vX.begin(), vX.end(), [](const std::vector<double>& v1, const std::vector<double>& v2){
		return v1[4] < v2[4];
	  });

	  wratcp.FillDS(iTrig, vX.front());


	  //
	  // ...
	  //

	}

	wratcp.FillTree();

	if(isVerbose)
	  progress_bar.display();

  }

  if(isVerbose)
	progress_bar.done();

  wratcp.Write();

  return EXIT_SUCCESS;
}
