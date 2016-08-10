#include <stdint.h>

#include <iostream>
#include <list>
#include <string>
#include <regex>
#include <fstream>
#include <vector>
#include <chrono>
#include <algorithm>

uint32_t counter = 0;

using namespace std;

list<string> getGoodWords(const string& filename)
{
    list<string> goodWords;
    regex pattern("^[abcdef]*$");
    ifstream infile(filename);
    string line;
    while(getline(infile, line)) {
        if (line.length() > 2 && regex_match(line, pattern))
            goodWords.push_back(line);
    }
    goodWords.push_back("a");
    return goodWords;
}

// mian2zi3 sent me this amazing alternate implementation...
// He realized that the words themselves don't matter - only their lengths.
// So instead of a vector of pointers to the words, wordsByLength
// contains just counters - e.g. wordsByLength[3] = 18.
// (there are 18 words of length 3 - "bed", "bee", etc).
// He can then handle all 18 such words in one single stroke,
// by multiplying the result of solve after using any one of them
// by 18! (that's what the 't*solve' does below...)
//
// Brilliant - and of course, 1000s of times faster than recursing
// on the words themselves.
//
// As ever, algorithmic improvements can be far more important 
// than language optimizations :-)
//
// Kudos, mian2zi3 !
//
// Below, the check verifying the results:
//
//    $ for i in {3..14} ; do ../hexspeak.py $i abcdef ../words  ; done | grep -v ^$
//
//    18 in   0.023 ms
//    56 in   0.061 ms
//    48 in   0.080 ms
//    335 in   0.175 ms
//    1667 in   0.654 ms
//    2834 in   2.868 ms
//    7688 in   6.172 ms
//    40992 in  17.437 ms
//    103684 in  68.678 ms
//    222138 in 198.328 ms
//    946764 in 617.432 ms
//    3020796 in 1961.062 ms
//    
//    $ for i in {3..14} ; do ./bin.debug/hexspeak $i abcdef ../words  ; done | grep -v ^$
//
//    18 in 0.001959 ms.
//    56 in 0.002233 ms.
//    48 in 0.00229 ms.
//    335 in 0.003186 ms.
//    1667 in 0.005968 ms.
//    2834 in 0.005749 ms.
//    7688 in 0.005978 ms.
//    40992 in 0.035903 ms.
//    103684 in 0.018924 ms.
//    222138 in 0.022032 ms.
//    946764 in 0.056376 ms.
//    3020796 in 0.07887 ms.
//
int solve(
    vector<int>& wordsByLength,
    int targetLength,
    int phraseLength)
{
    int counter = 0;
    for(int i=1; i<targetLength-phraseLength+1; i++) {
        if (phraseLength+i == targetLength) {
            counter += wordsByLength[i];
        } else {
            int t = wordsByLength[i]--;
            counter += t * solve(wordsByLength, targetLength, phraseLength+i);
            wordsByLength[i]++;
        }
    }
    return counter;
}

int main(int argc, char *argv[])
{
    const string targetLengthStr = (argc<2)?"14":argv[1];
    const uint32_t targetLength = atoi(targetLengthStr.c_str());
    const string letters = (argc<3)?"abcdef":argv[2];
    const string wordsFilename = (argc<4)?"../../words":argv[3];
    auto goodWords = getGoodWords(wordsFilename);
    vector<int> wordsByLength(128);
    for(const auto& word: goodWords)
        wordsByLength[word.length()]++;
    auto startTime = chrono::steady_clock::now();
    counter = solve(wordsByLength, targetLength, 0);
    auto endTime = chrono::steady_clock::now();
    cout << counter << " in " << chrono::duration<double, milli>(endTime-startTime).count()  << " ms.\n";
    cout << endl;
}
