//
// Created by Stephane Zsoldos on 8/2/22.
//

#ifndef SND_INCLUDE_SND_MAP_HH_
#define SND_INCLUDE_SND_MAP_HH_

#include <vector>

#include <TH1D.h>

#include <SnD/Hit.hh>
#include <SnD/Geom.hh>

void Map(const std::vector<Hit> &vHits, TH1D *hPDF, Bnd *b);

#endif //SND_INCLUDE_SND_MAP_HH_
