#include"global.h"
/**
 * @brief File contains method to process SORT commands.
 * 
 * syntax:
 * SORT relation_name BY column_names IN sorting_orders
 * 
 * sorting_orders = list of ASC | DESC 
 */
bool syntacticParseSORT(){
    logger.log("syntacticParseSORT");
    if(tokenizedQuery.size() < 6 || tokenizedQuery.size() % 2 != 0 || tokenizedQuery[2] != "BY"){
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }
    
    int INidx = -1; // index of the keyword "IN"
    int colCount = 0; // count of the number of columns 
    int ordCount = 0; // count of the number of sorting orders
    for(int idx = 3; idx < tokenizedQuery.size(); ++idx) {
        if(tokenizedQuery[idx] == "IN") {
            INidx = idx;
            continue;
        }
        else {
            if(INidx == -1) { // IN keyword not yet encountered
                parsedQuery.sortColumnNames.push_back(tokenizedQuery[idx]);
                ++colCount;
            }
            else { // IN keyword encountered
                if(tokenizedQuery[idx] == "ASC")
                    parsedQuery.sortingStrategy.push_back(ASC);
                else if(tokenizedQuery[idx] == "DESC")
                    parsedQuery.sortingStrategy.push_back(DESC);
                else {
                    cout<<"SYNTAX ERROR"<<endl;
                    return false;
                }
                ++ordCount;
            }
        }
    }
    if(INidx == -1 || colCount != ordCount) { // No IN in command
        cout<<"SYNTAX ERROR"<<endl;
        return false;
    }

    parsedQuery.queryType = SORT;
    parsedQuery.sortRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseSORT(){
    logger.log("semanticParseSORT");

    if(!tableCatalogue.isTable(parsedQuery.sortRelationName)){
        cout<<"SEMANTIC ERROR: Relation doesn't exist"<<endl;
        return false;
    }

    for(string &columnName: parsedQuery.sortColumnNames) {
        if(!tableCatalogue.isColumnFromTable(columnName, parsedQuery.sortRelationName)){
            cout<<"SEMANTIC ERROR: Column doesn't exist in relation"<<endl;
            return false;
        }
    }

    return true;
}

void executeSORT(){
    logger.log("executeSORT");
    Table* table = tableCatalogue.getTable(parsedQuery.sortRelationName);
    vector<int> sortingStrategy;
    for(SortingStrategy strategy: parsedQuery.sortingStrategy) {
        sortingStrategy.push_back(strategy);
    }
    table->sortTable(parsedQuery.sortColumnNames, sortingStrategy);

    cout<<"Sorted Table."<<endl;

    return;
}
