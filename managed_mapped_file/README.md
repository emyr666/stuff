Various programs to try out using the boost interprocess managed_mapped_file class.

map1
----
Create a managed_mapped_file db so we can see how much space is allocated and how much space is actually taken up on disk.

map2
----
Open or create a managed_mapped_file, look for a vector of strings with the name 'DATA'. Write a random word from a word list into the vector to extend it by one then print the vector. On each invocation should see it print more and more words, doing a du on the data file should see the space taken up on disk increase accordingly. Check with valgrind.
