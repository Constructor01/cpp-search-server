// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
//(Поддержка стоп-слов. Стоп-слова исключаются из текста документов.)
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = { 1, 2, 3 };
    // Сначала убеждаемся, что поиск слова, не входящего в список стоп-слов,
    // находит нужный документ
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    // Затем убеждаемся, что поиск этого же слова, входящего в список стоп-слов,
    // возвращает пустой результат
    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(), "Stop words must be excluded from documents"s);
    }
}

/*
Разместите код остальных тестов здесь
*/
//Добавление документов.Добавленный документ должен находиться по поисковому запросу, который содержит слова из документа.
void AddDocumentForSearching()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL).size() == 1,"Number of documents inconsistent");
}

//Поддержка минус-слов. Документы, содержащие минус-слова из поискового запроса, не должны включаться в результаты поиска.
void SupportMinusWord()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный -кот"s, DocumentStatus::ACTUAL).size() == 0, "Request has the minus word");
}


//Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void SortDocumentRelevance()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL)[0].id == 1, "Missorted documents");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL)[1].id == 2, "Missorted documents");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL)[2].id == 0, "Missorted documents");
}


//Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void CountRatingDocument()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s)[0].rating==5, "Not correct rating");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s)[1].rating == 2, "Not correct rating");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s)[2].rating == -1, "Not correct rating");
}


//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void FiltrPredicate()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    //обычный
    ASSERT_EQUAL(search_server.FindTopDocuments("пушистый ухоженный кот"s)[0].id, 1);
    ASSERT_EQUAL(search_server.FindTopDocuments("пушистый ухоженный кот"s)[0].rating , 5);
    ASSERT_EQUAL(search_server.FindTopDocuments("пушистый ухоженный кот"s)[1].id , 0);
    ASSERT_EQUAL(search_server.FindTopDocuments("пушистый ухоженный кот"s)[1].rating , 2);
    ASSERT_EQUAL(search_server.FindTopDocuments("пушистый ухоженный кот"s)[2].id , 2);
    ASSERT_EQUAL(search_server.FindTopDocuments("пушистый ухоженный кот"s)[2].rating , -1);
    //status
    ASSERT_EQUAL_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)[0].id , 3, "Wrong document");
    ASSERT_EQUAL_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)[0].rating , 9, "Wrong document");
    //predicate
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })[0].id == 0);
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })[0].rating == 2);
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })[1].id == 2);
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })[1].rating == -1);
}

//Поиск документов, имеющих заданный статус.
void FindDocumentWhichHaveStatus()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::IRRELEVANT, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::BANNED, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::REMOVED, { 9 });
    search_server.AddDocument(4, "белый кот и модный ошейник ухожунный пес"s, DocumentStatus::ACTUAL, { 8, -3, 10, 11 });
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)[0].id == 2,"The given status is not matched by these documents");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::IRRELEVANT)[0].id == 1, "The given status is not matched by these documents");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL)[0].id == 0, "The given status is not matched by these documents");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL)[1].id == 4, "The given status is not matched by these documents");
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::REMOVED)[0].id == 3, "The given status is not matched by these documents");

}

//Корректное вычисление релевантности найденных документов.
void CorrectCountRelevanceDocument()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s)[0].relevance == 0);
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();

    SupportMinusWord();
    AddDocumentForSearching();
    SortDocumentRelevance();
    CountRatingDocument();
    FiltrPredicate();
    FindDocumentWhichHaveStatus();
    CorrectCountRelevanceDocument();

    // Не забудьте вызывать остальные тесты здесь
}
