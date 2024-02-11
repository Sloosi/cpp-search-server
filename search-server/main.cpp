// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь: 271

// Закомитьте изменения и отправьте их в свой репозиторий.
#include <algorithm>
#include <cmath>
#include <iostream>
#include <set>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;

    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }

    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        int words_count = words.size();

        for (const string& word : words) {
            word_to_document_freqs_[word][document_id] += 1.0 / words_count;
        }

        ++documents_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(),
            [](const Document& lhs, const Document& rhs) {
                return lhs.relevance > rhs.relevance;
            });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        
        return matched_documents;
    }

private:
    map<string, map<int, double>> word_to_document_freqs_;
    set<string> stop_words_;
    int documents_count_ = 0;
    
    struct QueryWord {
        string data;
        bool is_minus;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    QueryWord ParseQueryWord(string word) const {
        bool is_minus = false;

        if (word[0] == '-')
        {
            is_minus = true;
            word = word.substr(1);
        }
        
        return {word, is_minus};
    }

    Query ParseQuery(const string& text) const {
        Query query_words;

        for (string& word : SplitIntoWordsNoStop(text)) {
            const QueryWord query_word = ParseQueryWord(word);

            if (query_word.is_minus) {
                query_words.minus_words.insert(query_word.data);
            }
            else {
                query_words.plus_words.insert(query_word.data);
            }
        }

        return query_words;
    }

    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> matched_documents = MatchDocuments(query_words);

        vector<Document> documents;

        for (const auto& [id, relevance] : matched_documents) {
            documents.push_back({ id, relevance });
        }
        
        return documents;
    }
    
    map<int, double> MatchDocuments(const Query& query_words) const {
        map<int, double> documents;
        
        const set<string>& plus_words = query_words.plus_words;
        const set<string>& minus_words = query_words.minus_words;

        for (const string& word : plus_words) {
            if (word_to_document_freqs_.count(word)) {
                double IDF = CalculateIDF(word);
                
                const map<int, double>& words_tf = word_to_document_freqs_.at(word);

                for (const auto& [document_id, TF] : words_tf) {
                    documents[document_id] += TF * IDF;
                }
            }
        }

        for (const string& word : minus_words) {
            if (word_to_document_freqs_.count(word)) {
                const map<int, double>& words_tf = word_to_document_freqs_.at(word);

                for (const auto& [document_id, TF] : words_tf) {
                    documents.erase(document_id);
                }
            }
        }
        
        return documents;
    }
    
    double CalculateIDF(const string& query_word) const {
        return log(static_cast<double>(documents_count_) / word_to_document_freqs_.at(query_word).size());
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();

    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
            << "relevance = "s << relevance << " }"s << endl;
    }
}