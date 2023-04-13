#include "test_example_functions.h"
void RemoveDuplicates(SearchServer& search_server)
{
    std::set <int> remove_id;
    std::set<std::set<std::string>> new_request;
    for (auto start = search_server.begin(); start != search_server.end(); start++)
    {

        std::set<std::string> words;
        for (const auto& word_frequency : search_server.GetWordFrequencies(*start))
        {
            words.insert(word_frequency.first);
        }
        if (new_request.count(words))
        {
            remove_id.insert(*start);
        }
        else
        {
            new_request.insert(words);
        }
    }
    for (auto id : remove_id)
    {
        search_server.RemoveDocument(id);
        std::cout << "Found duplicate document id " << id << std::endl;
    }
}