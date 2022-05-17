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
void TestAddDocumentForSearching()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL).size() == 1,"Number of documents inconsistent");
}

//Поддержка минус-слов. Документы, содержащие минус-слова из поискового запроса, не должны включаться в результаты поиска.
void TestSupportMinusWord()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на с"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый ухоженный пес"s, DocumentStatus::ACTUAL, { 9, 13, 22 });
    search_server.AddDocument(2, "пушистый пес с ошейником"s, DocumentStatus::ACTUAL, { 7, 8, -3, 17 });
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный -кот"s, DocumentStatus::ACTUAL).size() == 2, "Request has the minus word");
    ASSERT_HINT(search_server.FindTopDocuments("-пушистый ухоженный кот"s, DocumentStatus::ACTUAL).size() == 1, "Request has the minus word");
}


//Сортировка найденных документов по релевантности. Возвращаемые при поиске документов результаты должны быть отсортированы в порядке убывания релевантности.
void TestSortDocumentRelevance()
{
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
        vector <Document> test_correct_relevance = search_server.FindTopDocuments("пушистый ухоженный кот"s);
        ASSERT_HINT(test_correct_relevance.size() == 3, "Wrong size vector");
        const double zapros0 = (log(3) * 0.5 + log(1.5) * 0.25);
        ASSERT_EQUAL_HINT(test_correct_relevance[0].relevance, zapros0, "Wrong sort document for relevance");
        const double zapros1 = log(3) * 0.25;
        ASSERT_EQUAL_HINT(test_correct_relevance[1].relevance, zapros1, "Wrong sort document for relevance");
        const double zapros2 = log(1.5) * 0.25;
        ASSERT_EQUAL_HINT(test_correct_relevance[2].relevance, zapros2, "Wrong sort document for relevance");
}


//Вычисление рейтинга документов. Рейтинг добавленного документа равен среднему арифметическому оценок документа.
void TestCountRatingDocument()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    vector <Document> test_count_raiting = search_server.FindTopDocuments("пушистый ухоженный кот"s);
    ASSERT_HINT(test_count_raiting.size() == 3, "Not correct size vector");
    ASSERT_HINT(test_count_raiting[0].rating == (7+2+7)/3, "Not correct rating");
    ASSERT_HINT(test_count_raiting[1].rating == (8-3)/2, "Not correct rating");
    ASSERT_HINT(test_count_raiting[2].rating == (5-12+2+1)/4, "Not correct rating");
}


//Фильтрация результатов поиска с использованием предиката, задаваемого пользователем.
void TestFiltrPredicate()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });
    //обычный
    vector <Document> Test_usual = search_server.FindTopDocuments("пушистый ухоженный кот"s);
    ASSERT_HINT(Test_usual.size() == 3, "Wrong size vector");
    ASSERT_EQUAL(Test_usual[0].id, 1);
    ASSERT_EQUAL(Test_usual[1].id , 0);
    ASSERT_EQUAL(Test_usual[2].id , 2);
    //status
    vector <Document> Test_status = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
    ASSERT_HINT(Test_status.size() == 1, "Wrong size vector");
    ASSERT_EQUAL_HINT(Test_status[0].id , 3, "Wrong document");
    //predicate
    vector <Document> Test_Predicate = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
    ASSERT_HINT(Test_Predicate.size() == 2, "Wrong size vector");
    ASSERT(Test_Predicate[0].id == 0);
    ASSERT(Test_Predicate[1].id == 2);
}

//Поиск документов, имеющих заданный статус.
void TestFindDocumentWhichHaveStatus()
{
    SearchServer search_server;
    search_server.SetStopWords("и в на"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::IRRELEVANT, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::BANNED, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::REMOVED, { 9 });
    search_server.AddDocument(4, "белый кот и модный ошейник ухожунный пес"s, DocumentStatus::ACTUAL, { 8, -3, 10, 11 });
    //ACTUAL
    vector <Document> test_Actual = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL);
    ASSERT(test_Actual.size() == 2);
    ASSERT_HINT(test_Actual[0].id == 0, "The given status is not matched by these documents");
    ASSERT_HINT(test_Actual[1].id == 4, "The given status is not matched by these documents");
    //BANNED
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED).size()==1);
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)[0].id == 2,"The given status is not matched by these documents");
    //IRRELEVANTE
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::IRRELEVANT).size() == 1);
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::IRRELEVANT)[0].id == 1, "The given status is not matched by these documents");
    //REMOVED
    ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::REMOVED).size() == 1);
    ASSERT_HINT(search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::REMOVED)[0].id == 3, "The given status is not matched by these documents");

}

//Корректное вычисление релевантности найденных документов.
void TestCorrectCountRelevanceDocument()
{
    //1 test
    {
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s).size() == 1);
        ASSERT(search_server.FindTopDocuments("пушистый ухоженный кот"s)[0].relevance == 0);
    }
    //2 test
    {
        SearchServer search_server;
        search_server.SetStopWords("и в на"s);
        search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
        search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
        vector <Document> test_correct_relevance = search_server.FindTopDocuments("пушистый ухоженный кот"s);
        ASSERT_HINT(test_correct_relevance.size() == 3, "Wrong size vector");
        ASSERT_EQUAL_HINT(test_correct_relevance[0].relevance, log(search_server.GetDocumentCount() / 1.0) * 0.5 + log(search_server.GetDocumentCount() / 2.0) * 0.25, "Wrong relevance");
        ASSERT_EQUAL_HINT(test_correct_relevance[1].relevance, log(search_server.GetDocumentCount() / 1.0) * 0.25, "Wrong relevance");
        ASSERT_EQUAL_HINT(test_correct_relevance[2].relevance, log(search_server.GetDocumentCount() / 2.0) * 0.25, "Wrong relevance");
    }
}

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    TestExcludeStopWordsFromAddedDocumentContent();

    TestSupportMinusWord();
    TestAddDocumentForSearching();
    TestSortDocumentRelevance();
    TestCountRatingDocument();
    TestFiltrPredicate();
    TestFindDocumentWhichHaveStatus();
    TestCorrectCountRelevanceDocument();

    // Не забудьте вызывать остальные тесты здесь
}
