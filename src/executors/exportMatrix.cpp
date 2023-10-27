#include "global.h"

/**
 * @brief
 * SYNTAX: EXPORT MATRIX <matrix_name>
 */

bool syntacticParseEXPORT_MATRIX() {
  logger.log("syntacticParseEXPORT_MATRIX");
  if (tokenizedQuery.size() != 3)
  {
      cout << "SYNTAX ERROR" << endl;
      return false;
  }
  parsedQuery.queryType = EXPORT_MATRIX;
  parsedQuery.exportRelationName = tokenizedQuery[2];
  return true;
}

bool semanticParseEXPORT_MATRIX() {
  logger.log("semanticParseEXPORT_MATRIX");
  //Matrix should exist
  if (matrixCatalogue.isMatrix(parsedQuery.exportRelationName))
      return true;
  cout << "SEMANTIC ERROR: No such relation exists" << endl;
  return false;
}

void executeEXPORT_MATRIX() {
  logger.log("executeEXPORT_MATRIX");
  Matrix* matrix = matrixCatalogue.getMatrix(parsedQuery.exportRelationName);

  matrix->export_matrix(parsedQuery.exportRelationName);
  
  cout<< "No. of blocks read: " << blockReadCounter << endl; 
  cout<< "No. of blocks written: " << blockWriteCounter << endl; 
  cout<< "No. of blocks accessed: " << blockReadCounter + blockWriteCounter << endl; 
  blockReadCounter = 0;
  blockWriteCounter = 0;
  return;
}
