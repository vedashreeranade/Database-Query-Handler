#include "global.h"
/**
 * @brief 
 * SYNTAX: RENAME MATRIX matrix_current_name matrix_new_name
 */


bool syntacticParseRENAME_MATRIX()
{
    logger.log("syntacticParseRENAME_MATRIX");
    if (tokenizedQuery.size() != 4)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = RENAME_MATRIX;
    parsedQuery.renameCurrentMatrix = tokenizedQuery[2];
    parsedQuery.renameNewMatrix = tokenizedQuery[3];
    return true;
}

bool semanticParseRENAME_MATRIX()
{
    logger.log("semanticParseRENAME_MATRIX");

    if (!matrixCatalogue.isMatrix(parsedQuery.renameCurrentMatrix))
    {
        cout << "SEMANTIC ERROR: Matrix doesn't exist" << endl;
        return false;
    }

    return true;
}

void executeRENAME_MATRIX()
{
    logger.log("executeRENAME_MATRIX");
    
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.renameCurrentMatrix);
    matrix->rename_matrix();

    //changed the matrix name
    matrix->matrixName = parsedQuery.renameNewMatrix;
    matrixCatalogue.matrices[parsedQuery.renameNewMatrix] = matrixCatalogue.matrices[parsedQuery.renameCurrentMatrix];
    matrixCatalogue.matrices.erase(parsedQuery.renameCurrentMatrix); 
    
    // cout<< "\n\n Renaming Done! \n";

    cout<< "No. of blocks read: " << blockReadCounter << endl; 
    cout<< "No. of blocks written: " << blockWriteCounter << endl; 
    cout<< "No. of blocks accessed: " << blockReadCounter + blockWriteCounter << endl; 
    blockReadCounter = 0;
    blockWriteCounter = 0;
    return;
}
