#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <chrono>
#include <fstream>

using namespace std;

struct summResults
{
    vector<int> summaryString;
    vector<int> summarySuffixOffsets;
    int summaryAlphabetSize; 
};

vector<int> SA_IS(const string& input, int alphabetSize);
vector<int> makeSummarySuffixArray(summResults results);

string LMSClassify(string input)
{
    unsigned size = input.size();
    string LMSTypeMap(size, ' ');
    LMSTypeMap[size - 1] = 'S';

    for(int i = size - 2; i >= 0; i--)
    {

        if(input[i] < input[i + 1])
        {
            LMSTypeMap[i] = 'S';
        }
        else if(input[i] > input[i + 1])
        {
            LMSTypeMap[i] = 'L';
        }
        else
        {
            LMSTypeMap[i] = LMSTypeMap[i + 1];
        }
    }
    return LMSTypeMap;
} 

bool isLMSChar(unsigned pos, const std::string& LMSTypeMap) {

    if (pos == 0)
    {
        return false;
    }
    return (LMSTypeMap[pos - 1] == 'L' && LMSTypeMap[pos] == 'S');
}

bool areLMSSubstringsEqual(const string& input, const string& LMSTypeMap, unsigned posA, unsigned posB) {
    if (posA == input.size() || posB == input.size()) {
        return false;
    }
    int i = 0;
    const int p = 31;  // Base for rolling hash
    const int m = 1e9 + 9;  // Modulo for rolling hash
    long long hashA = 0, hashB = 0, p_power = 1;

    while (true) {
        bool ALMS = isLMSChar(i + posA, LMSTypeMap);
        bool BLMS = isLMSChar(i + posB, LMSTypeMap);

        if (i > 0 && ALMS && BLMS) {
            return true;
        }

        // Compute the hash for the current position
        hashA = (hashA + (input[posA + i] - 'a' + 1) * p_power) % m;
        hashB = (hashB + (input[posB + i] - 'a' + 1) * p_power) % m;

        // Compare hashes
        if (hashA != hashB) {
            return false;
        }

        // If one is LMS and the other is not, we stop comparing
        if (ALMS != BLMS) {
            return false;
        }

        // Update the power of p for the next character
        p_power = (p_power * p) % m;
        i++;
    }
}


vector<int> findBucketSize(string input, unsigned alphabetSize=96)
{
    vector<int> buckets(alphabetSize, 0);

    for(int i = 0; i < input.size(); i++)
    {
        buckets[input[i] - 31]++;
    }
    return buckets;
}

vector<int> findBucketTail(const vector<int>& bucketSize)
{
    vector<int> bucketHeadVector;
    unsigned offset = 0;
    for (int i = 0; i < bucketSize.size(); i++)
    {
        int tail = offset + bucketSize[i] - 1;
        bucketHeadVector.push_back(tail);
        offset += bucketSize[i];
    }
    return bucketHeadVector;
}

vector<int> findBucketHead(const vector<int>& bucketSize)
{
    vector<int> bucketHeadVector;
    unsigned offset = 0;
    for (int i = 0; i < bucketSize.size(); i++)
    {
        bucketHeadVector.push_back(offset);
        offset += bucketSize[i];
    }
    return bucketHeadVector;
}   

void showSuffixArray(vector<int> arr, unsigned pos = -1)
{
    for(int i = 0; i < arr.size(); i++)
    {
        cout << setw(2) << setfill('0') << arr[i] << " ";
    }
    cout << "\n";
    if(pos != static_cast<unsigned>(-1) )
    {
        for (int i = 0; i < arr.size(); i++)
        {
            if(i == pos)
            {
                cout << "^^";
                break;
            }
            else
            {
                cout << "   ";
            }
        }
        
    }
    cout << "\n";
}

vector<int> guessLMSSort(string input, vector<int> bucketSize, string LMSTypeMap)
{
    vector<int> guessedSA(input.size(), -1);
    guessedSA.reserve(input.size());
    vector<int> bucketTails = findBucketTail(bucketSize);
    for(int i = 0; i < input.size(); i++)
    {
        if(!isLMSChar(i, LMSTypeMap))
        {
            continue;
        }
        
        int bucketIndex = input[i] - 31;
        guessedSA[bucketTails[bucketIndex]] = i;
        bucketTails[bucketIndex] -= 1;

        //showSuffixArray(guessedSA, i);
    }
    guessedSA[0] = input.size() - 1;
    return guessedSA;
}

