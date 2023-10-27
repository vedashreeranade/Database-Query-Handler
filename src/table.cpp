#include "global.h"
#include <cmath>
#include <queue>
#include <stdexcept>
#include <string>

/**
 * @brief Construct a new Table:: Table object
 *
 */
Table::Table() { logger.log("Table::Table"); }

/**
 * @brief Construct a new Table:: Table object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param tableName
 */
Table::Table(string tableName) {
  logger.log("Table::Table");
  this->sourceFileName = "../data/" + tableName + ".csv";
  this->tableName = tableName;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName
 * @param columns
 */
Table::Table(string tableName, vector<string> columns) {
  logger.log("Table::Table");
  this->sourceFileName = "../data/temp/" + tableName + ".csv";
  this->tableName = tableName;
  this->columns = columns;
  this->columnCount = columns.size();
  this->maxRowsPerBlock =
      (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
  this->writeRow<string>(columns);
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates table
 * statistics.
 *
 * @return true if the table has been successfully loaded
 * @return false if an error occurred
 */
bool Table::load() {
  logger.log("Table::load");
  fstream fin(this->sourceFileName, ios::in);
  string line;
  if (getline(fin, line)) {
    fin.close();
    if (this->extractColumnNames(line))
      if (this->blockify())
        return true;
  }
  fin.close();
  return false;
}

/**
 * @brief Function extracts column names from the header line of the .csv data
 * file.
 *
 * @param line
 * @return true if column names successfully extracted (i.e. no column name
 * repeats)
 * @return false otherwise
 */
bool Table::extractColumnNames(string firstLine) {
  logger.log("Table::extractColumnNames");
  unordered_set<string> columnNames;
  string word;
  stringstream s(firstLine);
  while (getline(s, word, ',')) {
    word.erase(std::remove_if(word.begin(), word.end(), ::isspace), word.end());
    if (columnNames.count(word))
      return false;
    columnNames.insert(word);
    this->columns.emplace_back(word);
  }
  this->columnCount = this->columns.size();
  this->maxRowsPerBlock =
      (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
  this->maxColsPerBlock =
      (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->columnCount));
  return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */
bool Table::blockify() {
  logger.log("Table::blockify");
  ifstream fin(this->sourceFileName, ios::in);
  string line, word;
  vector<int> row(this->columnCount, 0);
  vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
  int pageCounter = 0;
  unordered_set<int> dummy;
  dummy.clear();
  this->distinctValuesInColumns.assign(this->columnCount, dummy);
  this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
  getline(fin, line);
  // cout << "Line: " << line << endl;
  while (getline(fin, line)) {
    stringstream s(line);
    for (int columnCounter = 0; columnCounter < this->columnCount;
         columnCounter++) {
      if (!getline(s, word, ','))
        return false;
      try {
        row[columnCounter] = stoi(word);
      }
      catch (const std::invalid_argument&) {
        cout << "Not a integer: |" << word << "|" << endl;
      }

      // row[columnCounter] = stoi(word);
      rowsInPage[pageCounter][columnCounter] = row[columnCounter];
    }
    pageCounter++;
    this->updateStatistics(row);
    if (pageCounter == this->maxRowsPerBlock) {
      bufferManager.writePage(this->tableName, this->blockCount, rowsInPage,
                              pageCounter);
      this->blockCount++;
      this->rowsPerBlockCount.emplace_back(pageCounter);
      pageCounter = 0;
    }
  }
  if (pageCounter) {
    bufferManager.writePage(this->tableName, this->blockCount, rowsInPage,
                            pageCounter);
    this->blockCount++;
    this->rowsPerBlockCount.emplace_back(pageCounter);
    pageCounter = 0;
  }

  if (this->rowCount == 0)
    return false;
  this->distinctValuesInColumns.clear();
  return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row
 */
void Table::updateStatistics(vector<int> row) {
  this->rowCount++;
  for (int columnCounter = 0; columnCounter < this->columnCount;
       columnCounter++) {
    if (!this->distinctValuesInColumns[columnCounter].count(
            row[columnCounter])) {
      this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
      this->distinctValuesPerColumnCount[columnCounter]++;
    }
  }
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName
 * @return true
 * @return false
 */
bool Table::isColumn(string columnName) {
  logger.log("Table::isColumn");
  for (auto col : this->columns) {
    if (col == columnName) {
      return true;
    }
  }
  return false;
}

/**
 * @brief Renames the column indicated by fromColumnName to toColumnName. It is
 * assumed that checks such as the existence of fromColumnName and the non prior
 * existence of toColumnName are done.
 *
 * @param fromColumnName
 * @param toColumnName
 */
void Table::renameColumn(string fromColumnName, string toColumnName) {
  logger.log("Table::renameColumn");
  for (int columnCounter = 0; columnCounter < this->columnCount;
       columnCounter++) {
    if (columns[columnCounter] == fromColumnName) {
      columns[columnCounter] = toColumnName;
      break;
    }
  }
  return;
}

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Table::print() {
  logger.log("Table::print");
  uint count = min((long long)PRINT_COUNT, this->rowCount);

  if(count == 0){
    cout<<"Empty table."<<endl;
    return;
  }
  // print headings
  this->writeRow(this->columns, cout);

  Cursor cursor(this->tableName, 0);
  vector<int> row;
  for (int rowCounter = 0; rowCounter < count; rowCounter++) {
    row = cursor.getNext();
    this->writeRow(row, cout);
  }
  printRowCount(this->rowCount);
}

/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Table::getNextPage(Cursor *cursor) {
  logger.log("Table::getNext");

  if (cursor->pageIndex < this->blockCount - 1) {
    cursor->nextPage(cursor->pageIndex + 1);
  }
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Table::makePermanent() {
  logger.log("Table::makePermanent");
  if (!this->isPermanent())
    bufferManager.deleteFile(this->sourceFileName);
  string newSourceFile = "../data/" + this->tableName + ".csv";
  ofstream fout(newSourceFile, ios::out);

  // print headings
  this->writeRow(this->columns, fout);

  Cursor cursor(this->tableName, 0);
  vector<int> row;
  for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++) {
    row = cursor.getNext();
    this->writeRow(row, fout);
  }
  fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Table::isPermanent() {
  logger.log("Table::isPermanent");
  if (this->sourceFileName == "../data/" + this->tableName + ".csv")
    return true;
  return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Table::unload() {
  logger.log("Table::~unload");
  for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
    bufferManager.deleteFile(this->tableName, pageCounter);
  if (!isPermanent())
    bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 *
 * @return Cursor
 */
Cursor Table::getCursor() {
  logger.log("Table::getCursor");
  Cursor cursor(this->tableName, 0);
  return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName.
 *        Assumes that the columnName is valid
 *
 * @param columnName
 * @return int
 */
int Table::getColumnIndex(string columnName) {
  logger.log("Table::getColumnIndex");
  // logger.log(to_string(this->columnCount));
  for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++) {
    if (this->columns[columnCounter] == columnName)
      return columnCounter;
  }
  logger.log("Table::getColumnIndex got incorrect columnName: " + columnName);
}

void Table::merge(vector<string> sortColumnNames, vector<int> sortingStrategy, 
    vector<Cursor> blockCursors, int round, int setCount) {

      logger.log("Table::merge");
  auto compare = [this, &sortColumnNames, &sortingStrategy](
                     pair<vector<int>, int> &X, pair<vector<int>, int> &Y) {
    // check in the order of the column names
    vector<int> A = X.first;
    vector<int> B = Y.first;
    for (int idx = 0; idx < sortColumnNames.size(); ++idx) {
      int colIdx = this->getColumnIndex(sortColumnNames[idx]);
      if (A[colIdx] != B[colIdx]) {
        if (sortingStrategy[idx] == ASC) {
          return A[colIdx] > B[colIdx];
        } else {
          return A[colIdx] < B[colIdx];
        }
      }
    }
    return true;
  };


  // initialize a priority queue with the first row from each block
  priority_queue<pair<vector<int>, int>, vector<pair<vector<int>, int>>,
                 decltype(compare)>
      min_heap(compare);
  for (int idx = 0; idx < blockCursors.size(); ++idx) {
    vector<int> row = blockCursors[idx].getNext();
    // for(int &n: row) {
    //   cout << n << " ";
    // }
    // cout << "\n";
    min_heap.push({row, idx});
  }

  // create an empty table and put all the data from the min heap
  // into the pages of this table 
  string currentTableName = this->tableName + "_" + to_string(round) + "_" + to_string(setCount);
  cout << "Creating table " << currentTableName << "\n";
  Table *resultantTable = new Table(currentTableName, this->columns);

  int idx = 0;
  while (!min_heap.empty()) {
    // cout << "Min heap size: " << min_heap.size() << "\n";
    vector<int> row = min_heap.top().first;
    int idx = min_heap.top().second;
    min_heap.pop();

    // for(int &n: row) {
    //   cout << n << " ";
    // }
    // cout << "\n";
    resultantTable->writeRow(row);
    // get the next row from the cursor at idx
    row = blockCursors[idx].getNext();

    if(!row.empty()) {
      min_heap.push({row, idx});
    }
  }

  resultantTable->blockify();
  tableCatalogue.insertTable(resultantTable);
}

void Table::sortTable(vector<string> sortColumnNames, vector<int> sortingStrategy) {
  logger.log("Table::sort");

  Cursor cursor = this->getCursor();
  vector<int> row;
  // load the values onto tableData
  vector<vector<int>> tableData;
  
  for (int idx = 0; idx < this->blockCount; ++idx) {
    // // load the values onto tableData
    // vector<vector<int>> tableData;
    logger.log(to_string(idx));

    int count = this->rowsPerBlockCount[idx];
    while (count > 0) {
        row = cursor.getNext();      
        tableData.push_back(row);
      --count;
    }

    // sort the tableData 2D vector
    ::stable_sort(tableData.begin(), tableData.end(), 
            [this, &sortColumnNames, &sortingStrategy](const vector<int> &A, const vector<int> &B) {
             // check in the order of the column names
            
             for (int idx = 0; idx < (int)sortColumnNames.size(); idx++) {
               int colIdx = this->getColumnIndex(sortColumnNames[idx]);
             
               if (A[colIdx] != B[colIdx]) {
                //  logger.log("in if");
                 if (sortingStrategy[idx] == ASC) {
                   return A[colIdx] < B[colIdx];
                 } else {
                   return A[colIdx] > B[colIdx];
                 }
               }
             }
             
            return false;
           });

    logger.log("Table::sort: Writing sorted page back into memory");


    if(this->blockCount == 1) {// no merging step needed
      // overwrite contents of table page
      bufferManager.writePage(this->tableName, 0, tableData, (int)tableData.size());
      // remove the page from the cache so that it is loaded again 
      bufferManager.removeFromPool(this->tableName, 0);
      return;
    }
    else {
      // create an empty table and put all the data
      // into the pages of this table 
      string currentTableName = this->tableName + "_0_" + to_string(idx);
      Table *resultantTable = new Table(currentTableName, this->columns);

      for(vector<int> &row: tableData) {
        resultantTable->writeRow(row);
      }

      resultantTable->blockify();
      tableCatalogue.insertTable(resultantTable);
    }
    tableData.clear();
  }

  logger.log("Table::sort: Internal sorting complete");
  logger.log("Table::sort: Merging pages");
  // NOTE: we need to merge the internally sorted pages using K-way mergesort
  // Since we have 10 blocks in total, we will use 9 for reading blocks
  // and 1 for writing block
  int rounds = ceil(log(this->blockCount) / log(9));
  int set_size = 1;
  int maxRowsPerBlock = this->rowsPerBlockCount[0];
  int K = 9;
  
  logger.log("Table::sort: Rounds: " + to_string(rounds) + " | maxRowsPerBlock: " + to_string(maxRowsPerBlock));

  for (int r = 1; r <= rounds; ++r) {
    // cout << "Round" << r << ": \n";
    int sets = ceil((1.0 * this->blockCount) / set_size);
    // cout << this->blockCount << " | " << set_size << " | Sets: " << sets
    //      << "\n";
    int blockCount = 0;
    int setCount = 0;
    // get the cursor from each set (upto K cursors at a time)
    vector<Cursor> blockCursors;
    for (int s = 0; s < sets; ++s) {
      string prevTableName = this->tableName + "_" + to_string(r - 1) + "_" + to_string(s);
      // cout << "Getting cursor for " << prevTableName + "_Page0" << "\n";

      Cursor cursor = Cursor(prevTableName, 0);
      blockCursors.push_back(cursor);

      if ((int)blockCursors.size() == K) {
        // cout << "Got K cursors, now start merging\n";
        logger.log("Got K cursors, now start merging\n");

        this->merge(sortColumnNames, sortingStrategy, blockCursors, r, setCount);

        // cout << "Deleting the previously read tables\n";
        logger.log("Deleting the previously read tables\n");

        for (int id = s; id > s - (int)blockCursors.size(); --id) {
          string prevTableName =  this->tableName + "_" + to_string(r - 1) + "_" + to_string(id);
          // cout << "Deleting table: " << prevTableName << "\n";
          logger.log("Deleting table block cursor size == k: " + prevTableName + "\n");
          // tableCatalogue.getTable(prevTableName)->unload();
          int pageIdx = 0;
          while(bufferManager.removeFromPool(prevTableName, pageIdx)) {
            ++pageIdx;
          }
          tableCatalogue.deleteTable(prevTableName);
          bufferManager.deleteFile(prevTableName);
        }

        ++setCount;
        // clear the blockCursors vector
        blockCursors.clear();
      }
    }
    if (!blockCursors.empty()) {
      // cout << "Merging remaining pages\n";
      logger.log("Merging remaining pages\n");

      this->merge(sortColumnNames, sortingStrategy, blockCursors, r, setCount);
      
      for (int id = sets - 1; id >= sets - (int)blockCursors.size(); --id) {
        string prevTableName =  this->tableName + "_" + to_string(r - 1) + "_" + to_string(id);
        // cout << "Deleting table: " << prevTableName << "\n";
        logger.log("Deleting table block cursor not empty: " + prevTableName + "\n");
        // tableCatalogue.getTable(prevTableName)->unload();
        int pageIdx = 0;
        while(bufferManager.removeFromPool(prevTableName, pageIdx)) {
          ++pageIdx;
        }
        tableCatalogue.deleteTable(prevTableName);
        bufferManager.deleteFile(prevTableName);
      }
      
      ++setCount;
      // clear the blockCursors vector
      blockCursors.clear();
    }
    set_size *= K;
    logger.log("Done\n");
    // cout << "Done\n";
  }

  // cout << "Writing the final sorted pages into memory" << endl;

  // TODO: replace the original pages with the pages of the final round
  // (Maybe rename these pages directly to reduce block access)
  
  string currentTableName = this->tableName + "_" + to_string(rounds) + "_0";
  cursor = tableCatalogue.getTable(currentTableName)->getCursor();
  // cout << "Creating table " << currentTableName << "\n";
  //
  // cout << "Total blocks: " << this->blockCount << endl;
  
  // cursor.getNext(); // skip the first row containing header
  row = cursor.getNext();
  for (int idx = 0; idx < this->blockCount; ++idx) {
    // Cursor cursor = bufferManager.getPage(currentTableName, idx);
    
    // cout << "Block: " << idx << endl;
    // cout << "Row Count: " <<  this->rowsPerBlockCount[idx] << "\n";
    // load the values onto tableData
    vector<vector<int>> tableData;
    int count = this->rowsPerBlockCount[idx];
    while (count > 0) {
      if(!row.empty()) {
        tableData.push_back(row);
        row = cursor.getNext();
      }
      else{
        cout << "Empty row\n";
        break;
      }
      --count;
    }

    // cout << "Table Size: " << tableData.size() << endl;
    // overwrite the contents of the current table pages 
    bufferManager.writePage(this->tableName, idx, tableData, (int)tableData.size());

    // remove the page from the cache so that it is loaded again 
    bufferManager.removeFromPool(this->tableName, idx);
    // cout << idx << "Done" << endl;
  }

  logger.log("Table::sort: External sorting complete");
  
  int pageIdx = 0;
  while(bufferManager.removeFromPool(currentTableName, pageIdx)) {
    ++pageIdx;
  }
  tableCatalogue.deleteTable(currentTableName);
  bufferManager.deleteFile(currentTableName);
}

void Table::join(Table* table1, Table* table2, string col1, string col2, int binaryop){
  logger.log("Table::join");

  map<int, BinaryOperator> mp = 
  {
    {0, LESS_THAN},
    {1, GREATER_THAN},
    {2, LEQ},
    {3, GEQ},
    {4, EQUAL},
    {5, NOT_EQUAL},
    {6, NO_BINOP_CLAUSE}
  };

  vector<int> resultantRow;
  resultantRow.reserve(this->columnCount);

  Cursor cursor1 = table1->getCursor();
  Cursor cursor2 = table2->getCursor();

  vector<int> row1 = cursor1.getNext();
  vector<int> row2 = cursor2.getNext();

  int ind1 = table1->getColumnIndex(col1);
  int ind2 = table2->getColumnIndex(col2);

  // EQUAL
  if(binaryop == 4){
    logger.log("Equal join");
    while (!row1.empty() && !row2.empty() ){
      if(evaluateBinOp(row1[ind1], row2[ind2], GREATER_THAN))
      {
        row2 = cursor2.getNext();
      }
      else if(evaluateBinOp(row1[ind1], row2[ind2], mp[binaryop]))
      {
        resultantRow = row1;
        resultantRow.insert(resultantRow.end(), row2.begin(), row2.end());

        this->writeRow<int>(resultantRow);
        row2 = cursor2.getNext();
      }
      else
      {
        row1 = cursor1.getNext();
      }
    }
  }
  // LESS THAN
  else if(binaryop == 0){
    logger.log("Less than join");
     
     while (!row1.empty() && !row2.empty() ){
      if(evaluateBinOp(row1[ind1], row2[ind2], GEQ))
        {
          row2 = cursor2.getNext();
        }
        else 
        {
          Cursor cursor3 = cursor2;
          vector<int> row3 = row2;

          while(!row3.empty())
          {
            resultantRow = row1;
            resultantRow.insert(resultantRow.end(), row3.begin(), row3.end());

            logger.log("Printing res row:");
            string temp = "";
            for(int i=0; i<resultantRow.size(); i++)
              temp += to_string(resultantRow[i]);
            logger.log(temp);

            this->writeRow<int>(resultantRow);
            row3 = cursor3.getNext();
          }
          row1 = cursor1.getNext();
        }
    }
  }
  // LESS THAN EQUAL TO
  else if(binaryop == 2){
    logger.log("Less than equal join");
     
     while (!row1.empty() && !row2.empty() ){
      if(evaluateBinOp(row1[ind1], row2[ind2], GREATER_THAN))
        {
          row2 = cursor2.getNext();
        }
        else 
        {
          Cursor cursor3 = cursor2;
          vector<int> row3 = row2;

          while(!row3.empty())
          {
            resultantRow = row1;
            resultantRow.insert(resultantRow.end(), row3.begin(), row3.end());

            logger.log("Printing res row:");
            string temp = "";
            for(int i=0; i<resultantRow.size(); i++)
              temp += to_string(resultantRow[i]);
            logger.log(temp);

            this->writeRow<int>(resultantRow);
            row3 = cursor3.getNext();
          }
          row1 = cursor1.getNext();
        }
    }
  }
  // GREATER THAN 
  else if(binaryop == 1){
    logger.log("Greater than join");
     
     while (!row1.empty() && !row2.empty() ){
      if(evaluateBinOp(row1[ind1], row2[ind2], LEQ))
        {
          row2 = cursor2.getNext();
        }
        else 
        {
          Cursor cursor3 = cursor2;
          vector<int> row3 = row2;

          while(!row3.empty())
          {
            resultantRow = row1;
            resultantRow.insert(resultantRow.end(), row3.begin(), row3.end());

            logger.log("Printing res row:");
            string temp = "";
            for(int i=0; i<resultantRow.size(); i++)
              temp += to_string(resultantRow[i]);
            logger.log(temp);

            this->writeRow<int>(resultantRow);
            row3 = cursor3.getNext();
          }
          row1 = cursor1.getNext();
        }
    }  
  }
  // GREATER THAN EQUAL TO
  else if(binaryop == 3){
    logger.log("Greater than equal join");
     
     while (!row1.empty() && !row2.empty() ){
      if(evaluateBinOp(row1[ind1], row2[ind2], LESS_THAN))
        {
          row2 = cursor2.getNext();
        }
        else 
        {
          Cursor cursor3 = cursor2;
          vector<int> row3 = row2;

          while(!row3.empty())
          {
            resultantRow = row1;
            resultantRow.insert(resultantRow.end(), row3.begin(), row3.end());

            logger.log("Printing res row:");
            string temp = "";
            for(int i=0; i<resultantRow.size(); i++)
              temp += to_string(resultantRow[i]);
            logger.log(temp);

            this->writeRow<int>(resultantRow);
            row3 = cursor3.getNext();
          }
          row1 = cursor1.getNext();
        }
    }
  }
  // NOT EQUAL
  else if(binaryop == 5){
    logger.log("Not equal join");
     
      while (!row1.empty())
      {
        cursor2 = table2->getCursor();
        row2 = cursor2.getNext();
        while (!row2.empty())
        { 
            if(evaluateBinOp(row1[ind1], row2[ind2], NOT_EQUAL)){
              resultantRow = row1;
              resultantRow.insert(resultantRow.end(), row2.begin(), row2.end());
              this->writeRow<int>(resultantRow);
            }
            row2 = cursor2.getNext();
        }
        row1 = cursor1.getNext();
      }
  }

  this->blockify();

}

/**
 * @brief Copy contents from original table to new table
 *
 */
void Table::copy(Table* table){
  logger.log("Table::copy");

  Cursor cursor = table->getCursor();
  vector<int> row = cursor.getNext();

  while(!row.empty()){
    this->writeRow<int>(row);
    row = cursor.getNext();
  }

  this->blockify();

  // delete csv file
  string filename = "../data/temp/" + this->tableName + ".csv";
  logger.log("csv file to be deleted: " + filename);
  if(remove(filename.c_str()) == 0)
    logger.log("csv file deleted successfully");
  else
    logger.log("csv file deleteion unsuccessful");
}

/**
 * @brief Trim the table such that it contains only the required columns
 *
 */
void Table::trim(Table* table){
  logger.log("Table::trim");

  vector<string> cols = this->columns;
  vector<int> indices;
  for(int i=0; i<cols.size(); i++)
    indices.push_back(table->getColumnIndex(cols[i]));

  Cursor cursor = table->getCursor();
  vector<int> row = cursor.getNext();

  while(!row.empty()){
      vector<int> resultantRow;
      resultantRow.reserve(this->columnCount);

      for(int i=0; i<indices.size(); i++)
        resultantRow.push_back(row[indices[i]]);

      this->writeRow<int>(resultantRow);
      row = cursor.getNext();
  }

  this->blockify();
}

void Table::calculate(Table* table){
  logger.log("Table::calculate");

  Cursor cursor = table->getCursor();
  vector<int> row = cursor.getNext();

  while(!row.empty()){
      int curr = row[0];
      int count = 1;
      int mini = row[1];
      int maxi = row[1];
      int sum = row[1];

      row = cursor.getNext();

      while(!row.empty() && curr == row[0]){
        count++;
        mini = min(mini, row[1]);
        maxi = max(maxi, row[1]);
        sum += row[1];

        row = cursor.getNext();
      }

      vector<int> resultantRow = {curr, maxi, mini, sum, count};
      this->writeRow<int>(resultantRow);
  }

  this->blockify();
}

void Table::groupBy(Table* table, string aggFunc1, string aggFunc2, int binaryOp, int attrValue){
  logger.log("Table::groupBy"); 

  map<int, BinaryOperator> mp = 
  {
    {0, LESS_THAN},
    {1, GREATER_THAN},
    {2, LEQ},
    {3, GEQ},
    {4, EQUAL},
    {5, NOT_EQUAL},
    {6, NO_BINOP_CLAUSE}
  };

  Cursor cursor = table->getCursor();
  vector<int> row = cursor.getNext();

  while(!row.empty()){
    int val = 0;
    if(aggFunc1 == "MAX") val = row[1];
    else if(aggFunc1 == "MIN") val = row[2];
    else if(aggFunc1 == "SUM") val = row[3];
    else if(aggFunc1 == "COUNT") val = row[4];
    else  val = row[3]/row[4];
    
    if(evaluateBinOp(val, attrValue, mp[binaryOp])){
      vector<int> resultantRow;
      resultantRow.push_back(row[0]);

      if(aggFunc2 == "MAX") resultantRow.push_back(row[1]);
      else if(aggFunc2 == "MIN") resultantRow.push_back(row[2]);
      else if(aggFunc2 == "SUM") resultantRow.push_back(row[3]);
      else if(aggFunc2 == "COUNT") resultantRow.push_back(row[4]);
      else  resultantRow.push_back(row[3]/row[4]);

      this->writeRow<int>(resultantRow);
    }

    row = cursor.getNext();
  }

  this->blockify();
}

// MATRIX IMPLEMENTATION

/**
 * @brief Construct a new Matrix:: Matrix object
 *
 */
Matrix::Matrix() { logger.log("Matrix::Matrix"); }

/**
 * @brief Construct a new Matrix:: Matrix object used in the case where the data
 * file is available and LOAD command has been called. This command should be
 * followed by calling the load function;
 *
 * @param matrixName
 */
Matrix::Matrix(string matrixName) {
  logger.log("Matrix::Matrix");
  this->sourceFileName = "../data/" + matrixName + ".csv";
  this->matrixName = matrixName;
}

/**
 * @brief Construct a new Table:: Table object used when an assignment command
 * is encountered. To create the table object both the table name and the
 * columns the table holds should be specified.
 *
 * @param tableName
 * @param columns
 */
Matrix::Matrix(string matrixName, vector<string> columns) {
  logger.log("Matrix::Matrix for Assignment");
  this->sourceFileName = "../data/temp/" + matrixName + ".csv";
  this->matrixName = matrixName;
  this->columns = columns;
  this->columnCount = columns.size();
  this->maxRowsPerBlock =
      (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * columnCount));
  this->writeRow<string>(columns);
}

/**
 * @brief The load function is used when the LOAD command is encountered. It
 * reads data from the source file, splits it into blocks and updates matrix
 * statistics.
 *
 * @return true if the table has been successfully loaded
 * @return false if an error occurred
 */
bool Matrix::load() {
  logger.log("Matrix::load");
  fstream fin(this->sourceFileName, ios::in);
  string line;
  if (getline(fin, line)) {
    fin.close();
    if (this->getColumnCount(line))
      if (this->blockify())
        return true;
  }
  fin.close();
  return false;
}

/**
 * @brief Function extracts column count from the header line of the .csv data
 * file.
 *
 * @param line
 * @return true if column count is non-zero
 * @return false otherwise
 */
bool Matrix::getColumnCount(string firstLine) {
  logger.log("Matrix::getColumnCount");
  string word;
  stringstream s(firstLine);
  while (getline(s, word, ',')) {
    this->columnCount += 1;
  }
  this->limit = this->columnCount > 20 ? 20 : this->columnCount;
  // this->columnCount = this->columns.size();
  this->maxColsPerBlock = 15;
  // (uint)(  floor(  sqrt((BLOCK_SIZE * 1000) / (sizeof(int)))  ));
  this->maxRowsPerBlock = 15;
  // (uint)((BLOCK_SIZE * 1000) / (sizeof(int) * this->maxColsPerBlock));
  this->maxBlocksPerRow =
      (uint)(ceil((float)(this->columnCount) / this->maxColsPerBlock));
  logger.log("maxColsPerBlock: " + to_string(this->maxColsPerBlock));
  logger.log("maxRowsPerBlock: " + to_string(this->maxRowsPerBlock));
  logger.log("maxBlocksPerRow: " + to_string(this->maxBlocksPerRow));
  return true;
}

/**
 * @brief This function splits all the rows and stores them in multiple files of
 * one block size.
 *
 * @return true if successfully blockified
 * @return false otherwise
 */

bool Matrix::blockify() {
  logger.log("Matrix::blockify");
  ifstream fin(this->sourceFileName, ios::in);
  string line, word;
  int dummy = 0;
  // vector<int> row(this->columnCount, 0);
  // vector<vector<int>> rowsInPage(this->maxRowsPerBlock, row);
  vector<vector<vector<int>>> pageData(
      this->maxBlocksPerRow,
      vector<vector<int>>(this->maxRowsPerBlock,
                          vector<int>(this->maxColsPerBlock, dummy)));

  int rowCounter = 0;
  int pageCounter = 0;
  // unordered_set<int> dummy;
  // dummy.clear();
  // this->distinctValuesInColumns.assign(this->columnCount, dummy);
  // this->distinctValuesPerColumnCount.assign(this->columnCount, 0);
  while (getline(fin, line)) {
    stringstream s(line);

    for (int columnCounter = 0; columnCounter < this->columnCount;
         columnCounter++) {
      if (!getline(s, word, ','))
        return false;

      // page no, row no, col no
      try {  
        pageData[columnCounter / this->maxColsPerBlock][rowCounter]
                [columnCounter % this->maxColsPerBlock] = stoi(word);
      }
      catch (const std::invalid_argument&) {
        cout << "Word is: |" << word <<"|" << endl;
      }
      // pageData[columnCounter / this->maxColsPerBlock][rowCounter]
      //         [columnCounter % this->maxColsPerBlock] = stoi(word);
    }
    ++rowCounter;

    if (rowCounter == this->maxRowsPerBlock) { // the pages are filled, to write
                                               // them onto memory
      for (int idx = 0; idx < this->maxBlocksPerRow; ++idx) {
        bufferManager.writeMatrixPage(this->matrixName, this->blockCount,
                                      this->maxBlocksPerRow, pageData[idx],
                                      this->maxRowsPerBlock);
        this->blockCount++;
        // this->rowsPerBlockCount.emplace_back(pageCounter);
        logger.log("Page " + to_string(blockCount) + " Stored");
      }
      rowCounter = 0;
      // fill the pages with default values
      fill(pageData.begin(), pageData.end(),
           vector<vector<int>>(this->maxRowsPerBlock,
                               vector<int>(this->maxColsPerBlock, dummy)));
    }
  }

  if (rowCounter > 0) {
    // some pages are partially filled, to write them onto memory
    for (int idx = 0; idx < this->maxBlocksPerRow; ++idx) {
      bufferManager.writeMatrixPage(this->matrixName, this->blockCount,
                                    this->maxBlocksPerRow, pageData[idx],
                                    this->maxRowsPerBlock);
      this->blockCount++;
      // this->rowsPerBlockCount.emplace_back(pageCounter);
      logger.log("Page " + to_string(blockCount) + " Stored");
    }
    rowCounter = 0;
  }

  if (this->columnCount == 0)
    return false;
  return true;
}

/**
 * @brief Given a row of values, this function will update the statistics it
 * stores i.e. it updates the number of rows that are present in the column and
 * the number of distinct values present in each column. These statistics are to
 * be used during optimisation.
 *
 * @param row
 */
void Matrix::updateStatistics(vector<int> row) {
  this->rowCount++;
  for (int columnCounter = 0; columnCounter < this->columnCount;
       columnCounter++) {
    if (!this->distinctValuesInColumns[columnCounter].count(
            row[columnCounter])) {
      this->distinctValuesInColumns[columnCounter].insert(row[columnCounter]);
      this->distinctValuesPerColumnCount[columnCounter]++;
    }
  }
}

/**
 * @brief Checks if the given column is present in this table.
 *
 * @param columnName
 * @return true
 * @return false
 */
bool Matrix::isColumn(string columnName) {
  logger.log("Matrix::isColumn");
  for (auto col : this->columns) {
    if (col == columnName) {
      return true;
    }
  }
  return false;
}

// /**
//  * @brief Renames the column indicated by fromColumnName to toColumnName. It
//  is
//  * assumed that checks such as the existence of fromColumnName and the non
//  prior
//  * existence of toColumnName are done.
//  *
//  * @param fromColumnName
//  * @param toColumnName
//  */
// void Matrix::renameColumn(string fromColumnName, string toColumnName) {
//   logger.log("Matrix::renameColumn");
//   for (int columnCounter = 0; columnCounter < this->columnCount;
//        columnCounter++) {
//     if (columns[columnCounter] == fromColumnName) {
//       columns[columnCounter] = toColumnName;
//       break;
//     }
//   }
//   return;
// }

/**
 * @brief Function prints the first few rows of the table. If the table contains
 * more rows than PRINT_COUNT, exactly PRINT_COUNT rows are printed, else all
 * the rows are printed.
 *
 */
void Matrix::print() {
  logger.log("Matrix::print");
  uint print_limit = min((uint)PRINT_COUNT, this->columnCount);
  logger.log(to_string(print_limit));

  vector<vector<vector<int>>> pageData(
      this->maxBlocksPerRow,
      vector<vector<int>>(this->maxRowsPerBlock,
                          vector<int>(this->maxColsPerBlock)));

  // int rowCounter = 0;
  // int pageCounter = 0;

  MatrixCursor *cursor;

  logger.log("Total Indices: " + to_string(this->maxBlocksPerRow));
  logger.log(to_string(blockCount) + "  |  " +
             to_string(this->maxBlocksPerRow));
  vector<int> row;

  // int limit = this->blockCount / this->maxBlocksPerRow;
  int pageCounter = 0;
  int accessCounter = 0;

  // get the content page wise for a row of blocks
  for (int idx = 0; idx < this->blockCount; ++idx) {
    logger.log("Getting cursor");
    cursor = new MatrixCursor(this->matrixName, this->maxBlocksPerRow, idx);
    accessCounter++;
    logger.log("Got cursor");
    for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock; rowCounter++) {
      row = cursor->getNext();
      logger.log("Got row : " + to_string(rowCounter));
      string s = "";
      if (row.empty()) {
        logger.log("Empty row");
      }
      for (auto it : row) {
        s += to_string(it) + " ";
      }
      logger.log(s);
      pageData[idx % this->maxBlocksPerRow][rowCounter] = row;
    }

    pageCounter++;
    if (pageCounter == this->maxBlocksPerRow) {
      // cout << pageCounter << endl;
      // print the content from the row of blocks
      for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
           rowCounter++) {
        for (int pageIdx = 0; pageIdx < this->maxBlocksPerRow; pageIdx++) {
          for (int columnCounter = 0; columnCounter < this->maxColsPerBlock;
               columnCounter++) {
            if (pageIdx * this->maxColsPerBlock + columnCounter >=
                print_limit) {
              break;
            }
            cout << pageData[pageIdx][rowCounter][columnCounter];
            if (pageIdx != this->maxBlocksPerRow - 1 ||
                columnCounter != this->maxColsPerBlock - 1) {
              cout << ", ";
            }
          }
        }
        cout << endl;
        if ((idx / this->maxBlocksPerRow) * this->maxRowsPerBlock +
                rowCounter >=
            print_limit - 1) {
          break;
        }
      }
      pageCounter = 0;
      // fill the pages with default values
      fill(pageData.begin(), pageData.end(),
           vector<vector<int>>(this->maxRowsPerBlock,
                               vector<int>(this->maxColsPerBlock, 0)));
    }
  }
  logger.log("Stored data");

  printRowCount(this->columnCount);
  // printBlockAccess(accessCounter);
  // printRowCount(print_limit); // printing no of rows visible and not actual
  // no of rows
}

void Matrix::rename_matrix() {

  // TOOK ALL THE FILES FROM /data/temp DIRECTORY
  const char *command = "ls ../data/temp/"; // Replace with your desired command
  vector<string> outputLines;

  FILE *pipe = popen(command, "r");
  if (!pipe) {
    cerr << "Error executing command." << endl;
    return;
  }

  char buffer[128];
  while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
    // Remove the newline character at the end of each line
    buffer[strcspn(buffer, "\n")] = '\0';
    outputLines.push_back(buffer);
  }

  pclose(pipe); // Close the pipe

  // store required file and its new name in map
  unordered_map<string, string> mp;

  for (string &line : outputLines) {
    string oldfile = "", rem = "";
    int i = 0;
    while (line[i] != '_') {
      oldfile += line[i];
      i++;
    }
    if (oldfile == parsedQuery.renameCurrentMatrix) {
      while (line[i] != '\0') {
        rem += line[i];
        i++;
      }

      oldfile = parsedQuery.renameNewMatrix;
      string newfile = oldfile + rem;
      mp[parsedQuery.renameCurrentMatrix + rem] = newfile;
    }
  }

  // OPEN DIR TO CHANGE NAME
  FILE *pipe1 = popen(command, "r");
  if (!pipe1) {
    cerr << "Error executing command." << endl;
    return;
  }

  char buffer1[128];
  while (fgets(buffer1, sizeof(buffer1), pipe) != nullptr) {
    // Remove the newline character at the end of each line
    buffer1[strcspn(buffer1, "\n")] = '\0';
    // outputLines.push_back(buffer1);
    string oldname(buffer1);
    if (mp.find(oldname) != mp.end()) {
      // logger.log("renaming " + oldname + "-> " + mp[oldname]);

      string srcpath = "../data/temp/" + oldname;
      string despath = "../data/temp/" + mp[oldname];
      if (rename(srcpath.c_str(), despath.c_str())) {
        perror("Error Renaming ");
        return;
      }
    }
  }

  pclose(pipe); // Close the pipe
}

void Matrix::export_matrix(string sourceFileName) {
  logger.log("Matrix::export_matrix");
  // uint print_limit = min((uint)PRINT_COUNT, this->columnCount);
  // logger.log(to_string(print_limit));

  vector<vector<vector<int>>> pageData(
      this->maxBlocksPerRow,
      vector<vector<int>>(this->maxRowsPerBlock,
                          vector<int>(this->maxColsPerBlock)));

  // int rowCounter = 0;
  // int pageCounter = 0;
  MatrixCursor *cursor;

  logger.log("Total Indices: " + to_string(this->maxBlocksPerRow));
  logger.log(to_string(blockCount) + "  |  " +
             to_string(this->maxBlocksPerRow));
  vector<int> row;
  string destPath = "../data/" + sourceFileName + ".csv";

  logger.log("File path in export:" + destPath);

  ofstream outputFile(destPath);
  if (!outputFile.is_open()) {
    logger.log("Error: Unable to open the file.");
    return; // Exit the program with an error code
  }
  // int limit = this->blockCount / this->maxBlocksPerRow;
  int pageCounter = 0;
  int accessCounter = 0;

  // get the content page wise for a row of blocks
  for (int idx = 0; idx < this->blockCount; ++idx) {
    logger.log("Getting cursor");
    cursor = new MatrixCursor(this->matrixName, this->maxBlocksPerRow, idx);
    accessCounter++;
    logger.log("Got cursor");
    for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock; rowCounter++) {
      row = cursor->getNext();
      logger.log("Got row : " + to_string(rowCounter));
      pageData[idx % this->maxBlocksPerRow][rowCounter] = row;
    }

    pageCounter++;
    if (pageCounter == this->maxBlocksPerRow) {
      // cout << pageCounter << endl;
      // print the content from the row of blocks
      for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
           rowCounter++) {
        for (int pageIdx = 0; pageIdx < this->maxBlocksPerRow; pageIdx++) {
          for (int columnCounter = 0; columnCounter < this->maxColsPerBlock;
               columnCounter++) {
            if (pageIdx * this->maxColsPerBlock + columnCounter >=
                this->columnCount) {
              break;
            }
            // cout << pageData[pageIdx][rowCounter][columnCounter];
            outputFile << to_string(
                pageData[pageIdx][rowCounter][columnCounter]);
            if (pageIdx != this->maxBlocksPerRow - 1 ||
                columnCounter != this->maxColsPerBlock - 1) {
              // cout << ", ";
              outputFile << ", ";
            }
          }
        }
        // cout << endl;
        outputFile << "\n";
        if ((idx / this->maxBlocksPerRow) * this->maxRowsPerBlock +
                rowCounter >=
            this->columnCount - 1) {
          break;
        }
      }
      pageCounter = 0;
      // fill the pages with default values
      fill(pageData.begin(), pageData.end(),
           vector<vector<int>>(this->maxRowsPerBlock,
                               vector<int>(this->maxColsPerBlock, 0)));
    }
  }
  logger.log("Stored data");
  outputFile.close();

  printBlockAccess(accessCounter);
  // cout<< "\n\n Export Done! \n";
}

void writeOutputIntoFile(string destPath, vector<vector<int>> &data) {

  logger.log("Writing Output into : " + destPath);

  std::ofstream outputFile(destPath.c_str(), ios::out);

  // Check if the file opened successfully
  if (!outputFile.is_open()) {
    cerr << "Error opening the file!" << std::endl;
    return;
  }

  // Write data to the file
  for (int i = 0; i < data.size(); i++) {
    for (int j = 0; j < data[0].size(); j++) {
      outputFile << data[i][j] << " ";
    }
    outputFile << "\n";
  }

  // Close the file
  outputFile.close();
}

/**
 * @brief Function does the transpose of matrix.
 *
 */
void Matrix::transpose() {

  logger.log("Matrix::transpose");

  // this->maxBlocksPerRow = (uint)(ceil((float)(this->columnCount) /
  // this->maxColsPerBlock));
  // // MatrixCursor* cursor;

  // for(int i=0; i<this->maxBlocksPerRow; i++)
  // {
  //   for(int j=i; j<this->maxBlocksPerRow; j++)
  //   {
  //     if(i == j)
  //     {

  //       int pageId = (i * this->maxBlocksPerRow) + j;

  //       logger.log("Transposing page " + to_string(pageId));

  //       MatrixPage matrixPage = bufferManager.getMatrixPage(this->matrixName,
  //       this->maxBlocksPerRow, pageId); vector<vector<int>> data =
  //       matrixPage.getAllRows();

  //       string destPath = "../data/temp/" + matrixName + "_MatrixPage" +
  //       to_string(i) + "_" + to_string(j); logger.log("destPath: " +
  //       destPath);

  //       for(int r=0; r<data.size(); r++)
  //         for(int c=r+1; c<data[0].size(); c++)
  //           swap(data[r][c], data[c][r]);

  //       writeOutputIntoFile(destPath, data);
  //     }
  //     else
  //     {
  //       int pageId1 = (i * this->maxBlocksPerRow) + j;
  //       int pageId2 = (j * this->maxBlocksPerRow) + i;

  //       logger.log("Transposing page " + to_string(pageId1));

  //       MatrixPage matrixPage1 =
  //       bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow,
  //       pageId1); vector<vector<int>> data1 = matrixPage1.getAllRows();

  //       MatrixPage matrixPage2 =
  //       bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow,
  //       pageId2); vector<vector<int>> data2 = matrixPage2.getAllRows();

  //       string destPath1 = "../data/temp/" + matrixName + "_MatrixPage" +
  //       to_string(i) + "_" + to_string(j); logger.log("destPath: " +
  //       destPath1);

  //       logger.log("Transposing page " + to_string(pageId2));

  //       string destPath2 = "../data/temp/" + matrixName + "_MatrixPage" +
  //       to_string(j) + "_" + to_string(i); logger.log("destPath: " +
  //       destPath2);

  //       for(int r=0; r<data1.size(); r++)
  //         for(int c=r+1; c<data1[0].size(); c++)
  //           swap(data1[r][c], data1[c][r]);

  //       writeOutputIntoFile(destPath2, data1);

  //       for(int r=0; r<data2.size(); r++)
  //         for(int c=r+1; c<data2[0].size(); c++)
  //           swap(data2[r][c], data2[c][r]);

  //       writeOutputIntoFile(destPath1, data2);

  //     }
  //   }
  // }

  this->maxBlocksPerRow =
      (uint)(ceil((float)(this->columnCount) / this->maxColsPerBlock));
  // MatrixCursor* cursor;

  for (int i = 0; i < this->maxBlocksPerRow; i++) {
    for (int j = i; j < this->maxBlocksPerRow; j++) {
      if (i == j) {

        int pageId = (i * this->maxBlocksPerRow) + j;

        logger.log("Transposing page " + to_string(pageId));

        MatrixCursor *cursor =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId);

        vector<int> row;
        vector<vector<int>> data(this->maxRowsPerBlock,
                                 vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor->getNext();
          if (!row.empty()) {
            data[rowCounter] = row;
          }
        }

        // cout << "Start\n";
        // for(int r = 0; r < data.size(); ++r) {
        //   // cout << data[r].size() << endl;
        //   for(int c = 0; c < data[0].size(); ++c) {
        //     cout << data[r][c] << " ";
        //   }
        //   cout << endl;
        // }
        // cout << "Done\n";
        //
        for (int r = 0; r < data.size(); r++)
          for (int c = r + 1; c < data[0].size(); c++)
            swap(data[r][c], data[c][r]);

        // cout << "Start\n";
        // for(int r = 0; r < data.size(); ++r) {
        //   // cout << data[r].size() << endl;
        //   for(int c = 0; c < data[0].size(); ++c) {
        //     cout << data[r][c] << " ";
        //   }
        //   cout << endl;
        // }
        // cout << "Done\n";

        bufferManager.writeMatrixPage(this->matrixName, pageId,
                                      this->maxBlocksPerRow, data,
                                      this->maxRowsPerBlock);
        logger.log(">>> Page: " + to_string(pageId) + " done");
      } else {
        int pageId1 = (i * this->maxBlocksPerRow) + j;
        int pageId2 = (j * this->maxBlocksPerRow) + i;
        //
        logger.log("Transposing page " + to_string(pageId1));
        //
        // MatrixPage matrixPage1 =
        // bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow,
        // pageId1); vector<vector<int>> data1 = matrixPage1.getAllRows();

        // MatrixPage matrixPage2 =
        // bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow,
        // pageId2); vector<vector<int>> data2 = matrixPage2.getAllRows();
        //
        MatrixCursor *cursor1 =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId1);
        MatrixCursor *cursor2 =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId2);

        vector<int> row;
        vector<vector<int>> data1(this->maxRowsPerBlock,
                                  vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor1->getNext();
          if (!row.empty()) {
            data1[rowCounter] = row;
          }
        }

        // cout << "Start\n";
        // for(int r = 0; r < data1.size(); ++r) {
        //   // cout << data[r].size() << endl;
        //   for(int c = 0; c < data1[0].size(); ++c) {
        //     cout << data1[r][c] << " ";
        //   }
        //   cout << endl;
        // }
        // cout << "Done\n";

        vector<vector<int>> data2(this->maxRowsPerBlock,
                                  vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor2->getNext();
          if (!row.empty()) {
            data2[rowCounter] = row;
          }
        }
        //
        // cout << "Start\n";
        // for(int r = 0; r < data2.size(); ++r) {
        //   // cout << data[r].size() << endl;
        //   for(int c = 0; c < data2[0].size(); ++c) {
        //     cout << data2[r][c] << " ";
        //   }
        //   cout << endl;
        // }
        // cout << "Done\n";

        // string destPath1 = "../data/temp/" + matrixName + "_MatrixPage" +
        // to_string(i) + "_" + to_string(j); logger.log("destPath: " +
        // destPath1);
        //
        // logger.log("Transposing page " + to_string(pageId2));
        //
        // string destPath2 = "../data/temp/" + matrixName + "_MatrixPage" +
        // to_string(j) + "_" + to_string(i); logger.log("destPath: " +
        // destPath2);

        for (int r = 0; r < data1.size(); r++)
          for (int c = r + 1; c < data1[0].size(); c++)
            swap(data1[r][c], data1[c][r]);

        // cout << "Start\n";
        // for(int r = 0; r < data1.size(); ++r) {
        //   // cout << data[r].size() << endl;
        //   for(int c = 0; c < data1[0].size(); ++c) {
        //     cout << data1[r][c] << " ";
        //   }
        //   cout << endl;
        // }
        // cout << "Done\n";

        for (int r = 0; r < data2.size(); r++)
          for (int c = r + 1; c < data2[0].size(); c++)
            swap(data2[r][c], data2[c][r]);

        // cout << "Start\n";
        // for(int r = 0; r < data2.size(); ++r) {
        //   // cout << data[r].size() << endl;
        //   for(int c = 0; c < data2[0].size(); ++c) {
        //     cout << data2[r][c] << " ";
        //   }
        //   cout << endl;
        // }
        // cout << "Done\n";

        bufferManager.writeMatrixPage(this->matrixName, pageId1,
                                      this->maxBlocksPerRow, data2,
                                      this->maxRowsPerBlock);
        logger.log(">>> Page: " + to_string(pageId1) + " done");
        bufferManager.writeMatrixPage(this->matrixName, pageId2,
                                      this->maxBlocksPerRow, data1,
                                      this->maxRowsPerBlock);
        logger.log(">>> Page: " + to_string(pageId2) + " done");
      }
    }
  }
}

