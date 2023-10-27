#include "global.h"
/**
 * @brief 
 * SYNTAX: CHECKSYMMETRY matrix_name
 */
bool syntacticParseCHECKSYMMETRY()
{
    logger.log("syntacticParseCHECKSYMMETRY");
    if (tokenizedQuery.size() != 2)
    {
        cout << "SYNTAX ERROR" << endl;
        return false;
    }
    parsedQuery.queryType = CHECKSYMMETRY;
    parsedQuery.checkSymmetryRelationName = tokenizedQuery[1];
    return true;
}

bool semanticParseCHECKSYMMETRY()
{
    logger.log("semanticParseCHECKSYMMETRY");
    if (!matrixCatalogue.isMatrix(parsedQuery.checkSymmetryRelationName))
    {
        cout << "SEMANTIC ERROR: Relation doesn't exist" << endl;
        return false;
    }
    return true;
}

void executeCHECKSYMMETRY()
{
    logger.log("executeCHECKSYMMETRY");
    Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.checkSymmetryRelationName);
    if(matrix->checkSymmetry())
        cout<<"TRUE\n";
    else
        cout<<"FALSE\n";

    cout<< "No. of blocks read: " << blockReadCounter << endl; 
    cout<< "No. of blocks written: " << blockWriteCounter << endl; 
    cout<< "No. of blocks accessed: " << blockReadCounter + blockWriteCounter << endl; 
    blockReadCounter = 0;
    blockWriteCounter = 0;
    return;
}
