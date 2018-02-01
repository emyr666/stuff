// create a managed_mapped_file backed up to data.bin
// make it about 50GB in size. Can use ls -l and du to see the
// allocated size of the file and how much space it actually takes up
// on disk. should find that the allocated size is 50GB but the space taken
// up seems to be 8 bytes which presumably is some kind of structure to allow
// programs to hook in to the data in the file (if any!)

#include <iostream>

#include <boost/interprocess/managed_mapped_file.hpp>

namespace bip = boost::interprocess;

int main(int argc, char** argv) {

  if (argc != 2) {
    std::cerr << "Usage : map1 <db file>" << std::endl;
    return -1;
  }

  static bip::managed_mapped_file m_file(
    bip::open_or_create,
    argv[1],
    50ul*1024*1024*1024);

  return 0;
}