vector<int> induceL(string input, vector<int>& guessedSA, const vector<int>& bucketSizes, string LMSTypeMap)
{
    vector<int> bucketHeads = findBucketHead(bucketSizes);
    for(int i = 0; i < guessedSA.size(); i++)
    {
        if(guessedSA[i] == -1) continue;

        int j = guessedSA[i] - 1;
        if(j < 0 || LMSTypeMap[j] != 'L') continue;

        int bucketIndex = input[j] - 31;
        guessedSA[bucketHeads[bucketIndex]] = j;
        bucketHeads[bucketIndex]++;
    }
    return guessedSA;
}


vector<int> induceS(string input, vector<int>& guessedSA, const vector<int>& bucketSizes, const string& LMSTypeMap)
{
    vector<int> bucketTails = findBucketTail(bucketSizes);

    for (int i = guessedSA.size() - 1; i >= 0; --i)
    {
        int j = guessedSA[i] - 1;

        if(j < 0)
        {
            continue;
        }
        if(LMSTypeMap[j] != 'S')
        {
            continue;
        }
        int bucketIndex = input[j] - 31;
        guessedSA[bucketTails[bucketIndex]] = j;
        bucketTails[bucketIndex] -= 1;
    }
    return guessedSA;
}

summResults summariseSA(const string& input, vector<int>& guessedSA, const string& LMSTypeMap)
{
    vector<int> lmsNames(input.size(), -1);
    int currName = 0;
    lmsNames[guessedSA[0]] = currName;
    int lastLMSSuffixOffset = guessedSA[0];
    for (int i = 1; i < guessedSA.size(); i++) {
        int suffixOffset = guessedSA[i];
        if (!isLMSChar(suffixOffset, LMSTypeMap))
        {
         continue;
        }    

        if (!areLMSSubstringsEqual(input, LMSTypeMap, lastLMSSuffixOffset, suffixOffset)) {
            currName++;
        }
        lastLMSSuffixOffset = suffixOffset;
        lmsNames[suffixOffset] = currName;
    }

    vector<int> summarySuffixOffsets;
    summarySuffixOffsets.reserve(lmsNames.size());
    vector<int> summaryString;
    summaryString.reserve(lmsNames.size());
    for (int index = 0; index < lmsNames.size(); index++) {
        int name = lmsNames[index];
        if (name == -1) continue;
        summarySuffixOffsets.push_back(index);
        summaryString.push_back(name);
    }
    int summaryAlphabetSize = currName + 1;
    summResults result;
    result.summaryString = summaryString;
    result.summaryAlphabetSize = summaryAlphabetSize;
    result.summarySuffixOffsets = summarySuffixOffsets;

    return result;
}



vector<int> accurateLMSSort(string input, vector<int>& bucketSizes, const string& LMSTypeMap, vector<int> summarySuffixArray, vector<int> summarySuffixOffsets)
{
    vector<int> suffixOffsets(input.size(), -1);
    vector<int> bucketTails = findBucketTail(bucketSizes);
    int inputIndex;
    int bucketIndex;
    for (int i = summarySuffixArray.size() - 1; i >= 1; i--)
    {
        inputIndex = summarySuffixOffsets[summarySuffixArray[i]];
        bucketIndex = input[inputIndex] - 31;
        suffixOffsets[bucketTails[bucketIndex]] = inputIndex;

        bucketTails[bucketIndex] -= 1;
    }

    suffixOffsets[0] = input.size() - 1;

    return suffixOffsets;
}


vector<int> SA_IS(const string& input, int alphabetSize = 256)
{
    string modifiedInput = input + static_cast<char>(31);
    cout << "[CONSOLE] Classifying LMS... \n";
    string LMSTypeMap = LMSClassify(modifiedInput);
    
    // Precompute bucket sizes once
    vector<int> bucketSizes = findBucketSize(modifiedInput);
    
    cout << "[CONSOLE] Guessing LMS... \n";
    vector<int> guessed = guessLMSSort(modifiedInput, bucketSizes, LMSTypeMap);
    
    cout << "[CONSOLE] Inducing L... \n";
    vector<int> inducedL = induceL(modifiedInput, guessed, bucketSizes, LMSTypeMap);
    
    guessed.clear();
    guessed.shrink_to_fit();
    
    cout << "[CONSOLE] Inducing S... \n";
    vector<int> inducedS = induceS(modifiedInput, inducedL, bucketSizes, LMSTypeMap);
    
    inducedL.clear();
    inducedL.shrink_to_fit();
    
    cout << "[CONSOLE] Summarising SA... \n";
    summResults results = summariseSA(modifiedInput, inducedS, LMSTypeMap);
    
    inducedS.clear();
    inducedS.shrink_to_fit();
    
    cout << "[CONSOLE] Making Summary SA... \n";
    vector<int> summarySA = makeSummarySuffixArray(results);
    
    cout << "[CONSOLE] Bucket Sorting... \n";
    vector<int> res = accurateLMSSort(modifiedInput, bucketSizes, LMSTypeMap, summarySA, results.summarySuffixOffsets);
    
    summarySA.clear();
    summarySA.shrink_to_fit();
    results.summaryString.clear();
    results.summaryString.shrink_to_fit();
    results.summarySuffixOffsets.clear();
    results.summarySuffixOffsets.shrink_to_fit();
    
    cout << "[CONSOLE] Inducing L again... \n";
    vector<int> indL = induceL(modifiedInput, res, bucketSizes, LMSTypeMap);
    
    res.clear();
    res.shrink_to_fit();
    
    cout << "[CONSOLE] Inducing S again... \n";
    vector<int> indS = induceS(modifiedInput, indL, bucketSizes, LMSTypeMap);
    
    if (!indS.empty() && indS[0] == static_cast<int>(modifiedInput.size()) - 1) {
        indS.erase(indS.begin());
    }
    
    return indS;
}


