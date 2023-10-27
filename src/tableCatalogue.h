#include "table.h"

/**
 * @brief The TableCatalogue acts like an index of tables existing in the
 * system. Everytime a table is added(removed) to(from) the system, it needs to
 * be added(removed) to(from) the tableCatalogue. 
 *
 */
class TableCatalogue
{

    unordered_map<string, Table*> tables;

public:
    TableCatalogue() {}
    void insertTable(Table* table);
    void deleteTable(string tableName);
    Table* getTable(string tableName);
    bool isTable(string tableName);
    bool isColumnFromTable(string columnName, string tableName);
    void print();
    ~TableCatalogue();
};


class MatrixCatalogue {

public:

  unordered_map<string, Matrix *> matrices;

  MatrixCatalogue() {}
  void insertMatrix(Matrix *matrix);
  void deleteMatrix(string matrixName);
  Matrix *getMatrix(string matrixName);
  bool isMatrix(string matrixName);
  bool isColumnFromMatrix(string columnName, string matrixName);
  void print();
  ~MatrixCatalogue();
};
