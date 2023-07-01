//
// Created by Stephane Zsoldos on 2/23/23.
//

#include <future>
#include <numeric>

#include "Algo/WOpt.hh"

void BaseFitStruct::FillSliceIterateData(std::vector<double> *vIterX, std::vector<double> *vIterY, std::vector<double> *vIterZ,
										 std::vector<double> *vIterT,
										 std::vector<double> *vf, int *nIter){
  // Fill
  const int num_rows = static_cast<int>(iterData.vx.size()); // Get the number of rows
  const int num_cols = iterData.vx[0].size(); // Get the number of columns

  std::vector<std::vector<double>> sliced_vec(num_cols); // Create a vector of vectors for the sliced vectors

  for (int col = 0; col < num_cols; col++) {
	sliced_vec[col].resize(num_rows); // Resize each inner vector to the number of rows

	// Use std::transform to create the sliced vector
	std::transform(iterData.vx.begin(), iterData.vx.end(), sliced_vec[col].begin(),
				   [col](const std::vector<double>& inner_vec) { return inner_vec[col]; });
  }

  *vIterX = sliced_vec[0];
  *vIterY = sliced_vec[1];
  *vIterZ = sliced_vec[2];
  *vIterT = sliced_vec[3];
  *vf = iterData.vf;
  *nIter = iterData.iter;
}

std::vector<std::vector<double>> CreateGridSamplePts(const double& scale){
  // create the array of 4-D vectors
  const int num_vectors = 81;
  std::vector<std::vector<double>> vectors(num_vectors, std::vector<double>(4));

  // fill the array with all possible combinations of 1, 0, and -1 for each coordinate
  std::generate(vectors.begin(), vectors.end(), [scale]() {
	static int n = 0;
	std::vector<double> v(4);
	int t = n++;
	for (int i = 0; i < 4; i++) {
	  v[i] = (t % 3) - 1;
	  v[i] *= scale;
	  t /= 3;
	}
	return v;
  });

  return vectors;
};

void Loop(const TH1D& hPDF,
		  const std::vector<Hit>& vHits,
		  const std::vector<Vector3>& vPts, const std::vector<double> &vT,
		  std::vector<double>& vNLL,
		  int startIndex, int endIndex){
  //
  for (int i = startIndex; i < endIndex; i++) {
	vNLL[i] = GetUNLL(hPDF, vPts[i], vT[i], vHits);
  }
  //
}

double Walk(const std::vector<double> &x, std::vector<double> &grad, void *data){

  // Unpack the data
  auto *fs = static_cast<FitStruct*>(data);
  TH1D hPDF = *fs->hPDF;
  std::vector<Hit> vHits = fs->vHits;
  const int nHits = static_cast<int>(vHits.size());
  std::vector<std::vector<double>> vGridSamplePts = fs->vGridSamplePts;
  const int nPts = static_cast<int>(vGridSamplePts.size());
  //
  Vector3 v(x[0], x[1], x[2], SpaceUnit::dm);
  double TGuess = x[3];
  // Create the vector of points
  std::vector<Vector3> vPts(nPts);
  std::vector<double> vT(nPts, 0.f);
  for (int i = 0; i < nPts; i++) {
	vPts[i] = v + Vector3(vGridSamplePts[i][0],
						  vGridSamplePts[i][1],
						  vGridSamplePts[i][2],
						  SpaceUnit::mm);
	vT[i] = TGuess + vGridSamplePts[i][3];
  }
  //
  std::vector<double> vNLL(nPts, 0.f);

  //
  const int numThreads = 1;
  // const int numThreads = static_cast<int>(std::thread::hardware_concurrency());
  const int numElementsPerThread = nPts / numThreads;
  //
  std::vector<std::future<void>> futures(numThreads);
  for (int i = 0; i < numThreads; i++) {
	//
	int startIndex = i * numElementsPerThread;
	int endIndex = (i + 1) * numElementsPerThread;
	//
	futures[i] = std::async(
		std::launch::async, Loop,
		std::ref(hPDF),
		std::cref(vHits),
		std::cref(vPts), std::cref(vT),
		std::ref(vNLL),
		startIndex, endIndex
	);
  }
  for (int i = 0; i < numThreads; i++) {
	futures[i].wait();
  }
  // return the sum of the NLL
  return std::accumulate(vNLL.begin(), vNLL.end(), 0.0) / nHits / nPts;

}

void FillIterData(IterationData &iterData, const std::vector<double> &x, double f){
  iterData.iter++;
  iterData.vx.push_back(x);
  iterData.vf.push_back(f);
}

