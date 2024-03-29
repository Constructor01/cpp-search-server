#include "search_server.h"
#include <math.h>
using namespace std;

SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings) {

    const auto words = SplitIntoWordsNoStop(document);

    if (document_id < 0) {
        throw invalid_argument("id value can not be negative"s);
    }

    if (documents_.count(document_id) > 0)
    {
        throw invalid_argument("some document with such id has already been added"s);
    }

    const double inv_word_count = 1.0 / words.size();
    for (const string& word : words) {
        word_to_document_freqs_[word][document_id] += inv_word_count;
        doc_freqs_[document_id][word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    id_doc.insert(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query, DocumentStatus status) const {
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
        return document_status == status;
        });
}

vector<Document> SearchServer::FindTopDocuments(const std::string& raw_query) const {
    return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
}

int SearchServer::GetDocumentCount() const {
    return documents_.size();
}

set<int>::iterator SearchServer::begin()
{
    return id_doc.begin();
}
set<int>::iterator SearchServer::end()
{
    return id_doc.end();
}

tuple<vector<string>, DocumentStatus>SearchServer::MatchDocument(const std::string& raw_query, int document_id) const {
    const auto query = ParseQuery(raw_query);

    vector<std::string> matched_words;
    for (const std::string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.push_back(word);
        }
    }
    for (const std::string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if (word_to_document_freqs_.at(word).count(document_id)) {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

 const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const
{
     static std::map<std::string, double> container_for_idf;
     container_for_idf.clear();
     if (doc_freqs_.count(document_id))
     {
         container_for_idf = doc_freqs_.at(document_id);
         return container_for_idf;
     }
     return container_for_idf;
}

void SearchServer::RemoveDocument(int document_id)
{
    //удаляем из контенера заданный документ с id
    documents_.erase(document_id);
    //удаляем id из set
    id_doc.erase(document_id);
    //переделываем контейнер word_to_document_freqs_, так как контенер менят числа из-за удаления одного документа, значит меняется весь контейнер
    for (auto start : word_to_document_freqs_)
    {
        start.second.erase(document_id);
    }
}


bool SearchServer::IsStopWord(const std::string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const std::string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
        });
}

vector<string> SearchServer::SplitIntoWordsNoStop(const std::string& text) const {
    vector<std::string> words;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {
            throw invalid_argument("the word \"" + word + "\" in document content has forbidden symbol");
        }
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    if (ratings.empty()) {
        return 0;
    }
    int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(const std::string& text) const {
    if (text.empty()) {
        throw invalid_argument("Query word is empty"s);
    }
    std::string word = text;
    bool is_minus = false;
    if (word[0] == '-') {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-') {
        throw invalid_argument("invalid query content"s);
    }

    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(const std::string& text) const {
    Query result;
    for (const std::string& word : SplitIntoWords(text)) {
        if (!IsValidWord(word)) {  // проверка запроса на запретные символы 
            throw invalid_argument("the word \"" + word + "\" in query content has forbidden symbol");
        }
        const auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                result.minus_words.insert(query_word.data);
            }
            else {
                result.plus_words.insert(query_word.data);
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(const std::string& word) const {
    return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}


