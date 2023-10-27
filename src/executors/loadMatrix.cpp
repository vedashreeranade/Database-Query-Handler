#include "global.h"
/**
 * @brief 
 * SYNTAX: LOAD MATRIX matrix_name
 */
bool syntacticParseLOAD_MATRIX()
{
    logger.log("syntacticParseLOAD_MATRIX");
    if (tokenizedQuery.size() != 3)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = LOAD_MATRIX;
    parsedQuery.loadRelationName = tokenizedQuery[2];
    return true;
}

bool semanticParseLOAD_MATRIX()
{
    logger.log("semanticParseLOAD_MATRIX");
    if (matrixCatalogue.isMatrix(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Table already exists" << endl;
        return false;
    }

    if (!isFileExists(parsedQuery.loadRelationName))
    {
        cout << "SEMANTIC ERROR: Data file doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeLOAD_MATRIX()
{
    logger.log("executeLOAD_MATRIX");

    Matrix *matrix = new Matrix(parsedQuery.loadRelationName);
    if(matrix->load()){
        matrixCatalogue.insertMatrix(matrix);
        cout<< "Loaded Matrix. Dimensions: " << matrix->columnCount << " x " << matrix->columnCount << endl;
    }
    cout<< "No. of blocks read: " << blockReadCounter << endl; 
    cout<< "No. of blocks written: " << blockWriteCounter << endl; 
    cout<< "No. of blocks accessed: " << blockReadCounter + blockWriteCounter << endl; 
    blockReadCounter = 0;
    blockWriteCounter = 0;
    return;
}
