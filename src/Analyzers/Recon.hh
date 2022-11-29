//
// Created by Stephane Zsoldos on 7/6/22.
//

#ifndef SND_SRC_SND_RECONANALYSIS_HH_
#define SND_SRC_SND_RECONANALYSIS_HH_

#include <map>

#include <Templates/TAnalysis.hh>
#include "SnD/Geom.hh"
#include "SnD/PosT.hh"

#include <TH1D.h>
#include <TH2D.h>
#include <TTree.h>

class ReconAnalysis : public TAnalysis {
 public:
  TH1D* hPDF;
  std::map<int, TH2D*> mPDF2D;
  std::map<int, TH1D*> mPDF1D;

  Cylinder* Cyl;

  TTree* Tree;

  RecT RT;

  int nMaxEvts;
  int algo;
  int max_seed;
  bool ismap;
  std::string mapname;
  bool isverbose;

  bool isbinned;
  bool isunbinned;
  bool isperpmt;
  bool istrigtime;
 public:
  ReconAnalysis() = default;
  ReconAnalysis(const char *pdfname, const char *histname, const char* perpmthistname,
				const double &R, const double &HH,
				int me, int a, int ms,
				bool im,
				const char* mn,
				bool iv,
				bool ib, bool iu, bool ip,
				bool itt,
				const char *treename = "T");
  void Do(void *Data) override;
  void Export(const char* filename) const;
  ~ReconAnalysis();
};

#endif //SND_SRC_SND_RECONANALYSIS_HH_
