#pragma once
#include <iostream>
#include <vector>

struct Document
{
    Document();
    Document(int set_id, double set_relevance, int set_rating); // конструкторы для структуры

    int id = 0;
    double relevance = 0.0;
    int rating = 0;
};

//возможные статусы документа
enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

//функции вывода 
std::ostream& operator<<(std::ostream& out, const Document& document);

void PrintDocument(const Document& document);

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status);