double fPosT(const std::vector<double> &x, std::vector<double> &grad, void *data){
  //
  auto d = static_cast<FitStruct*>(data);
  auto norm = static_cast<double>(d->vHits.size());
  auto hPDF = *d->hPDF;
  if(d->isscaled)
	hPDF.Scale(norm);
  auto f = d->fNLL;
  // Create object to calculate TRes histogram
  Vector3 v(x[0], x[1], x[2], SpaceUnit::dm);
  double TGuess = x[3];
  // Calculate NLL
  double NLL = f(hPDF, v, TGuess, d->vHits) / norm;
  if(d->filldata)
	FillIterData(d->iterData, x, NLL);
  return NLL;
}

double fPosTPerPMT(const std::vector<double> &x, std::vector<double> &grad, void *data){
  //
  auto d = static_cast<FitMapStruct*>(data);
  auto norm = static_cast<double>(d->vHits.size());
  // Create object to calculate TRes histogram
  Vector3 v(x[0], x[1], x[2], SpaceUnit::dm);
  double TGuess = x[3];
  // Calculate NLL
  double NLL = 0.f;
  NLL =  GetMUNLL(d->mPDF, v, TGuess, d->vHits) / norm;
  if(d->filldata)
	FillIterData(d->iterData, x, NLL);
  return NLL;
}

double fLSC(const std::vector<double> &x, std::vector<double> &grad, void *data){
  auto d = static_cast<CylEdges *>(data);

  Vector3 v(x[0], x[1], x[2], SpaceUnit::dm);
  double TWall = d->GetTWall(v);

  // Inequality constraint
  return std::abs(TWall - x[3]) - 1;
}

void SetBounds(nlopt::opt &opt, CylEdges *c){
  double factor = Vector3::GetConversionFactor(c->unit) / Vector3::GetConversionFactor(SpaceUnit::dm);
  // Minimizer bounds
  std::vector<double> lb = {
	  -c->radius * factor, -c->radius * factor, -c->halfheight * factor , -1.f
  };
  std::vector<double> ub = {
	  c->radius * factor, c->radius * factor, c->halfheight * factor, c->T+1.f
  };
  // Set boundaries
  opt.set_lower_bounds(lb);
  opt.set_upper_bounds(ub);
}

void SetPars(nlopt::opt &opt, CylEdges *c){
  // Set stopping criteria
  opt.set_xtol_rel(1.e-18);
  opt.set_ftol_rel(1.e-18);
  // Set limits
  opt.set_maxtime(1./*sec*/);
}

void SetInequalityConstraint(nlopt::opt &opt, CylEdges *c){
  opt.add_inequality_constraint(fLSC, c, 1.e-8);
}

std::vector< RecCoord > DoRecon(nlopt::opt &opt, const std::vector< Coord > &vSeeds){
  //
  nlopt::result result;
  // Create return object
  std::vector< RecCoord > vResults;
  // Loop over seeds and recon
  std::transform(
	  vSeeds.begin(), vSeeds.end(),
	  std::back_inserter(vResults),
	  [&](const Coord &s) {
		double minf = std::numeric_limits<double>::max();
		std::vector<double> x = {
			s.Get(SpaceUnit::dm).GetX(),
			s.Get(SpaceUnit::dm).GetY(),
			s.Get(SpaceUnit::dm).GetZ(),
			s.GetT()
		};
		try{
		  result = opt.optimize(x, minf);
		} catch (std::exception &e) {
		  std::cout << "nlopt failed: " << e.what() << std::endl;
		}
		return RecCoord(x[0], x[1], x[2], SpaceUnit::dm, x[3], minf);
	  }
  );
  // Sort seeds by min NLL value
  std::sort(vResults.begin(), vResults.end(),
			[](const RecCoord &a, const RecCoord &b) {
			  return a.GetNLL() < b.GetNLL();
			}
  );
  // Return results
  return vResults;
}

nlopt::algorithm GetAlgo(const int &a){
  switch (a) {
	case 0:
	  return nlopt::LN_NELDERMEAD;
	  break;
	case 1:
	  return nlopt::LN_BOBYQA;
	  break;
	case 2:
	  return nlopt::LN_COBYLA;
	  break;
	case 3:
	  return nlopt::LN_NEWUOA;
	  break;
	case 4:
	  return nlopt::LN_PRAXIS;
	  break;
	case 5:
	  return nlopt::LN_SBPLX;
	  break;
	default:
	  return nlopt::LN_SBPLX;
	  break;
  }
}

RecCoord Recon(void* data,
			   CylEdges *c,
			   const std::vector<Coord> &vSeeds,
			   nlopt::algorithm alg,
			   double(*fRec)(const std::vector<double> &x, std::vector<double> &grad, void *data),
			   const std::vector<void (*)(nlopt::opt &opt, CylEdges *c)>& vSetPars){
  //
  // Create minimizer obj
  nlopt::opt opt(alg, 4);
  opt.set_min_objective(fRec, data);
  //
  for(auto& fSet: vSetPars)
	fSet(opt, c);
  //
  return DoRecon(opt, vSeeds).front();
}