/**
 * @brief Function checks if the matrix is symmetric or not. Return true or
 * false.
 *
 */
bool Matrix::checkSymmetry() {
  logger.log("Matrix::checksymmetry");
  this->maxBlocksPerRow =
      (uint)(ceil((float)(this->columnCount) / this->maxColsPerBlock));

  for (int i = 0; i < this->maxBlocksPerRow; i++) {
    for (int j = i; j < this->maxBlocksPerRow; j++) {
      if (i == j) {

        int pageId = (i * this->maxBlocksPerRow) + j;

        // MatrixPage matrixPage = bufferManager.getMatrixPage(this->matrixName,
        // this->maxBlocksPerRow, pageId); vector<vector<int>> data =
        // matrixPage.getAllRows();

        MatrixCursor *cursor =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId);

        vector<int> row;
        vector<vector<int>> data(this->maxRowsPerBlock,
                                 vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor->getNext();
          if (!row.empty()) {
            data[rowCounter] = row;
          }
        }

        for (int r = 0; r < data.size(); r++)
          for (int c = r + 1; c < data[0].size(); c++)
            if (data[r][c] != data[c][r])
              return false;
      } else {
        int pageId1 = (i * this->maxBlocksPerRow) + j;
        int pageId2 = (j * this->maxBlocksPerRow) + i;

        MatrixCursor *cursor1 =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId1);
        MatrixCursor *cursor2 =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId2);
        vector<int> row;
        vector<vector<int>> data1(this->maxRowsPerBlock,
                                  vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor1->getNext();
          if (!row.empty()) {
            data1[rowCounter] = row;
          }
        }

        vector<vector<int>> data2(this->maxRowsPerBlock,
                                  vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor2->getNext();
          if (!row.empty()) {
            data2[rowCounter] = row;
          }
        }

        // taking transpose of one matrix
        for (int r = 0; r < data1.size(); r++)
          for (int c = r; c < data1[0].size(); c++)
            swap(data1[r][c], data1[c][r]);

        // comparing each value with other original matrix
        for (int r = 0; r < data1.size(); r++)
          for (int c = 0; c < data1[0].size(); c++)
            if (data1[r][c] != data2[r][c])
              return false;
      }
    }
  }
  return true;
}

