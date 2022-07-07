//
// Created by Stephane Zsoldos on 7/3/22.
//

#include "ReconAnalysis.hh"
#include "ReconApp.hh"
#include "../RATReader.hh"

int main(int argc, char **argv) {

  // ######################################## //
  // Read arguments
  ReconAppArgs Args;
  Args.ProcessArgs(argc, argv);

  // ######################################## //
  // Create ReconAnalysis
  ReconAnalysis Ana(nullptr, nullptr, "T");

  // ######################################## //
  // Run analysis
  RATReader R(Args.GetInput(), Args.GetVerbose());
  R.Read(&Ana);


  return EXIT_SUCCESS;
}