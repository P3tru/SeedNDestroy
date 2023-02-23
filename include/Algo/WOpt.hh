//
// Created by Stephane Zsoldos on 2/23/23.
//

#ifndef SND_INCLUDE_SND_WOPT_HH_
#define SND_INCLUDE_SND_WOPT_HH_

#include <vector>
#include <map>

#include <TH1D.h>
#include <TVector3.h>

#include "nlopt.hpp"

#include "SnD/Hit.hh"
#include "SnD/Geom.hh"
#include "SnD/ZVector.hh"
#include "SnD/PosT.hh"


double fLS(const std::vector<double> &x, std::vector<double> &grad, void *data);

typedef struct BaseFitStruct{
  std::vector<Hit> vHits;
  BaseFitStruct() = default;
  explicit BaseFitStruct(std::vector<Hit> vHits) : vHits(std::move(vHits)){}
} BaseFitStruct;

typedef struct FitStruct : public BaseFitStruct{
  TH1D *hPDF = nullptr;
  FitStruct() = default;
  FitStruct(const std::vector<Hit> &vHits, TH1D *hPDF) : BaseFitStruct{vHits}, hPDF{hPDF} {}
} FitStruct;

typedef struct FitMapStruct : public BaseFitStruct{
  std::map<int, TH1D*> mPDF;
  FitMapStruct() = default;
  FitMapStruct(const std::vector<Hit> &vHits, std::map<int, TH1D*> mPDF) : BaseFitStruct{vHits}, mPDF{std::move(mPDF)} {}
} FitMapStruct;

double fPosT(const std::vector<double> &x, std::vector<double> &grad, void *data);

double fPosTU(const std::vector<double> &x, std::vector<double> &grad, void *data);
double fPosTPerPMT(const std::vector<double> &x, std::vector<double> &grad, void *data);
double fLSC(const std::vector<double> &x, std::vector<double> &grad, void *data);

void SetBounds(nlopt::opt &opt, Bnd *c);

void SetPars(nlopt::opt &opt, Bnd *c);

void SetInequalityConstraint(nlopt::opt &opt, Bnd *c);

std::vector< RecT > DoRecon(nlopt::opt &opt, const std::vector< PosT > &vSeeds);
nlopt::algorithm GetAlgo(const int &a);
RecT Recon(void* data,
		   Bnd *c,
		   std::vector<PosT> &vSeeds,
		   nlopt::algorithm alg,
		   double(*fRec)(const std::vector<double> &x, std::vector<double> &grad, void *data),
		   const std::vector<void (*)(nlopt::opt &opt, Bnd *c)>& vSetPars);

#endif //SND_INCLUDE_SND_WOPT_HH_
