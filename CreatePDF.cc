//
// Created by zsoldos on 11/25/20.
//

#include "CreatePDF.hh"

#include "wRATter/include/Wrapper.hh"
#include "wRATter/include/Hit.hh"
#include "PathFit.hh"

#include <ProgressBar.hpp>

#include <TH1D.h>
#include <TH2D.h>
#include <TFile.h>

int CreatePDF(const char* filename, unsigned int nEvts=0,
			  const char* foutname="fout.root",
			  int wPower = 1, double(*fW)(const Hit&, int) = fweight){

  // #################################################### //
  // #### #### #### OPEN FILE / READ TTREE #### #### #### //
  // #################################################### //

  wRAT w_rat(filename);
  nEvts = nEvts > 0 ? nEvts : w_rat.GetNEvts();
  ProgressBar progressBar(nEvts, 70);

  // ######################################## //
  // #### #### #### HISTOGRAMS #### #### #### //
  // ######################################## //

  hPDF h_pdf;

  // ################################## //
  // #### #### #### LOOP #### #### #### //
  // ################################## //

  for(auto iEvt=0; iEvt<nEvts; iEvt++){

	++progressBar;

	w_rat.SetEvt(iEvt);

	auto nTriggers = w_rat.GetNTriggers();

	for (auto iTrigger = 0; iTrigger < nTriggers; iTrigger++) {

	  const auto TrigTime = w_rat.GetTriggerTime(0);

	  auto nParticle = w_rat.GetNPrimaryParticle();
	  auto iParticle = nParticle > 1 ? (TrigTime > 1e3 ? 1 : 0) : 0;

	  if(iParticle>0)
		continue;

	  if(iTrigger>0)
		continue;

	  const auto PosTrue = w_rat.GetPosTrue(iParticle);
	  const auto DirTrue = w_rat.GetDirTrue(iParticle);
	  const auto TTrue = w_rat.GetTTrue(iParticle);

	  auto vHits = w_rat.GetVHits(iTrigger);
	  std::sort(vHits.begin(), vHits.end());

	  for(auto& hit: vHits){

		const double TCor = TTrue - TrigTime;

		h_pdf.hTResVSCT->Fill(hit.GetTRes(PosTrue, TCor),
							  hit.GetCosTheta(PosTrue, DirTrue),
							  fweight(hit));


	  }

	}


	progressBar.display();

  }

  progressBar.done();

  // #################################### //
  // #### #### #### FINISH #### #### #### //
  // #################################### //

  h_pdf.hTResVSCT->Scale(1./static_cast<double>(nEvts));

  TFile fOut(foutname, "RECREATE");
  h_pdf.hTResVSCT->Write();
  h_pdf.hTResVSCT->ProjectionX()->Write();
  h_pdf.hTResVSCT->ProjectionY()->Write();
  fOut.Close();

  return EXIT_SUCCESS;

}
