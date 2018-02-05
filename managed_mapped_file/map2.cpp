// create a managed_mapped_file backed up to data.bin
// make it about 50GB in size.
// put random words from a wordlist into it and print it

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/move/move.hpp>

// namesaces, typedefs etc.
namespace bip = boost::interprocess;
typedef bip::managed_mapped_file::segment_manager SegmentManager;
typedef bip::allocator<char, SegmentManager> CharAllocator;
typedef bip::basic_string<char, std::char_traits<char>, CharAllocator> SharedString;
typedef bip::allocator<SharedString, SegmentManager> SharedStringAllocator;
typedef bip::vector<SharedString, SharedStringAllocator> SharedStringVector;

// global wordlist
std::vector<std::string> wordlist;

/////////////////////////////////////////////////////////////////////////////
template<class T = std::mt19937, std::size_t N = T::state_size>
auto ProperlySeededRandomEngine () -> typename std::enable_if<!!N, T>::type {
/////////////////////////////////////////////////////////////////////////////
// seed a mersenne twister
    typename T::result_type random_data[N];
    std::random_device source;
    std::generate(std::begin(random_data), std::end(random_data), std::ref(source));
    std::seed_seq seeds(std::begin(random_data), std::end(random_data));
    T seededEngine (seeds);
    return seededEngine;
}

// global random number stuff
auto mt=ProperlySeededRandomEngine();
std::uniform_real_distribution<> U(0.0, 1.0);

///////////////////////////////////////
void create_wordlist(char* word_file) {
///////////////////////////////////////
// create the wordlist just want 4-7 letter words and ones that don't contain an apostrophe
  std::string apostrophe="'";
  std::ifstream ifs(word_file);
  if (ifs.is_open()) {
    std::string line;
    while ( ifs.good() ) {
      std::getline (ifs,line);
      std::transform(
        line.begin(),
        line.end(),
        line.begin(),
        ::tolower);
        if ((3 < line.size()) && (line.size() < 8)) {
          auto found=line.find(apostrophe);
          if (found == std::string::npos) {
            wordlist.push_back(line);
          }
        }
    }
  }
  ifs.close();
}

////////////////////////////////
std::string& get_random_word() {
////////////////////////////////
// get a random word from the wordlist

  unsigned int idx=static_cast<unsigned int>(U(mt)*wordlist.size());
  return wordlist[idx];
}

/////////////////////////////////
int main(int argc, char** argv) {
/////////////////////////////////

  if (argc != 3) {
    std::cerr << "Usage : map2 <wordlist file>  <db file>" << std::endl;
    return -1;
  }

  create_wordlist(argv[1]);

  // sort out the mapped file
  static bip::managed_mapped_file m_file(
    bip::open_or_create,
    argv[2],
    50ul*1024*1024*1024);

  // create allocator instances
  CharAllocator charAllocator(m_file.get_segment_manager());
  SharedStringAllocator sharedStringAllocator(m_file.get_segment_manager());

  // look for shared string vector called "DATA" in mapped file, create if not there
  // so first time program is run it will just create the vector and populate it with one word.
  // subsequent invocations of the program will apend one word to the list.
  SharedStringVector *data = m_file.find_or_construct<SharedStringVector>("DATA")(sharedStringAllocator);

  // append random word to the data vector, use move semantics to avoid a second deep copy
  // of the word when pushing it into the vector i.e. we have to do a copy when constructing the
  // word in shared memory as a copy of the word in the wordlist in main memory, but we don't do yet
  // another copy when putting that shared-memory word into the shared-memory vector
  std::string s=get_random_word();
  data->push_back(boost::move(SharedString(
    s.c_str(),
    s.size(),
    charAllocator)));

  // print out the data vector
  for (auto const& w : *data)
    std::cout << w <<std::endl;

  return 0;
}
