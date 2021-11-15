//
// Created by zsoldos on 1/5/21.
//

#ifndef _DEBUGRECON_HH_
#define _DEBUGRECON_HH_

// ####################################### //
// #### #### ####   C/C++   #### #### #### //
// ####################################### //
#include <iostream>
#include <string>

// ####################################### //
// #### #### ####   BOOST   #### #### #### //
// ####################################### //
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>

// ####################################### //
// #### #### ####   ROOT    #### #### #### //
// ####################################### //
#include <TApplication.h>
#include <TFile.h>

// ####################################### //
// #### #### ####   USER    #### #### #### //
// ####################################### //
#include <Utils.hh>
#include <Wrapper.hh>
#include "Output.hh"

std::vector<std::string> GetFilesInDir(const boost::filesystem::path& dir, const std::string& ext = ".root"){
  std::vector<std::string> vPaths;

  if(boost::filesystem::exists(dir) && boost::filesystem::is_directory(dir)){
    for(const auto& entry: boost::filesystem::recursive_directory_iterator(dir)){
      if (boost::filesystem::is_regular_file(entry) && entry.path().extension() == ext){
	vPaths.push_back(dir.string() + "/" + entry.path().filename().string());
      }
    }

  }

  return vPaths;

}

typedef struct Args{
  bool isVerbose = false;
  bool isDebug   = false;
  bool isDDebug   = false;

  std::vector<std::string> filename;
  std::string pdfname;
  std::string outname = "fRecon.root";

  unsigned int nEvts = 0;
  unsigned int wPower = 0;
  std::vector<double> bnds = {10.e3, 10.e3};
  bool isBox = false;
  unsigned int nThreads = 1;

  bool useGridSearch = false;

  bool isUnbinned = false;

  double cvg = 0.;

} Args;

static void ShowUsage(const std::string& name){

  std::cerr << "Usage: " << name << " <option(s)> -i (--input) IN.root -p (--pdf) PDF.root -o (--output) OUT.root" << std::endl
	    << "Options:\n"

	    << "\t-h\tShow this help message\n"
	    << "\t-v\tSet verbose mode (display progress bar)\n"

	    << "\t-d\tWrite debug plot into output file\n"
	    << "\t-dd\tAdditional debug info\n"

	    << "\t-n\tSet #Evts to process\n"

	    << "\t-w\tSet weight exponent power for Q kernel (default 0) \n"

	    << "\t-b <XX YY ZZ>\tSet boundaries for box geom (in mm) \n"
	    << "\t-c <R H>\tSet boundaries for cylinder geom (in mm) \n"

	    << "\t--nthreads\tSet nThread to run in parallel (default 1) \n"

	    << "\t-g\tGrid search seeding (default false) \n"
	    << "\t-u\tUnbinned fit (default false) \n"

	    << "\t--dir\tRead all .root files in directory \n"

	    << "\t--cvg\tSet new cvg [default 0] \n"

	    << std::endl;

}

