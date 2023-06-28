//
// Created by Stephane Zsoldos on 7/6/22.
//

#include "SnD/Hit.hh"
#include "LinAlg/Matrix.hh"
#include "LinAlg/SVD.hh"

#include <cmath>
#include <numeric>
#include <algorithm>

/**
 * @brief Compare two Hit objects based on their T values.
 *
 * This function compares two Hit objects based on their T values. It returns
 * true if the T value of the first Hit object is less than the T value of the
 * second Hit object.
 *
 * @param h1 The first Hit object to compare.
 * @param h2 The second Hit object to compare.
 * @return True if h1.T < h2.T, false otherwise.
 */
 bool operator<(const Hit& h1, const Hit& h2){
  return h1.T < h2.T;
}


/**
 * @brief Compare two Hit objects for equality based on their T values.
 *
 * This function compares two Hit objects for equality based on their T values.
 * It returns true if the T value of the first Hit object is equal to the T value
 * of the second Hit object.
 *
 * @param h1 The first Hit object to compare.
 * @param h2 The second Hit object to compare.
 * @return True if h1.T is equal to h2.T, false otherwise.
 */
bool operator==(const Hit& h1, const Hit& h2){
  return h1.T == h2.T;
}

/**
 * @brief Calculate the number of prompts in a vector of Hit objects.
 *
 * This function calculates the number of prompts in a vector of Hit objects,
 * based on a given time threshold T. It counts the number of Hit objects with
 * T values less than T and returns the count.
 *
 * @param vHits The vector of Hit objects to process.
 * @param T The time threshold to compare the Hit objects' T values against.
 * @return The number of prompts (Hit objects with T < T).
 */
double GetNPrompts(const std::vector<Hit>& vHits, const double& T){
  double NPrompts = 0.;
  for(auto& hit: vHits){
	if(hit.T<T){
	  NPrompts++;
	}
  }
  return NPrompts;
};


/**
 * @brief Calculate the weight of a Hit object based on its charge and power.
 *
 * This function calculates the weight of a Hit object based on its charge (`Q`)
 * raised to the power (`P`). It applies the power operation using the `std::pow`
 * function from the C++ standard library and returns the resulting value.
 *
 * @param h The Hit object for which to calculate the weight.
 * @param P The power to raise the Hit's charge to.
 * @return The weight of the Hit object based on the charge and power.
 */
double fWeight(const Hit& h, const int& P){
  return std::pow(h.Q, P);
}

