#include "global.h"
/**
 * @brief 
 * SYNTAX: new_table <- ORDER BY attribute ASC|DESC ON table_name
 */
bool syntacticParseORDERBY()
{
    logger.log("syntacticParseORDERBY");
    
    if (tokenizedQuery.size() != 8 || tokenizedQuery[2] != "ORDER" || tokenizedQuery[3] != "BY"
            || (tokenizedQuery[5] != "ASC" && tokenizedQuery[5] != "DESC") || tokenizedQuery[6] != "ON")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = ORDERBY;
    parsedQuery.orderbyResultRelationName = tokenizedQuery[0];
    parsedQuery.orderbyColumnName = tokenizedQuery[4];
    parsedQuery.orderbyRelationName = tokenizedQuery[7];

    return true;
}

bool semanticParseORDERBY()
{
    logger.log("semanticParseORDERBY");

    if (tableCatalogue.isTable(parsedQuery.orderbyResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.orderbyRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << parsedQuery.orderbyRelationName << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.orderbyColumnName, parsedQuery.orderbyRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeORDERBY()
{
    logger.log("executeORDERBY");

    Table* table = tableCatalogue.getTable(parsedQuery.orderbyRelationName);
    
    // create resultant table same as original table
    vector<string> resCols = table->columns;
    Table* resTable = new Table(parsedQuery.orderbyResultRelationName, resCols);
    tableCatalogue.insertTable(resTable);

    // copy data original table into resultant table
    resTable->copy(table);

    // get soting strategy
    vector<int> sortingStrategy;
    if(tokenizedQuery[5] == "ASC")
        sortingStrategy.push_back(ASC);
    else
        sortingStrategy.push_back(DESC);

    // get sorting col name
    vector<string> sortColumnNames;
    sortColumnNames.push_back(parsedQuery.orderbyColumnName);

    // sort
    resTable->sortTable(sortColumnNames, sortingStrategy);

    // delete resultant csv file
    string filename = "../data/temp/" + resTable->tableName + ".csv";
    logger.log("csv file to be deleted: " + filename);
    if(remove(filename.c_str()) == 0)
        logger.log("csv file deleted successfully");
    else
        logger.log("csv file deleteion unsuccessful");

    return;
}
