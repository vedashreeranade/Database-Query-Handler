#include "global.h"
#include <sstream>
#include <string>
/**
 * @brief Construct a new Page object. Never used as part of the code
 *
 */
Page::Page()
{
    this->pageName = "";
    this->tableName = "";
    this->pageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->rows.clear();
}

/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param tableName 
 * @param pageIndex 
 */
Page::Page(string tableName, int pageIndex)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->pageName = "../data/temp/" + this->tableName + "_Page" + to_string(pageIndex);
    Table table = *tableCatalogue.getTable(tableName);
    this->columnCount = table.columnCount;
    uint maxRowCount = table.maxRowsPerBlock;
    vector<int> row(columnCount, 0);
    this->rows.assign(maxRowCount, row);

    ifstream fin(pageName, ios::in);
    this->rowCount = table.rowsPerBlockCount[pageIndex];
    int number;
    for (uint rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
        {
            fin >> number;
            this->rows[rowCounter][columnCounter] = number;
        }
    }
    fin.close();
}

/**
 * @brief Get row from page indexed by rowIndex
 * 
 * @param rowIndex 
 * @return vector<int> 
 */
vector<int> Page::getRow(int rowIndex)
{
    logger.log("Page::getRow");
    vector<int> result;
    result.clear();
    if (rowIndex >= this->rowCount)
        return result;
    return this->rows[rowIndex];
}

Page::Page(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("Page::Page");
    this->tableName = tableName;
    this->pageIndex = pageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    this->columnCount = rows[0].size();
    this->pageName = "../data/temp/"+this->tableName + "_Page" + to_string(pageIndex);
}

/**
 * @brief writes current page contents to file.
 * 
 */
void Page::writePage()
{
    logger.log("Page::writePage");
    ofstream fout(this->pageName, ios::trunc);
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << this->rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    fout.close();
}



/**
 * @brief Construct a new MatrixPage object. Never used as part of the code
 *
 */
MatrixPage::MatrixPage()
{
    this->matrixPageName = "";
    this->matrixName = "";
    this->matrixRowIndex = -1;
    this->matrixColIndex = -1;
    // this->matrixPageIndex = -1;
    this->rowCount = 0;
    this->columnCount = 0;
    this->rows.clear();
}

/**
 * @brief Construct a new Page:: Page object given the table name and page
 * index. When tables are loaded they are broken up into blocks of BLOCK_SIZE
 * and each block is stored in a different file named
 * "<tablename>_Page<pageindex>". For example, If the Page being loaded is of
 * table "R" and the pageIndex is 2 then the file name is "R_Page2". The page
 * loads the rows (or tuples) into a vector of rows (where each row is a vector
 * of integers).
 *
 * @param tableName 
 * @param pageIndex 
 */


