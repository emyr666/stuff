// create a managed_mapped_file backed up to data.bin
// make it about 50GB in size.
// put random words from a wordlist into it and print it

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>

#include <functional>
#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>
#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/move/move.hpp>

// namesaces, typedefs etc.
namespace bip = boost::interprocess;
typedef bip::managed_mapped_file::segment_manager SegmentManager;
typedef std::string KeyType;
typedef std::string MappedType;
typedef std::pair<KeyType, MappedType> ValueType;
typedef bip::allocator<ValueType, SegmentManager> ValueTypeAllocator;
typedef boost::unordered_map < KeyType, MappedType, boost::hash<KeyType>,std::equal_to<KeyType>, ValueTypeAllocator> StringHashMap;

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

  //Construct a mapped file hash map.
  //Note that the first parameter is the initial bucket count and
  //after that, the hash function, the equality function and the allocator
  StringHashMap *data = m_file.find_or_construct<StringHashMap>("DATA")
    (3, boost::hash<KeyType>(), std::equal_to<KeyType>()
         , m_file.get_allocator<ValueType>());

  // insert data into the hash map
  std::string s1=get_random_word();
  std::string s2=get_random_word();
  data->insert(ValueType(s1,s2));

  // print out the data map
  for (auto const& w : *data)
    std::cout << w.first << " : " << w.second << std::endl;

  return 0;
}