static void ProcessArgs(TApplication &theApp,
			Args &args) {

  // Reading user input parameters
  if (theApp.Argc() < 2) {
    ShowUsage(theApp.Argv(0));
    exit(0);
  }

  for (int i = 1; i < theApp.Argc(); i++) {
    std::string arg = theApp.Argv(i);
    if ((arg == "-h") || (arg == "--help")) {
      ShowUsage(theApp.Argv(0));
      exit(0);
    } else if (boost::iequals(arg, "-v")) {
      args.isVerbose=true;
    } else if (boost::iequals(arg, "-d")) {
      args.isDebug=true;
    } else if (boost::iequals(arg, "-dd")) {
      args.isDDebug=true;
    } else if (boost::iequals(arg, "-n")) {
      args.nEvts=std::stoi(theApp.Argv(++i));
    } else if (boost::iequals(arg, "-w")) {
      args.wPower=std::stoi(theApp.Argv(++i));

    } else if (boost::iequals(arg, "--cvg")) {
      args.cvg=std::stod(theApp.Argv(++i));

    } else if (boost::iequals(arg, "-g")) {
      args.useGridSearch=true;

    } else if (boost::iequals(arg, "-u")) {
      args.isUnbinned=true;

    } else if (boost::iequals(arg, "-b")) {
      args.bnds.resize(3);
      args.bnds[0] = (std::stod(theApp.Argv(++i)));
      args.bnds[1] = (std::stod(theApp.Argv(++i)));
      args.bnds[2] = (std::stod(theApp.Argv(++i)));
      std::cout << "Setting box geom boundaries" << std::endl;
      args.isBox = true;
    } else if (boost::iequals(arg, "-c")) {
      args.bnds.resize(2);
      args.bnds[0] = (std::stod(theApp.Argv(++i)));
      args.bnds[1] = (std::stod(theApp.Argv(++i)));
      std::cout << "Setting cylinder geom boundaries" << std::endl;
      args.isBox = false;
    } else if (boost::iequals(arg, "--nthreads")) {
      args.nThreads=std::stoi(theApp.Argv(++i));

    } else if (boost::iequals(arg,"-i") || boost::iequals(arg,"--input")) {
      args.filename.emplace_back(theApp.Argv(++i));
    } else if (boost::iequals(arg,"--dir")) {
      auto v = GetFilesInDir(theApp.Argv(++i));
      std::merge(args.filename.begin(), args.filename.end(), v.begin(), v.end(), std::back_inserter(args.filename));
    } else if (boost::iequals(arg,"-p") || boost::iequals(arg,"--pdf")) {
      args.pdfname=theApp.Argv(++i);
    } else if (boost::iequals(arg,"-o") || boost::iequals(arg,"--output")) {
      args.outname=theApp.Argv(++i);
    } else {
      std::cout << "Unkown parameter" << std::endl;
      continue;
    }
  }

  auto RemoveEmptyFile = [](std::vector<std::string>& v){
    for(auto itFile=v.begin(); itFile!=v.end(); itFile++){
      if(!IsFileExist((*itFile).c_str())){
	v.erase(itFile);
      }
    }
  };

  RemoveEmptyFile(args.filename);

  if(args.filename.empty() || args.pdfname.empty()){
    std::cerr << "ERROR: No input file provided!" << std::endl;
    exit(EXIT_FAILURE);
  } else if (!IsFileExist(args.pdfname.c_str())){
    std::cerr << "ERROR: PDF doesn't exist!" << std::endl;
    exit(EXIT_FAILURE);
  }

}

template <typename T>
T* GetRootHisto(const char* filename, const char* histname){
  auto f = TFile::Open(filename);
  // Check if key exist
  if(!f->GetListOfKeys()->Contains(histname))
    return nullptr;
  auto hist = dynamic_cast<T *>(f->Get(histname)->Clone());
  hist->SetDirectory(nullptr);
  f->Close();
  delete f;
  return hist;
}

void LoadMCInfo2Evt(wRAT& w_rat, Event& evt){

  // IF USE SPLITEVDAQ
  // Get EV TrigTime
  const auto TrigTime = 0.;

  // Try to guess which particle is attached to this trigger
  // Make sense only for IBD gen, when n capture will be >> in T that prompt event
  // Note that also e+ is always first particle generated
  auto nParticle = w_rat.GetNPrimaryParticle();
  auto iParticle = nParticle > 1 ? (TrigTime > 1e3 ? 1 : 0) : 0;
  // Skip if it is not prompt
  // if(iParticle>0)
  // continue;

  // Get True info to record
  const auto PosTrue = w_rat.GetPosTrue(iParticle);
  const auto DirTrue = w_rat.GetDirTrue(iParticle);

  evt.MCPos = Vec(PosTrue);
  evt.MCDir = Vec(DirTrue);
  evt.MCT   = -TrigTime;
  evt.ETrue = w_rat.GetETrue(iParticle);

}

std::vector<double> GetTBndsLocal(const double& TSeed,
				  const std::vector<double>& vTBnds, const double& range = 3.){
  double TMin = TSeed - range > vTBnds[0] ? TSeed - range : vTBnds[0] ;
  double TMax = TSeed + range < vTBnds[1] ? TSeed + range : vTBnds[1] ;
  return {TMin, TMax};
}

void SlimVHits(std::vector<Hit>& vHOrig, const double ScaleFactor = 0.9){

  const auto nHitsScaled = static_cast<unsigned>( std::round(vHOrig.size() * (ScaleFactor / 0.9) ) );

  auto rng = std::default_random_engine {};
  std::shuffle(vHOrig.begin(), vHOrig.end(), rng);

  vHOrig.resize(nHitsScaled);

}

//the following are UBUNTU/LINUX, and MacOS ONLY terminal color codes.
#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */


void PrintDD(const std::string& name,
	     const TVector3& Pos, const double& T, const double& dWall,
	     const double& NLL = 0.){

  std::cout << "#### #### #### " << name << " #### #### ####" << std::endl;
  Pos.Print();
  std::cout << RED << T << "ns" << RESET << std::endl;
  std::cout << BLUE << dWall << "mm" << RESET << std::endl;
  if(NLL>0.)
    std::cout << GREEN << "NLL=" << NLL << RESET << std::endl;
  std::cout << "#### #### #### -------- #### #### ####" << std::endl;

}


#endif //_DEBUGRECON_HH_
