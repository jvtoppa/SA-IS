# SA-IS
My implementation of the Suffix Array construction through induction sorting. Also added an algorithm for calculating the Burrows-Wheeler Transform after the creation of the Suffix Array, if necessary.

I tested this algorithm for construction of the BWT + RLE + Huffman Coding and the results were favorable. For very repetitive texts I was able to compress the text > 90%. Obviously, the cost of building the SA is big. It's not an algorithm that favors locality, so it's going to be slow independently of the implementation. Also, SA-IS is O(nlogn) which makes the total pipeline to be O(nlogn). I tested with multiple different files and the bottleneck is usually constructing the SA, so maybe there are some optimizations that could be made. 

I didn't have the time to properly polish this script yet, so there are some clear optimizations that this algo will benefit from.

Alas, it works. Feel free to use this implementation however you want.

Based on Nong et al.