// // Calculate centroid of a vector of hits
// vec3 GetCentroid(const std::vector<Hit>& vHits){
//   vec3 centroid(0., 0., 0.);
//   const auto NHits = static_cast<double>(vHits.size());
//   for(const auto& hit: vHits){
// 	centroid += hit.PMTPos * (1./NHits);
//   }
//   return centroid;
// }
//
// std::vector<double> GetResiduals(const vec3& Pos, const double& T, const std::vector<Hit>& vHits){
//   std::vector<double> vRes;
//   std::transform(
// 	  vHits.begin(),
// 	  vHits.end(),
// 	  std::back_inserter(vRes),
// 	  [&Pos, &T](const Hit& h) {
// 		return h.GetTRes(Pos, T);
// 	  }
//   );
//   return vRes;
// }
//
// double GetSum2Residuals(const vec3& Pos, const double& T, const std::vector<Hit>& vHits){
//   std::vector<double> vRes = GetResiduals(Pos, T, vHits);
//   return std::accumulate(
// 	  vRes.begin(),
// 	  vRes.end(),
// 	  0.0,
// 	  [](double a, double b) {
// 		return a + std::pow(b, 2);
// 	  }
//   );
// }
//
// vec3 GetMLAT(std::vector<Hit> vHits){
//
//   std::size_t nDim = 3;
//   std::size_t nEq  = vHits.size()-2;
//
//   // Get first and second hit from vHits
//   Hit H0 = vHits[0];
//   Hit H1 = vHits[1];
//
//   // Shift all hits by H0 position
//   std::transform(
// 	  vHits.begin(),
// 	  vHits.end(),
// 	  vHits.begin(),
// 	  [&H0](Hit& h){
// 		h.PMTPos -= H0.PMTPos;
// 		return h;
// 	  }
//   );
//
//   // Define lambdas
//   auto GetDT = [](const Hit& h0, const Hit& h1){
// 	return h1.T - h0.T;
//   };
//
//   auto GetTau = [&H0, &GetDT](const Hit& h){
// 	return GetDT(H0, h);
//   };
//
//   auto GetACoeff = [&GetTau, &H1](const Hit& h, const int& iDim){
// 	return (2*h.PMTPos[iDim] / (Csts::GetSoL()*GetTau(h)))
// 		- (2 * H1.PMTPos[iDim] / (Csts::GetSoL()*GetTau(H1)));
//   };
//
//   auto GetBCoeff = [&GetTau, &H1](const Hit& h){
// 	return Csts::GetSoL()*(GetTau(h) - GetTau(H1))
// 		- h.PMTPos.Mag2()/(Csts::GetSoL()*GetTau(h))
// 		+ H1.PMTPos.Mag2()/(Csts::GetSoL()*GetTau(H1));
//   };
//
//   Matrix A(nEq, nDim);
//   DiagMatrix B(nEq);
//   DiagMatrix X(nDim);
//
//   double QCut = 1e-3;
//
//   for(auto itH = vHits.begin()+2; itH != vHits.end(); itH++){
//
// 	auto iHit = std::distance(vHits.begin()+2, itH);
// 	auto QWeight = itH->Q;
//
// 	for(auto iDim=0;iDim<nDim;iDim++){
// 	  if(QWeight > QCut)
// 		A[iHit][iDim] = GetACoeff(*itH, iDim);
// 	  else
// 		A[iHit][iDim] = 0;
// 	}
//
// 	if(QWeight > QCut)
// 	  B[iHit]= -GetBCoeff(*itH);
// 	else
// 	  B[iHit]= 0;
//
//   }
//
//   try {
//
// 	SVD svd(A);
//
// 	svd.solve(B, X);
//
// 	return vec3(X[0], X[1], X[2]);
//
//   } catch ( const char* e) {
// 	// handle the exception
// 	throw std::exception(); // rethrow the exception
//   }
//
// }
//
// static double EvalNLL(double nObs, double nPred){
//   double L;
//   if(nObs>0 && nPred>0)
// 	L=nObs*log(nObs/nPred) + nPred-nObs;
//   else
// 	L=nPred;
//   return L;
// }
//
// double GetNLL(const TH1D& hPDF,
// 			  const vec3& Pos, const double& T, const std::vector<Hit>& vHits){
//   double NLL = 0.f;
//   // Generate a random name for the histogram
//   std::string hName = "hExp_" + std::to_string(rand());
//   TH1D hExp(hName.c_str(), "hExp", hPDF.GetNbinsX(), hPDF.GetXaxis()->GetXmin(), hPDF.GetXaxis()->GetXmax());
//   for (const Hit& hit: vHits){
// 	double TRes = hit.GetTRes(Pos, T);
// 	hExp.Fill(TRes);
//   }
//   for (int iBin=1; iBin<=hPDF.GetNbinsX(); iBin++){
// 	double nObs = hExp.GetBinContent(iBin);
// 	double nPred = hPDF.GetBinContent(iBin);
// 	NLL += EvalNLL(nObs, nPred);
//   }
//   return NLL;
//
// }
// double GetUNLL(const TH1D& hPDF,
// 			   const vec3& Pos, const double& T, const std::vector<Hit>& vHits){
//   double NLL = 0.f;
//   for (const Hit& hit: vHits){
// 	double TRes = hit.GetTRes(Pos, T);
// 	double P_TRes = hPDF.Interpolate(TRes);
// 	NLL += P_TRes <= 0.f ? static_cast<double>(vHits.size()) : -log(P_TRes/hPDF.Integral());
//   }
//   return NLL;
// }
//
// double GetMUNLL(const std::map<int, TH1D*>& mPDF,
// 				const vec3& Pos, const double& T, const std::vector<Hit>& vHits){
//   double NLL = 0.f;
//   for (const Hit& hit: vHits){
// 	if(!mPDF.at(hit.ID))
// 	  continue;
// 	double TRes = hit.GetTRes(Pos, T);
// 	double P_TRes = mPDF.at(hit.ID)->Interpolate(TRes);
// 	NLL += P_TRes <= 0.f ? static_cast<double>(vHits.size()) : -log(P_TRes/mPDF.at(hit.ID)->Integral());
//   }
//   return NLL;
//
// }
//
// double GetFirstHitTime(const std::vector<Hit>& vHits, const double& threshold){
//   double T = 0.;
//   for(const auto& hit: vHits){
// 	if(hit.Q>threshold){
// 	  T = hit.T;
// 	  break;
// 	}
//   }
//   return T;
// }
//
// double GetFirstHitTime(const std::vector<Hit>& vHits){
//   double T = std::numeric_limits<double>::max();
//   for(const auto& hit: vHits){
// 	if(hit.T<T){
// 	  T = hit.T;
// 	}
//   }
//   return T;
// }
//
// double GetWindowHitTime(const std::vector<Hit>& vHits, const double& threshold, const int& windowsize){
//   double T = 0.;
//   for(int i=0; i<vHits.size()-windowsize; i++){
// 	double Q = 0.;
// 	for(int j=0; j<windowsize; j++) {
// 	  Q += vHits[i + j].Q;
// 	}
// 	if(Q>threshold){
// 	  T = vHits[i].T;
// 	  break;
// 	}
//   }
//   return T;
// }
//
// double GetMaxHitTime(const std::vector<Hit>& vHits){
//   double T = 0.;
//   double Q = 0.;
//   for(const auto& hit: vHits){
// 	if(hit.Q>Q){
// 	  T = hit.T;
// 	  Q = hit.Q;
// 	}
//   }
//   return T;
// }
//
// std::vector<Hit> RandomSubset(const std::vector<Hit>& vHits, const int& k){
//   std::vector<Hit> vHitsCopy = vHits;
//   std::random_shuffle(vHitsCopy.begin(), vHitsCopy.end());
//   vHitsCopy.erase(vHitsCopy.begin()+k, vHitsCopy.end());
//   return vHitsCopy;
// }
//
// void SortHitsFromPos(std::vector<Hit>& vHits, const vec3& Pos){
//   std::sort(vHits.begin(), vHits.end(), [Pos](const Hit& h1, const Hit& h2){
// 	return h1.GetD(Pos) < h2.GetD(Pos);
//   });
// }
//
// std::vector<double> GetTs(const std::vector<Hit>& vHits){
//   return GetVector<double>(vHits, [](const Hit& h){return h.T;});
// }
//
// std::vector<double> GetQs(const std::vector<Hit>& vHits){
//   return GetVector<double>(vHits, [](const Hit& h){return h.Q;});
// }
//
// std::vector<double> GetDs(const std::vector<Hit>& vHits, const vec3& Pos){
//   std::vector<double> vD;
//   std::transform(
// 	  vHits.begin(),
// 	  vHits.end(),
// 	  std::back_inserter(vD),
// 	  [Pos](const Hit& h){
// 		return h.GetD(Pos);
// 	  }
//   );
//   return vD;
// }
//
// TH1D GetHTres(TH1D* hPDF,
// 			  const std::vector<Hit>& vHits, const vec3& Pos, const double& TTrig,
// 			  const double& SoL){
//   TH1D hTRes("hTRes", "hTRes",
// 			 hPDF->GetNbinsX(), hPDF->GetXaxis()->GetXmin(), hPDF->GetXaxis()->GetXmax());
//   for (const Hit& hit: vHits){
// 	double TRes = hit.GetTRes(Pos, TTrig, SoL);
// 	hTRes.Fill(TRes);
//   }
//   return hTRes;
// }
//
// std::unordered_map<double, std::vector<Hit>> GetSubsets(const std::vector<Hit>& vHits, const vec3& Pos,
// 														const double& bin_size){
//   std::unordered_map<double, std::vector<Hit>> mHits;
//   for (int i = 0; i < vHits.size(); i++) {
// 	for (int j = i + 1; j < vHits.size(); j++) {
// 	  double dx = vHits[j].T - vHits[i].T;
// 	  if (std::abs(dx) > 1e-6) {  // avoid division by zero
// 		double slope = (vHits[j].GetD(Pos) - vHits[i].GetD(Pos)) / dx;
// 		double slope_bin = std::round(slope / bin_size) * bin_size;
// 		if(slope_bin>0.f)
// 		  mHits[slope_bin].push_back(vHits[i]);
// 	  }
// 	}
//   }
//
//   // remove mHits with less than 2 hits
//   for(auto it = mHits.begin(); it != mHits.end();){
// 	if(it->second.size()<5){
// 	  it = mHits.erase(it);
// 	} else {
// 	  ++it;
// 	}
//   }
//
//   return mHits;
// }