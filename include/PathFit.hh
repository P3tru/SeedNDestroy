//
// Created by zsoldos on 9/15/20.
//

#ifndef _PATHFIT_HH_
#define _PATHFIT_HH_

#include <TH1D.h>
#include <TH2D.h>

#include <Hit.hh>
#include <TRandom3.h>
#include "MathUtils.hh"
#include "Recorder.hh"
#include "NLL.hh"

typedef struct DataStruct{
  std::vector<Hit> vHits;
  unsigned int wPower;
  explicit DataStruct(unsigned int w_power)
	  : wPower(w_power) {}
} DataStruct;

typedef struct DataStruct1D : public DataStruct, Recorder {
  TH1D* hPDF;
  DataStruct1D(TH1D *h_pdf, unsigned int w_power)
	  : hPDF(h_pdf), DataStruct(w_power), Recorder() {}
} DataStruct1D;

typedef struct DataStructDir : public DataStruct1D {
  std::vector<double> PosTGuess;
} DataStructDir;

typedef struct DataStruct2D : public DataStruct {
  TH2D* hPDF;
  DataStruct2D(TH2D *h_pdf, unsigned int w_power)
	  : hPDF(h_pdf), DataStruct(w_power) {}
} DataStruct2D;


double fPosTDir(const std::vector<double> &x, std::vector<double> &grad, void *data){
  auto d = static_cast<DataStruct2D*>(data);

  // Create object to calculate TRes histogram
  TVector3 PosGuess(x[0], x[1], x[2]);
  TVector3 DirGuess(x[3], x[4], x[5]);
  double TGuess = x[6]*1.e-2;

  return GetNLL(d->vHits, d->hPDF, PosGuess, TGuess, DirGuess.Unit(), fweight, d->wPower);

}

static std::vector<TVector3> GetVSpml(const TVector3& orig = TVector3(0.,0.,0.),
									  const double& radius = 10. /*mm*/,
									  const unsigned& nPts = 10){

  std::vector<TVector3> vSmpl(nPts);

  TRandom3 r(0);

  for(auto& v:vSmpl){
	v = TVector3(r.Gaus(), r.Gaus(), r.Gaus());
	v.SetMag(r.Uniform(radius));
  }

  return vSmpl;

}

double fPosTSmear(const std::vector<double> &x, std::vector<double> &grad, void *data){
  auto d = static_cast<DataStruct1D*>(data);

  // Create object to calculate TRes histogram
  std::vector<TVector3> vPosGuess = GetVSpml(TVector3(x[0], x[1], x[2]));
  double TGuess = x[3]*1.e-2;

  // Calculate NLL
  double NLL = 0.;
  for(const auto& PosGuess:vPosGuess)
    NLL += GetNLL(d->vHits, d->hPDF, PosGuess, TGuess, fweight, d->wPower) / (double)(vPosGuess.size());

  // Record
  d->iCall++;
  d->vPosGuess.emplace_back(TVector3(x[0], x[1], x[2]));
  d->vTGuess.emplace_back(TGuess);
  d->vNLL.emplace_back(NLL);

  return NLL;

}


const double PosScale = 1.e-1;
const double TScale   = 1.e1;

typedef struct DetParams {
  double radius;
  double hheight;
  double A;
} DetParams;

double fPosTC(const std::vector<double> &x, std::vector<double> &grad, void *data) {
  auto d = static_cast<DetParams *>(data);

  TVector3 PosGuess(x[0] / PosScale, x[1] / PosScale, x[2] / PosScale);
  double TGuess = x[3] / TScale;
  double dWall = GetDWall(PosGuess, d->radius, d->hheight);

  return -TGuess - dWall*d->A;

}

double fPosT(const std::vector<double> &x, std::vector<double> &grad, void *data){
  auto d = static_cast<DataStruct1D*>(data);

  // Create object to calculate TRes histogram
  TVector3 PosGuess(x[0] / PosScale, x[1] / PosScale, x[2] / PosScale);
  double TGuess = x[3] / TScale;

  // Calculate NLL
  double NLL = GetNLL(d->vHits, d->hPDF, PosGuess, TGuess, fweight, d->wPower);

  // Record
  d->iCall++;
  d->vPosGuess.emplace_back(PosGuess);
  d->vTGuess.emplace_back(TGuess);
  d->vNLL.emplace_back(NLL);

  return NLL;

}


#endif //_PATHFIT_HH_