/**
 * @brief Function does the compute operation on matrix.
 *
 */
void Matrix::compute() {
  logger.log("Matrix::compute");
  this->maxBlocksPerRow =
      (uint)(ceil((float)(this->columnCount) / this->maxColsPerBlock));

  string newName = this->matrixName + "_RESULT";
  Matrix *newMatrix = new Matrix(newName);

  newMatrix->sourceFileName = "./data/" + this->matrixName + ".csv";
  newMatrix->columnCount = this->columnCount;
  newMatrix->rowCount = this->rowCount;
  newMatrix->blockCount = this->blockCount;
  newMatrix->maxRowsPerBlock = this->maxRowsPerBlock;
  newMatrix->maxColsPerBlock = this->maxColsPerBlock;
  newMatrix->maxBlocksPerRow = this->maxBlocksPerRow;

  for (int i = 0; i < this->maxBlocksPerRow; i++) {
    for (int j = i; j < this->maxBlocksPerRow; j++) {
      if (i == j) {

        int pageId = (i * this->maxBlocksPerRow) + j;
        logger.log("Transposing page " + to_string(pageId));

        // MatrixPage matrixPage = bufferManager.getMatrixPage(this->matrixName,
        // this->maxBlocksPerRow, pageId); vector<vector<int>> data =
        // matrixPage.getAllRows();

        MatrixCursor *cursor =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId);

        vector<int> row;
        vector<vector<int>> data(this->maxRowsPerBlock,
                                 vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor->getNext();
          if (!row.empty()) {
            data[rowCounter] = row;
          }
        }

        string destPath = "../data/temp/" + newName + "_MatrixPage" +
                          to_string(i) + "_" + to_string(j);
        logger.log("destPath: " + destPath);

        for (int r = 0; r < data.size(); r++) {
          for (int c = r; c < data[0].size(); c++) {
            if (r == c)
              data[r][c] = 0;
            else {
              int temp = data[r][c];
              data[r][c] -= data[c][r];
              data[c][r] -= temp;
            }
          }
        }

        bufferManager.writeMatrixPage(newName, pageId, this->maxBlocksPerRow,
                                      data, this->maxRowsPerBlock);

      } else {
        int pageId1 = (i * this->maxBlocksPerRow) + j;
        int pageId2 = (j * this->maxBlocksPerRow) + i;

        // MatrixPage matrixPage1 =
        // bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow,
        // pageId1); vector<vector<int>> data1 = matrixPage1.getAllRows();

        // MatrixPage matrixPage2 =
        // bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow,
        // pageId2); vector<vector<int>> data2 = matrixPage2.getAllRows();

        MatrixCursor *cursor1 =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId1);
        MatrixCursor *cursor2 =
            new MatrixCursor(this->matrixName, this->maxBlocksPerRow, pageId2);
        vector<int> row;
        vector<vector<int>> data1(this->maxRowsPerBlock,
                                  vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor1->getNext();
          if (!row.empty()) {
            data1[rowCounter] = row;
          }
        }

        vector<vector<int>> data2(this->maxRowsPerBlock,
                                  vector<int>(this->maxColsPerBlock, 0));
        for (int rowCounter = 0; rowCounter < this->maxRowsPerBlock;
             rowCounter++) {
          row = cursor2->getNext();
          if (!row.empty()) {
            data2[rowCounter] = row;
          }
        }

        // transposing data1 and storing to data1Transpose
        vector<vector<int>> data1Transpose = data1;
        for (int r = 0; r < data1.size(); r++)
          for (int c = r; c < data1[0].size(); c++)
            swap(data1Transpose[r][c], data1Transpose[c][r]);

        // transposing data2 and storing to data2Transpose
        vector<vector<int>> data2Transpose = data2;
        for (int r = 0; r < data2.size(); r++)
          for (int c = r; c < data2[0].size(); c++)
            swap(data2Transpose[r][c], data2Transpose[c][r]);

        // data1 = data1 - data2Transpose
        for (int r = 0; r < data1.size(); r++)
          for (int c = 0; c < data1[0].size(); c++)
            data1[r][c] -= data2Transpose[r][c];

        // data2 = data2 - data1Transpose
        for (int r = 0; r < data2.size(); r++)
          for (int c = 0; c < data2[0].size(); c++)
            data2[r][c] -= data1Transpose[r][c];

        string destPath1 = "../data/temp/" + matrixName + "_MatrixPage" +
                           to_string(i) + "_" + to_string(j);
        logger.log("destPath: " + destPath1);

        bufferManager.writeMatrixPage(newName, pageId1, this->maxBlocksPerRow,
                                      data1, this->maxRowsPerBlock);

        string destPath2 = "../data/temp/" + matrixName + "_MatrixPage" +
                           to_string(j) + "_" + to_string(i);
        logger.log("destPath: " + destPath2);

        bufferManager.writeMatrixPage(newName, pageId2, this->maxBlocksPerRow,
                                      data2, this->maxRowsPerBlock);

        // writeOutputIntoFile(destPath1, data1);
        // writeOutputIntoFile(destPath2, data2);
      }
    }
  }
  // insert the newly created matrix to matrixCatalogue
  matrixCatalogue.insertMatrix(newMatrix);
}

