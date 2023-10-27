#include "global.h"
/**
 * @brief 
 * SYNTAX: <new_table> <- GROUP BY <grouping_attribute> FROM <table_name> 
 *                          HAVING <aggregate_func(attribute)> <bin_op> <attribute_value> 
 *                          RETURN <aggregate_func(attribute)>
 */
bool syntacticParseGROUPBY()
{
    logger.log("syntacticParseGROUPBY");
    // for(int i = 0; i < tokenizedQuery.size(); ++i) {
    //     cout << i << "|" << tokenizedQuery[i] << endl;
    // }
    if (tokenizedQuery.size() != 13 || tokenizedQuery[2] != "GROUP" || tokenizedQuery[3] != "BY"
            || tokenizedQuery[5] != "FROM" || tokenizedQuery[7] != "HAVING"|| tokenizedQuery[11] != "RETURN")
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = GROUPBY;
    parsedQuery.groupbyResultRelationName = tokenizedQuery[0];
    parsedQuery.groupbyColumnName = tokenizedQuery[4];
    parsedQuery.groupbyRelationName = tokenizedQuery[6];

    string attr1 = "", attr2 = "", func1 = "", func2 = "";
    int i=0; 
    while(tokenizedQuery[8][i] != '('){
        func1.push_back(tokenizedQuery[8][i]);
        i++;
    }
    i++;
    while(tokenizedQuery[8][i] != ')'){
        attr1.push_back(tokenizedQuery[8][i]);
        i++;
    }   
    
    i=0;
    while(tokenizedQuery[12][i] != '('){
        func2.push_back(tokenizedQuery[12][i]);
        i++;
    }
    i++;
    while(tokenizedQuery[12][i] != ')'){
        attr2.push_back(tokenizedQuery[12][i]);
        i++;
    } 

    if(attr1 != attr2){
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.groupbyGroupingColumnName = attr1;
    parsedQuery.groupbyAggregateFunc1 = func1;
    parsedQuery.groupbyAggregateFunc2 = func2;

    // bool flag = false;
    // for(char &ch: tokenizedQuery[8]) {
    //     if(ch == '(') {
    //         flag = true;
    //         continue;
    //     }
    //     else if(ch == ')') {
    //         flag = false;
    //         continue;
    //     }

    //     if(flag) {
    //         str.push_back(ch);
    //     }
    // }

    // parsedQuery.groupby = tokenizedQuery[];

    string binaryOperator = tokenizedQuery[9];
    if (binaryOperator == "<")
        parsedQuery.groupbyBinaryOperator = LESS_THAN;
    else if (binaryOperator == ">")
        parsedQuery.groupbyBinaryOperator = GREATER_THAN;
    else if (binaryOperator == ">=" || binaryOperator == "=>")
        parsedQuery.groupbyBinaryOperator = GEQ;
    else if (binaryOperator == "<=" || binaryOperator == "=<")
        parsedQuery.groupbyBinaryOperator = LEQ;
    else if (binaryOperator == "==")
        parsedQuery.groupbyBinaryOperator = EQUAL;
    else if (binaryOperator == "!=")
        parsedQuery.groupbyBinaryOperator = NOT_EQUAL;
    else
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }

    parsedQuery.groupbyConditionValue = stoi(tokenizedQuery[10]);

    return true;
}

