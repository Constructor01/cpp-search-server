#pragma once
#include "search_server.h"
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        int request_time;
        int results;
    };
    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    int time_;
    int no_request_;
    const int total_time_ = 1440;

    void NewRequest(int results);
};

template <typename DocumentPredicate>
std::vector<Document>RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    const auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
    NewRequest(result.size());
    return result;
}