// void Matrix::print() {
//   logger.log("Matrix::print");
//   uint count = min((uint)PRINT_COUNT, this->columnCount);
//   logger.log(to_string(count));
//   // print headings
//   // this->writeRow(this->columns, cout);
//   logger.log("writing first row");
//
//   vector<MatrixCursor*> cursors(this->maxBlocksPerRow);
//   vector<int> pageIndices(this->maxBlocksPerRow);
//   vector<int> row;
//   logger.log("Total Indices: " + to_string(this->maxBlocksPerRow));
//
//   // Initialization
//   for(int idx = 0; idx < this->maxBlocksPerRow; ++idx)
//   {
//     pageIndices[idx] = idx;
//   }
//
//   // do until loop contents
//   int rowCounter = 0;
//   while(rowCounter < this -> maxBlocksPerRow) // number of blocks per row is
//   same as number of blocks per col
//   {
//     for(int idx = 0; idx < this->maxBlocksPerRow; ++idx)
//     {
//       logger.log("Idx: " + to_string(idx));
//       cursors[idx] = new MatrixCursor(this->matrixName,
//       this->maxBlocksPerRow, pageIndices[idx]);
//       // row = cursors[idx] -> getNext();
//       // logger.log("Got next");
//       // this->writeRow(row, cout);
//       // logger.log("row written");
//     }
//
//     logger.log("Updated cursors");
//     for(int r = 0; r < this->maxRowsPerBlock; ++r)
//     {
//       for (int idx = 0; idx < this->maxBlocksPerRow; ++idx)
//       {
//         row = cursors[idx] -> getNext();
//         this->writeRow(row, cout);
//         logger.log("row written for cursor " + to_string(idx));
//       }
//       cout << endl;
//     }
//
//     // for updating the pageIndices
//     for(int idx = 0; idx < this->maxBlocksPerRow; ++idx)
//     {
//       pageIndices[idx] += maxBlocksPerRow;
//     }
//     ++rowCounter;
//   }
//
//
//   // printRowCount(this->rowCount);
//   printRowCount(count); // printing no of rows visible and not actual no of
//   rows
// }

