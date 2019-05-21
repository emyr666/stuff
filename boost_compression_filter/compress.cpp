#include <boost/iostreams/filter/lzma.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/traits.hpp>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/copy.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <regex>
#include <random>

namespace io = boost::iostreams;

int main() {

  // list to hold words
  std::vector<std::string> word_list;

  // set up for regex matching
  std::regex re("^[a-z]+$");
  std::smatch match;

  // open words file
  std::ifstream infile("words");

  // read words line by line 
  // put words that are 3-8 letters long
  // and only contain lower case ascii
  // in the wordlist
  // last character will be the 
  std::string line;
  while (std::getline(infile, line)) {
    if (std::regex_search(line, match, re)) {
      if (line.length() >= 3 && line.length() <= 8) {
        word_list.push_back(line);
      }
    } 
  } 

  io::filtering_streambuf<io::output> out;
  out.push(io::lzma_compressor());
  io::file_sink ofs("output.xz");
  out.push(ofs);
  std::stringstream ss;
  io::copy(ss,out);

  // print 100 random words from the wordlist
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_int_distribution<int> dist(0, word_list.size()-1);

  for (int i=0; i<16; ++i) {
    ss << word_list[dist(mt)] << std::endl;
  }

  return 0;
}