bool semanticParseGROUPBY()
{
    logger.log("semanticParseGROUPBY");

    if (tableCatalogue.isTable(parsedQuery.joinResultRelationName))
    {
        cout << "SEMANTIC ERROR: Resultant relation already exists" << endl;
        return false;
    }

    if (!tableCatalogue.isTable(parsedQuery.groupbyRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << parsedQuery.groupbyRelationName << endl;
        return false;
    }

    if (!tableCatalogue.isColumnFromTable(parsedQuery.groupbyColumnName, parsedQuery.groupbyRelationName) || 
        !tableCatalogue.isColumnFromTable(parsedQuery.groupbyGroupingColumnName, parsedQuery.groupbyRelationName))
    {
        cout << "SEMANTIC ERROR: Column doesn't exist in relation" << endl;
        return false;
    }
    return true;
}

void executeGROUPBY()
{
    logger.log("executeGROUPBY");

    Table* table = tableCatalogue.getTable(parsedQuery.groupbyRelationName);
    
    // create trimmed table by taking only the two required columns
    vector<string> trimmedTableCols;
    trimmedTableCols.push_back(parsedQuery.groupbyColumnName);
    trimmedTableCols.push_back(parsedQuery.groupbyGroupingColumnName);

    Table* trimmedTable = new Table(parsedQuery.groupbyResultRelationName+"trimmedTable", trimmedTableCols);
    tableCatalogue.insertTable(trimmedTable);

    // copy only the required columns from original table into trim table
    trimmedTable->trim(table);

    // get sorting strategy
    vector<int> sortingStrategy;    
    sortingStrategy.push_back(ASC);

    // get sorting col name
    vector<string> sortColumnNames;
    sortColumnNames.push_back(parsedQuery.groupbyColumnName);

    // // sort 
    trimmedTable->sortTable(sortColumnNames, sortingStrategy);
    // table->sortTable(sortColumnNames, sortingStrategy);

    vector<string> trimmedTableCols2;
    trimmedTableCols2.push_back(parsedQuery.groupbyColumnName);
    // trimmedTableCols2.push_back(parsedQuery.groupbyGroupingColumnName);
    trimmedTableCols2.push_back("MAX");
    trimmedTableCols2.push_back("MIN");
    trimmedTableCols2.push_back("SUM");
    trimmedTableCols2.push_back("COUNT");

    Table* groupbyCalculations = new Table(parsedQuery.groupbyResultRelationName+"trimmedTable2", trimmedTableCols2);
    tableCatalogue.insertTable(groupbyCalculations);

    groupbyCalculations->calculate(trimmedTable);
    // groupbyCalculations->calculate(table);

    vector<string> resTableCols;
    resTableCols.push_back(parsedQuery.groupbyColumnName);
    resTableCols.push_back(parsedQuery.groupbyAggregateFunc2+parsedQuery.groupbyGroupingColumnName);
    Table* resTable = new Table(parsedQuery.groupbyResultRelationName, resTableCols);
    tableCatalogue.insertTable(resTable);

    resTable->groupBy(groupbyCalculations, parsedQuery.groupbyAggregateFunc1, parsedQuery.groupbyAggregateFunc2, 
                    parsedQuery.groupbyBinaryOperator, parsedQuery.groupbyConditionValue);

    // delete resultant csv file
    string filename = "../data/temp/" + resTable->tableName + ".csv";
    logger.log("csv file to be deleted: " + filename);
    if(remove(filename.c_str()) == 0)   logger.log("csv file deleted successfully");
    else                                logger.log("csv file deleteion unsuccessful");
    
    // delete resTrimmedtable csv  
    filename = "../data/temp/" + parsedQuery.groupbyResultRelationName+"trimmedTable" + ".csv";
    logger.log("csv file to be deleted: " + filename);
    if(remove(filename.c_str()) == 0)   logger.log("csv file deleted successfully");
    else                                logger.log("csv file deleteion unsuccessful");  
    
    // delete resTrimmedtable2 csv
    filename = "../data/temp/" + parsedQuery.groupbyResultRelationName+"trimmedTable2" + ".csv";
    logger.log("csv file to be deleted: " + filename);
    if(remove(filename.c_str()) == 0)   logger.log("csv file deleted successfully");
    else                                logger.log("csv file deleteion unsuccessful");
    
    // delete pages of resTrimmedtable
    int pageIdx = 0;
    while(bufferManager.removeFromPool(parsedQuery.groupbyResultRelationName+"trimmedTable", pageIdx)){ ++pageIdx;}
    tableCatalogue.deleteTable(parsedQuery.groupbyResultRelationName+"trimmedTable");
    bufferManager.deleteFile(parsedQuery.groupbyResultRelationName+"trimmedTable");

    // delete pages of resTrimmedtable2
    pageIdx = 0;
    while(bufferManager.removeFromPool(parsedQuery.groupbyResultRelationName+"trimmedTable2", pageIdx)){ ++pageIdx;}
    tableCatalogue.deleteTable(parsedQuery.groupbyResultRelationName+"trimmedTable2");
    bufferManager.deleteFile(parsedQuery.groupbyResultRelationName+"trimmedTable2");

    return;
}