/**
 * @brief This function returns one row of the table using the cursor object. It
 * returns an empty row is all rows have been read.
 *
 * @param cursor
 * @return vector<int>
 */
void Matrix::getNextMatrixPage(MatrixCursor *cursor) {
  logger.log("Matrix::getNextMatrixPage");

  if (cursor->matrixPageIndex < this->blockCount - 1) {
    cursor->nextMatrixPage(cursor->matrixPageIndex + 1);
  }
}

/**
 * @brief called when EXPORT command is invoked to move source file to "data"
 * folder.
 *
 */
void Matrix::makePermanent() {
  logger.log("Matrix::makePermanent");
  // if (!this->isPermanent())
  //   bufferManager.deleteFile(this->sourceFileName);
  string newSourceFile = "../data/" + this->matrixName + ".csv";
  ofstream fout(newSourceFile, ios::out);

  // print headings
  // this->writeRow(this->columns, fout);

  MatrixCursor matrixCursor(this->matrixName, this->maxBlocksPerRow, 0);
  vector<int> row;

  for (int rowCounter = 0; rowCounter < this->columnCount; rowCounter++) {
    row = matrixCursor.getNext();
    this->writeRow(row, fout);
  }

  fout.close();
}

/**
 * @brief Function to check if table is already exported
 *
 * @return true if exported
 * @return false otherwise
 */
