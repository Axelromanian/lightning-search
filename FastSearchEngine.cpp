#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <cmath>
#include <thread>
#include <mutex>
#include <chrono>

using namespace std;
namespace fs = std::filesystem;

struct SearchResult {
    int docId;
    double score;
    string title;
};

class SearchEngine {
private:
    unordered_map<string, unordered_map<int, int>> invertedIndex;
    unordered_map<int, string> docTitles;
    unordered_map<int, int> docLengths;
    int totalDocuments = 0;
    double avgDocLength = 0;
    mutex mtx;

    vector<string> tokenize(const string& text) {
        vector<string> tokens;
        stringstream ss(text);
        string word;
        while (ss >> word) {
            string cleanWord = "";
            for (char c : word) {
                if (isalnum(static_cast<unsigned char>(c))) {
                    cleanWord += tolower(static_cast<unsigned char>(c));
                }
            }
            if (!cleanWord.empty()) {
                tokens.push_back(cleanWord);
            }
        }
        return tokens;
    }

public:
    void addDocument(int docId, const string& title, const string& text) {
        vector<string> words = tokenize(text);
        int length = words.size();
        if (length == 0) return;

        lock_guard<mutex> lock(mtx);
        docTitles[docId] = title;
        docLengths[docId] = length;
        totalDocuments++;

        for (const string& word : words) {
            invertedIndex[word][docId]++;
        }
    }

    void addFile(int docId, const string& filePath) {
        ifstream file(filePath);
        if (!file.is_open()) return;
        stringstream buffer;
        buffer << file.rdbuf();
        addDocument(docId, filePath, buffer.str());
    }

    // Scanează automat folderul curent
    void indexCurrentFolder() {
        int idCounter = 1;
        vector<thread> threads;

        // Caută automat în folderul curent și subfoldere
        for (auto& p : fs::recursive_directory_iterator("./")) {
            if (p.is_regular_file()) {
                string ext = p.path().extension().string();
                if (ext == ".cpp" || ext == ".h" || ext == ".txt" || ext == ".py" || ext == ".md") {
                    string pathStr = p.path().string();
                    threads.emplace_back(&SearchEngine::addFile, this, idCounter++, pathStr);
                }
            }
        }

        for (auto& t : threads) {
            if (t.joinable()) t.join();
        }

        long long totalLen = 0;
        for (const auto& pair : docLengths) {
            totalLen += pair.second;
        }
        if (totalDocuments > 0) {
            avgDocLength = static_cast<double>(totalLen) / totalDocuments;
        }

        cout << "[C++] Indexare gata! Fișiere de cod găsite și indexate: " << totalDocuments << "\n\n";
    }

    void search(const string& query) {
        auto start = chrono::high_resolution_clock::now();

        vector<string> terms = tokenize(query);
        if (terms.empty()) {
            cout << "Căutare goală.\n\n";
            return;
        }

        unordered_map<int, double> docScores;
        double k1 = 1.2;
        double b = 0.75;

        for (const string& term : terms) {
            if (invertedIndex.find(term) != invertedIndex.end()) {
                int docFreq = invertedIndex[term].size();
                double idf = log(1.0 + (totalDocuments - docFreq + 0.5) / (docFreq + 0.5));

                for (const auto& pair : invertedIndex[term]) {
                    int docId = pair.first;
                    int freq = pair.second;
                    int docLen = docLengths[docId];

                    double numerator = freq * (k1 + 1.0);
                    double denominator = freq + k1 * (1.0 - b + b * (static_cast<double>(docLen) / avgDocLength));
                    double score = idf * (numerator / denominator);

                    docScores[docId] += score;
                }
            }
        }

        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();

        if (docScores.empty()) {
            cout << "Niciun rezultat pentru \"" << query << "\" (Timp: " << duration << " µs)\n\n";
            return;
        }

        vector<SearchResult> results;
        for (const auto& pair : docScores) {
            results.push_back({ pair.first, pair.second, docTitles[pair.first] });
        }

        sort(results.begin(), results.end(), [](const SearchResult& x, const SearchResult& y) {
            return x.score > y.score;
            });

        cout << "Rezultate pentru \"" << query << "\" (Timp: " << duration << " microsecunde):\n";
        int limit = 0;
        for (const auto& res : results) {
            cout << "  -> [Scor BM25: " << res.score << "] " << res.title << "\n";
            if (++limit >= 5) break;
        }
        cout << "\n";
    }
};

int main() {
    SearchEngine engine;

    cout << "========================================\n";
    cout << "  MOTOR DE CĂUTARE NATIV ÎN C++ (BM25)\n";
    cout << "========================================\n";

    // Scanează automat folderul proiectului
    engine.indexCurrentFolder();

    cout << " Motorul este gata! Scrie un cuvânt din codul tău sau 'exit'.\n";
    cout << "----------------------------------------\n\n";

    string userInput;
    while (true) {
        cout << "Căutare > ";
        getline(cin, userInput);

        if (userInput == "exit" || userInput == "QUIT") {
            break;
        }

        if (!userInput.empty()) {
            engine.search(userInput);
        }
    }

    return 0;
}