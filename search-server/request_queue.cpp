#include "request_queue.h"

using namespace std;

//public:
RequestQueue::RequestQueue(const SearchServer& search_server) :
    search_server_(search_server),
    time_(0),
    no_request_(0)
{ }

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    const auto result = search_server_.FindTopDocuments(raw_query, status);
    NewRequest(result.size());
    return result;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    const auto result = search_server_.FindTopDocuments(raw_query);
    NewRequest(result.size());
    return result;
}

int RequestQueue::GetNoResultRequests() const {
    return no_request_;
}

//private:
void RequestQueue::NewRequest(int results) {
    ++time_;

    while (!requests_.empty() && total_time_ <= time_ - requests_.front().request_time) {
        if (requests_.front().results == 0) {
            no_request_--;
        }

        requests_.pop_front();
    }

    requests_.push_back({ time_, results });
    if (results == 0) {
        no_request_++;
    }
}