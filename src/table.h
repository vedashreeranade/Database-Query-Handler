#include "cursor.h"

enum IndexingStrategy
{
    BTREE,
    HASH,
    NOTHING
};

/**
 * @brief The Table class holds all information related to a loaded table. It
 * also implements methods that interact with the parsers, executors, cursors
 * and the buffer manager. There are typically 2 ways a table object gets
 * created through the course of the workflow - the first is by using the LOAD
 * command and the second is to use assignment statements (SELECT, PROJECT,
 * JOIN, SORT, CROSS and DISTINCT). 
 *
 */
class Table
{
    vector<unordered_set<int>> distinctValuesInColumns;

public:
    string sourceFileName = "";
    string tableName = "";
    vector<string> columns;
    vector<uint> distinctValuesPerColumnCount;
    uint columnCount = 0;
    long long int rowCount = 0;
    uint blockCount = 0;
    uint maxRowsPerBlock = 0;
    uint maxColsPerBlock = 0;
    vector<uint> rowsPerBlockCount;
    bool indexed = false;
    string indexedColumn = "";
    IndexingStrategy indexingStrategy = NOTHING;
    
    bool extractColumnNames(string firstLine);
    bool blockify();
    void updateStatistics(vector<int> row);
    Table();
    Table(string tableName);
    Table(string tableName, vector<string> columns);
    bool load();
    bool isColumn(string columnName);
    void renameColumn(string fromColumnName, string toColumnName);
    void print();
    void makePermanent();
    bool isPermanent();
    void getNextPage(Cursor *cursor);
    Cursor getCursor();
    int getColumnIndex(string columnName);
    void unload();
    void sortTable(vector<string> sortColumnNames, vector<int>sortingStrategy);
    void merge(vector<string> sortColumnNames, vector<int> sortingStrategy, 
        vector<Cursor> blockCursors, int round, int setCount);
    void join(Table* table1, Table* table2, string col1, string col2, int binaryop);
    void copy(Table* table);
    void trim(Table* table);
    void calculate(Table* table);
    void groupBy(Table* table, string aggFunc1, string aggFunc2, int binaryOp, int attrValue);

  /**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
void writeRow(vector<T> row, ostream &fout)
{
    logger.log("Table::printRow");
    for (int columnCounter = 0; columnCounter < row.size(); columnCounter++)
    {
        if (columnCounter != 0)
            fout << ", ";
        fout << row[columnCounter];
    }
    fout << endl;
}

/**
 * @brief Static function that takes a vector of valued and prints them out in a
 * comma seperated format.
 *
 * @tparam T current usaages include int and string
 * @param row 
 */
template <typename T>
void writeRow(vector<T> row)
{
    logger.log("Table::printRow");
    ofstream fout(this->sourceFileName, ios::app);
    this->writeRow(row, fout);
    fout.close();
}
};


class Matrix {
  vector<unordered_set<int>> distinctValuesInColumns;

public:
  string sourceFileName = "";
  string matrixName = "";
  vector<string> columns;
  vector<uint> distinctValuesPerColumnCount;
  uint columnCount = 0;
  long long int rowCount = 0;
  uint blockCount = 0;
  uint maxRowsPerBlock = 0;
  uint maxColsPerBlock = 0;
  uint maxBlocksPerRow = 0;
  vector<uint> rowsPerBlockCount;
  bool indexed = false;
  string indexedColumn = "";
  IndexingStrategy indexingStrategy = NOTHING;
  uint limit = 0; 

  bool getColumnCount(string firstLine);
  bool blockify();
  void updateStatistics(vector<int> row);
  Matrix();
  Matrix(string matrixName);
  Matrix(string matrixName, vector<string> columns);
  bool load();
  bool isColumn(string columnName);
  void renameColumn(string fromColumnName, string toColumnName);
  void print();
  void transpose();
  bool checkSymmetry();
  void compute();
  void makePermanent();
  void export_matrix(string sourceFileName);
  void rename_matrix();
  bool isPermanent();
  void getNextMatrixPage(MatrixCursor *cursor);
  MatrixCursor getMatrixCursor();
  int getColumnIndex(string columnName);
  void unload();

  /**
   * @brief Static function that takes a vector of valued and prints them out in
   * a comma seperated format.
   *
   * @tparam T current usages include int and string
   * @param row
   */
  template <typename T> void writeRow(vector<T> row, ostream &fout) {
    logger.log("Matrix::printRow");
    if(row.empty()) 
    {
      logger.log("Empty row");
      return;
    }
    uint limit = columnCount > 20 ? 20 : columnCount;
    logger.log(to_string(row.size()));
    logger.log(to_string(limit));
    for (int columnCounter = 0; columnCounter < limit; columnCounter++) {
      if (columnCounter != 0)
        fout << ", ";
      fout << row[columnCounter];
    }
    // fout << endl;
  }

  /**
   * @brief Static function that takes a vector of valued and prints them out in
   * a comma seperated format.
   *
   * @tparam T current usages include int and string
   * @param row
   */
  template <typename T> void writeRow(vector<T> row) {
    logger.log("Matrix::printRow");
    if(row.empty()) 
    {
      logger.log("Empty row");
      return;
    }
    ofstream fout(this->sourceFileName, ios::app);
    this->writeRow(row, fout);
    fout.close();
  }
};
