Carter Rows - 1170615
October 13, 2023

IMPLEMENTATION:

- Insertion works as expected. Only thing I have noticed in testing is that you must set the table size to a
sufficient size so every key can be inserted, orelse the program quits.
- Searching works as expected. One thing to note, if we search for a key that isn't in the table, the cost
of the search increases depending on which probing algorithm is used. For example, if we probe with linear, and
try to search for a key that isn't in the table, we will have to probe through every table cell before realizing
it is not in the table. Quadratic and double hash probing will realize it is not in the table much faster.
- Deletion works as expected. The same thing applies here with the probing costs for keys that are not in the table.

VALGRIND:

- I've run valgrind for insertion only, deletion, and searching. All dynamically allocated blocks are freed, and
I have no leaks.

hash_CARTER:

My hashing algorithm I implemented is an adapted version of the Fowler-Noll-Vo hashing function.

This algorithm starts by setting our return HashIndex as an offset value defined as 0x811c9dc5. This is the offset value for the 32 bit implementation of the fnv hashing algorithm. This value helps with the distribution of hash indexes provided for different keys. 

Then, for every byte in the AAKeyType key, we XOR it with the current HashIndex, and multiply it with the FNV prime value defined as 2^24 + 2^8 + 0x93 0x01000193, again using the 32 bit prime value for the FNV function. Then once we have iterated through every byte in the key, we simply return our HashIndex % size_of_array. This ensures every index returned is inside the boundary of allowed return indexes.

I have experimented with using the 64 bit FNV prime and offset values, and found that for this specific use case, the 32 bit offset and prime hex values lower the insertion cost of the program. 

Here is a comparison of the three hashing algorithms used in this assignment, with different table sizes:

Size = 250:
- hash_CARTER cost = 144
- hashByLength cost = 5662
- hashBySum cost = 157

Size = 500:
- hash_CARTER cost = 121
- hashByLength cost = 5662
- hashBySum cost = 136

Size = 1000:
- hash_CARTER cost = 122
- hashByLength cost = 5662
- hashBySum cost = 131

As seen above, my hashing algorithm performs better than hashBySum and hashByLength for all three table sizes. The distribution of keys seems to be uniform and has minimal clustering using my hash function, for all three table sizes. My algorithm reduces collisions, reducing the need for probing, and speeds up the program as a result.

hash_CARTER meets the must have requirements of a hashing algorithm:
- For a given key, the hash-CARTER will return the same index every time
- The return indexes for different keys are uniformally distributed throughout any hash table size I used.

hash_CARTER meets the should have goals:
- For a small change in the input key, the index returned is significantly different, helping with uniform distribution.
- Uses all the space available in the table. Some keys are inserted at the end, others at the start, middle, etc.

hash_CARTER meets the ideally goals:
- Reduced clustering or hotspots in indexing. The offset value helps with this.
- Reduced collisions. As shown in the cost comparisons, my algorithm reduces collisions and insert costs.

REFERENCES:

To help create my hashing algorithm I used the following:

http://www.isthe.com/chongo/tech/comp/fnv/index.html
