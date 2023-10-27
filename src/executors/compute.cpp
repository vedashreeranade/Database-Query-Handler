#include "global.h"
/**
 * @brief 
 * SYNTAX: COMPUTE matrix_name
 */
bool syntacticParseCOMPUTE()
{
    logger.log("syntacticParseCOMPUTE");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = COMPUTE;
    parsedQuery.computeRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseCOMPUTE()
{
    logger.log("semanticParseCOMPUTE");
    if (!matrixCatalogue.isMatrix(parsedQuery.computeRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeCOMPUTE()
{
    logger.log("executeCOMPUTE");
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.computeRelationName);
    matrix->compute();

    // parsedQuery.renameCurrentMatrix = parsedQuery.computeRelationName;
    // parsedQuery.renameNewMatrix = parsedQuery.computeRelationName + "_RESULT";
    // executeRENAME_MATRIX();
    //
    // parsedQuery.exportRelationName = parsedQuery.renameNewMatrix;
    // executeEXPORT_MATRIX();
    
    cout<< "No. of blocks read: " << blockReadCounter << endl; 
    cout<< "No. of blocks written: " << blockWriteCounter << endl; 
    cout<< "No. of blocks accessed: " << blockReadCounter + blockWriteCounter << endl; 
    blockReadCounter = 0;
    blockWriteCounter = 0;
    return;
}
