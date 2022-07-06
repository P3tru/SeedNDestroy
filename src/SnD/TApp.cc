//
// Created by Stephane Zsoldos on 7/3/22.
//

#include "TApp.hh"
#include "RATReader.hh"
#include "PDFAnalysis.hh"

int main(int argc, char **argv) {

  // ######################################## //
  // Read arguments
  TAppArgs Args;
  Args.ProcessArgs(argc, argv);

  // ######################################## //
  // Create analysis class
  Analysis Ana(Args.GetTResBins()[0], Args.GetTResBins()[1], Args.GetTResBins()[2]);

  // ######################################## //
  // Run analysis
  RATReader R(Args.GetInput(), Args.GetVerbose());
  R.Read(&Ana);

  // ######################################## //
  // Export results
  Ana.Export(Args.GetOutput());

  return EXIT_SUCCESS;
}