bool Matrix::isPermanent() {
  logger.log("Matrix::isPermanent");
  if (this->sourceFileName == "../data/" + this->matrixName + ".csv")
    return true;
  return false;
}

/**
 * @brief The unload function removes the table from the database by deleting
 * all temporary files created as part of this table
 *
 */
void Matrix::unload() {
  logger.log("Matrix::~unload");
  for (int pageCounter = 0; pageCounter < this->blockCount; pageCounter++)
    bufferManager.deleteFile(this->matrixName, pageCounter);
  // if (!isPermanent())
  //   bufferManager.deleteFile(this->sourceFileName);
}

/**
 * @brief Function that returns a cursor that reads rows from this table
 *
 * @return Cursor
 */
MatrixCursor Matrix::getMatrixCursor() {
  logger.log("Matrix::getCursor");
  MatrixCursor cursor(this->matrixName, this->maxBlocksPerRow, 0);
  return cursor;
}
/**
 * @brief Function that returns the index of column indicated by columnName
 *
 * @param columnName
 * @return int
 */
int Matrix::getColumnIndex(string columnName) {
  logger.log("Table::getColumnIndex");
  for (int columnCounter = 0; columnCounter < this->columnCount;
       columnCounter++) {
    if (this->columns[columnCounter] == columnName)
      return columnCounter;
  }
}
