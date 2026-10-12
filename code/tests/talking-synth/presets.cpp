#include "PresetFile.hpp"
#include <cassert>
#include <fstream>
#include <iterator>
#include <vector>
#include <cstdio>
using namespace kastle2::talking;
int main(int argc,char **argv) {
 assert(argc==2);std::ifstream file(argv[1],std::ios::binary);
 std::vector<uint8_t> bytes((std::istreambuf_iterator<char>(file)),{});assert(!bytes.empty());
 PresetFile presets;assert(presets.Load(bytes.data(),bytes.size()));
 assert(presets.num_banks==2 && presets.num_samples==2);
 const auto p=presets.GetPhrase(0,0);assert(!strcmp(p.text,"HELLO, WORLD!") && p.speed==120 && p.mouth==90 && p.throat==210 && p.sing==1);
 assert(presets.Color(1)==0x00ff80 && presets.Scale(0)==0xfff);
 assert(!presets.Load(bytes.data(),31));
 for(size_t i=0;i<bytes.size();++i) {
  auto damaged=bytes; damaged[i]^=0x80;
  // All payload bytes are protected by CRC. Header reserved bytes are ignored.
  if(i>=32 || i<19) assert(!presets.Load(damaged.data(),damaged.size()));
 }
 auto corrupt=bytes;corrupt[14]=33;assert(!presets.Load(corrupt.data(),corrupt.size()));
 puts("Preset tests passed: JS-generated binary parsed by firmware and corruption rejected");
}