MatrixPage::MatrixPage(string matrixName, int maxBlocksPerRow, int matrixPageIndex)
{
  logger.log("MatrixPage::MatrixPage");
  this->matrixName = matrixName;
  // this->PageIndex = matrixPageIndex;
  this->matrixRowIndex = to_string(matrixPageIndex / maxBlocksPerRow);
  this->matrixColIndex = to_string(matrixPageIndex % maxBlocksPerRow);
  this->matrixPageName = "../data/temp/"+this->matrixName + "_MatrixPage" + this->matrixRowIndex + "_" + this->matrixColIndex;
  
  logger.log("Getting page:" + this->matrixPageName);
  
  Matrix matrix = *matrixCatalogue.getMatrix(matrixName);
  this->columnCount = matrix.columnCount;
  this->rowCount = this->columnCount;
  uint maxRowCount = matrix.maxRowsPerBlock;

  vector<int> row(maxRowCount, 0);
  this->rows.assign(maxRowCount, row);

  logger.log(matrixPageName);
  ifstream fin(matrixPageName, ios::in);

  string line;

  string word;
  int rowCounter = 0;
  int columnCounter = 0;
  while(getline(fin, line))
  {
    stringstream s(line);
    // cout << "Line: " << line << endl;
    while(getline(s, word, ' ')) 
    {
       this->rows[rowCounter][columnCounter] = stoi(word);
       // cout << "Int: " << stoi(word) << endl;
       ++columnCounter;
    }
    columnCounter = 0;
    ++rowCounter;
  }

  logger.log("Read Matrix");
  fin.close();
  logger.log("Done");
}
// MatrixPage::MatrixPage(string matrixName, int maxBlocksPerRow, int matrixPageIndex)
// {
//     logger.log("MatrixPage::MatrixPage");
//     this->matrixName = matrixName;
//     this->matrixRowIndex = to_string(matrixPageIndex / maxBlocksPerRow);
//     this->matrixColIndex = to_string(matrixPageIndex % maxBlocksPerRow);
//     // this->matrixPageIndex = matrixPageIndex;
//     // logger.log("Here");
//     this->matrixPageName = "../data/temp/"+this->matrixName + "_MatrixPage" + this->matrixRowIndex + "_" + this->matrixColIndex;
//     // this->matrixPageName = "../data/temp/" + this->matrixName + "_MatrixPage" + to_string(matrixPageIndex);
//     logger.log("Getting page:" + this->matrixPageName);
//     Matrix matrix = *matrixCatalogue.getMatrix(matrixName);
//     this->columnCount = matrix.columnCount;
//     uint maxRowCount = matrix.maxRowsPerBlock;
//     vector<int> row(columnCount, 0);
//     this->rows.assign(maxRowCount, row);
//     
//     cout << matrixPageName << " : \n";
//     ifstream fin(matrixPageName, ios::in);
//     string line;
//         // getline(fin, line);
//         // cout << line << endl;
//  
//     this->rowCount = matrix.rowsPerBlockCount[matrixPageIndex];
//     int number;
//     cout << this-> columnCount << " " << columnCount << endl;
//
//     // string line;
//     string word;
//     int rowCounter = 0;
//     int columnCounter = 0;
//     while(getline(fin, line))
//     {
//         while(getline(fin, word, ' ')) 
//         {
//            this->rows[rowCounter][columnCounter] = stoi(word);
//             ++columnCounter;
//         }
//         ++rowCounter;
//     }
//
//     // for (uint rowCounter = 0; rowCounter < this->columnCount; rowCounter++)
//     // {
//     //
//     //     string line;
//     //     // getline(fin, line);
//     //     // cout << line;
//     //     for (int columnCounter = 0; columnCounter < columnCount; columnCounter++)
//     //     {
//     //        fin >> number;
//     //        cout << number << " ";
//     //        this->rows[rowCounter][columnCounter] = number;
//     //     }
//     //     cout << endl;
//     // }
//     fin.close();
// }

/**
 * @brief Get row from page indexed by rowIndex
 * 
 * @param rowIndex 
 * @return vector<int> 
 */
vector<int> MatrixPage::getRow(int rowIndex)
{
    logger.log("MatrixPage::getRow");
    vector<int> result;
    result.clear();
    logger.log("columnCount:" + to_string(this->columnCount));
    if (rowIndex >= this->columnCount)
        return result;
    return this->rows[rowIndex];
}

vector<vector<int>> MatrixPage::getAllRows(){
    return this->rows;
}

MatrixPage::MatrixPage(string matrixName, int matrixPageIndex, int maxBlocksPerRow, vector<vector<int>> rows, int rowCount)
{
    logger.log("MatrixPage::MatrixPage");
    // logger.log("Get stats");
    this->matrixName = matrixName;
    this->matrixRowIndex = to_string(matrixPageIndex / maxBlocksPerRow);
    this->matrixColIndex = to_string(matrixPageIndex % maxBlocksPerRow);
    // this->matrixPageIndex = matrixPageIndex;
    this->rows = rows;
    this->rowCount = rowCount;
    logger.log("Row Count: " + to_string(this -> rows.size()));
    logger.log("Getting columnCount");
    this->columnCount = rows[0].size();
    // logger.log("Got stats");
    this->matrixPageName = "../data/temp/"+this->matrixName + "_MatrixPage" + this->matrixRowIndex + "_" + this->matrixColIndex;
    logger.log("Creating Page: " + this->matrixPageName);
}

/**
 * @brief writes current page contents to file.
 * 
 */
void MatrixPage::writeMatrixPage()
{
    logger.log("MatrixPage::writePage");
    ofstream fout(this->matrixPageName, ios::trunc);
    logger.log("Writing on: " + this->matrixPageName);
    for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    {
        for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
        {
            if (columnCounter != 0)
                fout << " ";
            fout << this->rows[rowCounter][columnCounter];
        }
        fout << endl;
    }
    // for (int rowCounter = 0; rowCounter < this->rowCount; rowCounter++)
    // {
    //     for (int columnCounter = 0; columnCounter < this->columnCount; columnCounter++)
    //     {
    //         if (columnCounter != 0)
    //             fout << " ";
    //         fout << this->rows[rowCounter][columnCounter];
    //     }
    //     fout << endl;
    // }
    fout.close();
}

void MatrixPage::update_rows(vector<vector<int>> rows)
{
    this->rows = rows;
} 