vector<int> makeSummarySuffixArray(summResults results)
{
    
    if(results.summaryAlphabetSize == results.summaryString.size())
    {
        vector<int> summarySuffixArray(results.summaryString.size() + 1, -1);
        summarySuffixArray[0] = results.summaryString.size();
        for(int x = 0; x < results.summaryString.size(); x++)
        {
            //showSuffixArray(summarySuffixArray);
            summarySuffixArray[results.summaryString[x]] = x;
        }
        return summarySuffixArray;
    }
    else
    {
        string summaryStr;
        for(int val : results.summaryString)
        {
            char c = static_cast<char>(val + 31);
            summaryStr += c;
        }
        return SA_IS(summaryStr, 95);
    }
    
}



void printLMS(string input, string LMSTypeMap)
{
    cout << LMSTypeMap << "\n";
    for(int i = 0; i < input.size(); i++)
    {
        if(isLMSChar(i, LMSTypeMap))
        {
            cout <<  "^";
        }
        else
        {
            cout << " "; 
        }
    }
    cout << "\n";
}

string constructBWTFromSA(string text, vector<int> suffix_array) {
    string bwt;
    string extended_text = text + static_cast<char>(31);
    
    for (int i = 0; i < suffix_array.size(); i++) {
        int pos = suffix_array[i];
        if (pos == 0) {
            bwt += extended_text.back();
        } else {
            bwt += extended_text[pos - 1];
        }
    }
    
    return bwt;
}

void writeToDiskText(const std::string& text, const string& outPath) {
    std::ofstream out(outPath, std::ios::binary);
    if (!out) {
        std::cerr << "[Error] Not possible to open file for writing.\n";
        return;
    }

    out.write(text.c_str(), text.size());
    out.close();
}

int main(int argc, char* argv[])
{
    if (argc < 3) 
    {
        cout << "Usage: ./sa-is <input_file> <output_file>\n";
        return 1;
    }

    ifstream inFile(argv[1]);
    if (!inFile) {
        cerr << "Error: Could not open input file " << argv[1] << endl;
        return 1;
    }
    string input((istreambuf_iterator<char>(inFile)), istreambuf_iterator<char>());
    inFile.close();

    string baseName = argv[2];

    cout << "[CONSOLE] Initializing...\n";
    auto start = chrono::high_resolution_clock::now();   
    vector<int> SAIS = SA_IS(input, 256);
    auto end = chrono::high_resolution_clock::now();

    cout << "[CONSOLE] Building BWT... \n";
    
    auto start2 = chrono::high_resolution_clock::now();
    string bwt = constructBWTFromSA(input, SAIS);
    auto end2 = chrono::high_resolution_clock::now();
    
    if (input.size() < 100) {
        cout << "\nSuffix Array:\n";
        for(int i = 0; i < SAIS.size(); i++) {
            cout << SAIS[i] << " ";
        }
        cout << endl;
        cout<< "\nBWT:\n";
        for(int i =0; i < bwt.size(); i++) {
            cout << bwt[i] << " ";
        }
        cout<< "\n";
    }
    
    cout << "\n[SAIS Construction: Time elapsed: "
         << chrono::duration_cast<chrono::milliseconds>(end - start).count()
         << " ms]\n";
    
    cout << "\n[BWT Construction: Time elapsed: "
         << chrono::duration_cast<chrono::milliseconds>(end2 - start2).count()
         << " ms]\n\n";

    writeToDiskText(bwt, baseName);
    cout << "BWT written to " << baseName << endl;
}
