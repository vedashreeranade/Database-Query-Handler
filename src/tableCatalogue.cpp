#include "global.h"

void TableCatalogue::insertTable(Table *table) {
  logger.log("TableCatalogue::~insertTable");
  this->tables[table->tableName] = table;
}
void TableCatalogue::deleteTable(string tableName) {
  logger.log("TableCatalogue::deleteTable");
  this->tables[tableName]->unload();
  delete this->tables[tableName];
  this->tables.erase(tableName);
}
Table *TableCatalogue::getTable(string tableName) {
  logger.log("TableCatalogue::getTable");
  logger.log(tableName);
  Table *table = this->tables[tableName];
  if(!table) {
    logger.log(tableName + " NULL");
  }
  return table;
}
bool TableCatalogue::isTable(string tableName) {
  logger.log("TableCatalogue::isTable");
  if (this->tables.count(tableName))
    return true;
  return false;
}

bool TableCatalogue::isColumnFromTable(string columnName, string tableName) {
  logger.log("TableCatalogue::isColumnFromTable");
  if (this->isTable(tableName)) {
    Table *table = this->getTable(tableName);
    if (table->isColumn(columnName))
      return true;
  }
  return false;
}

void TableCatalogue::print() {
  logger.log("TableCatalogue::print");
  cout << "\nRELATIONS" << endl;

  int rowCount = 0;
  for (auto rel : this->tables) {
    cout << rel.first << endl;
    rowCount++;
  }
  printRowCount(rowCount);
}

TableCatalogue::~TableCatalogue() {
  logger.log("TableCatalogue::~TableCatalogue");
  for (auto table : this->tables) {
    table.second->unload();
    delete table.second;
  }
}

// MATRIX CATALOGUE

void MatrixCatalogue::insertMatrix(Matrix *matrix) {
  logger.log("MatrixCatalogue::~insertMatrix");
  this->matrices[matrix->matrixName] = matrix;
}
void MatrixCatalogue::deleteMatrix(string matrixName) {
  logger.log("MatrixCatalogue::deleteMatrix");
  this->matrices[matrixName]->unload();
  delete this->matrices[matrixName];
  this->matrices.erase(matrixName);
}
Matrix *MatrixCatalogue::getMatrix(string matrixName) {
  logger.log("MatrixCatalogue::getMatrix");
  // logger.log("Getting Matrix");
  // for (auto idx = this->matrices.begin(); idx != this->matrices.end(); ++idx)
  // {
  //   cout << idx->first << " " << idx->second << endl;
  // }
  // logger.log("Completed");
  Matrix *matrix = this->matrices[matrixName];
  logger.log("Got Matrix: " + matrix->matrixName);
  return matrix;
}
bool MatrixCatalogue::isMatrix(string matrixName) {
  logger.log("MatrixCatalogue::isMatrix");
  if (this->matrices.count(matrixName))
    return true;
  logger.log(matrixName + "not found in MatrixCatalogue");
  return false;
}

bool MatrixCatalogue::isColumnFromMatrix(string columnName, string matrixName) {
  logger.log("MatrixCatalogue::isColumnFromMatrix");
  if (this->isMatrix(matrixName)) {
    Matrix *matrix = this->getMatrix(matrixName);
    if (matrix->isColumn(columnName))
      return true;
  }
  return false;
}

void MatrixCatalogue::print() {
  logger.log("MatrixCatalogue::print");
  cout << "\nRELATIONS" << endl;

  int rowCount = 0;
  for (auto rel : this->matrices) {
    cout << rel.first << endl;
    rowCount++;
  }
  printRowCount(rowCount);
}

MatrixCatalogue::~MatrixCatalogue() {
  logger.log("MatrixCatalogue::~MatrixCatalogue");
  for (auto table : this->matrices) {
    table.second->unload();
    delete table.second;
  }
}
