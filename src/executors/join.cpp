#include "global.h"
/**
 * @brief 
 * SYNTAX: R <- JOIN relation_name1, relation_name2 ON column_name1 bin_op column_name2
 */
bool syntacticParseJOIN()
{
    logger.log("syntacticParseJOIN");
    if (tokenizedQuery.size() != 9 || tokenizedQuery[5] != "ON")
    {
        cout << "SYNTAC ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = JOIN;
    parsedQuery.joinResultRelationName = tokenizedQuery[0];
    parsedQuery.joinFirstRelationName = tokenizedQuery[3];
    parsedQuery.joinSecondRelationName = tokenizedQuery[4];
    parsedQuery.joinFirstColumnName = tokenizedQuery[6];
    parsedQuery.joinSecondColumnName = tokenizedQuery[8];

    string binaryOperator = tokenizedQuery[7];
    if (binaryOperator == "<")
        parsedQuery.joinBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.joinBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.joinBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.joinBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.joinBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.joinBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    return true;
}

bool semanticParseJOIN()
{
    logger.log("semanticParseJOIN");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.joinFirstRelationName) || !tableCatalogue.isTable(parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.joinFirstColumnName, parsedQuery.joinFirstRelationName) || !tableCatalogue.isColumnFromTable(parsedQuery.joinSecondColumnName, parsedQuery.joinSecondRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeJOIN()
{
    logger.log("executeJOIN");
    Table* table1 = tableCatalogue.getTable(parsedQuery.joinFirstRelationName);
    Table* table2 = tableCatalogue.getTable(parsedQuery.joinSecondRelationName);

    vector<int> sortingStrategy;
    if(parsedQuery.joinBinaryOperator == EQUAL || parsedQuery.joinBinaryOperator == NOT_EQUAL ||
        parsedQuery.joinBinaryOperator == LESS_THAN || parsedQuery.joinBinaryOperator == LEQ)
        sortingStrategy.push_back(ASC);
    else
        sortingStrategy.push_back(DESC);

    vector<string> sortColumnNames1, sortColumnNames2;
    sortColumnNames1.push_back(parsedQuery.joinFirstColumnName);
    sortColumnNames2.push_back(parsedQuery.joinSecondColumnName);

    // Make copies of both tables
    vector<string> cols1 = table1->columns;
    Table* table1copy = new Table("table1copy", cols1);
    tableCatalogue.insertTable(table1copy);
    table1copy->copy(table1);

    vector<string> cols2 = table2->columns;
    Table* table2copy = new Table("table2copy", cols2);
    tableCatalogue.insertTable(table2copy);
    table2copy->copy(table2);

    table1copy->sortTable(sortColumnNames1, sortingStrategy);
    table2copy->sortTable(sortColumnNames2, sortingStrategy);

    // create resultant table
    vector<string> resCols;
    resCols.insert(resCols.end(), cols1.begin(), cols1.end());
    resCols.insert(resCols.end(), cols2.begin(), cols2.end());

    // logger.log("columns of res: ");
    // for(int i=0; i<resCols.size(); i++)
    // {
    //     logger.log(resCols[i]);
    // }
    
    Table* resTable = new Table(parsedQuery.joinResultRelationName, resCols);
    tableCatalogue.insertTable(resTable);
    
    resTable->join(table1copy, table2copy, parsedQuery.joinFirstColumnName, parsedQuery.joinSecondColumnName, parsedQuery.joinBinaryOperator);

    // delete resultant csv file
    string filename = "../data/temp/" + resTable->tableName + ".csv";
    logger.log("csv file to be deleted: " + filename);
    if(remove(filename.c_str()) == 0)
        logger.log("csv file deleted successfully");
    else
        logger.log("csv file deleteion unsuccessful");

    // delete pages of copies of both the original tables
    int pageIdx = 0;
    while(bufferManager.removeFromPool("table1copy", pageIdx)){ ++pageIdx;}
    tableCatalogue.deleteTable("table1copy");
    bufferManager.deleteFile("table1copy");

    pageIdx = 0;
    while(bufferManager.removeFromPool("table2copy", pageIdx)){ ++pageIdx;}
    tableCatalogue.deleteTable("table2copy");
    bufferManager.deleteFile("table2copy");

    return;
}