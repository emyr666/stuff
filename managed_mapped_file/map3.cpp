// create a managed_mapped_file backed up to data.bin
// make it about 50GB in size.

// create a tree object which has a pointer to its root node and an int saying how many nodes in the tree

// can call 'print' on a tree which should print it out using a lisp-style syntax
// e.g. ( root ( child 1 ( child 3) ) child 2 )

// a tree node is made up of..
// a word
// a vector of pointers to child nodes. note we need to use the boost interprocess offset_pointer
// so that pointers are valid across invocations

// first invocation just creates the tree and root node with no children
// subsequent iterations will choose a random node in the tree and add a child to that node
// with the 'word' part being set to a random word and the vector of children being empty

// will print the tree at each invocation

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <random>

#include <boost/interprocess/allocators/allocator.hpp>
#include <boost/interprocess/managed_mapped_file.hpp>
#include <boost/interprocess/containers/string.hpp>
#include <boost/interprocess/containers/vector.hpp>
#include <boost/interprocess/offset_ptr.hpp>
#include <boost/move/move.hpp>

// forward reference of SharedTreeNode;
class SharedTreeNode {
  bip::offset_ptr<SharedTreeNode> 
};

// namesaces, typedefs etc.
namespace bip = boost::interprocess;
typedef bip::managed_mapped_file::segment_manager                      SegmentManager;
typedef bip::allocator<void, SegmentManager>                           VoidAllocator;
typedef bip::allocator<char, SegmentManager>                           CharAllocator;
typedef bip::basic_string<char, std::char_traits<char>, CharAllocator> SharedString;

template <typename T>
using Ptr = bip::offset_ptr<T>;

template <typename T>
using SharedPointerVector = bip::vector< Ptr<T>, bip::allocator<Ptr<T>, SegmentManager>;

// tree node class
class SharedTreeNode {
  SharedPointerVector<SharedTreeNode> children;
  SharedString name;

  SharedTreeNode(std::string s, CharAllocator a) {
  
  } 
};

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

  // look for data vector in mapped file, create if not there
  SharedStringVector *data = m_file.find_or_construct<SharedStringVector>("DATA")(sharedStringAllocator);

  // append random word to the data vector
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
