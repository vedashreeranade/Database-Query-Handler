#include "global.h"

Cursor::Cursor(string tableName, int pageIndex)
{
    logger.log("Cursor::Cursor");
    this->page = bufferManager.getPage(tableName, pageIndex);
    this->pagePointer = 0;
    this->tableName = tableName;
    this->pageIndex = pageIndex;
}

/**
 * @brief This function reads the next row from the page. The index of the
 * current row read from the page is indicated by the pagePointer(points to row
 * in page the cursor is pointing to).
 *
 * @return vector<int> 
 */
vector<int> Cursor::getNext()
{
    logger.log("Cursor::getNext");
    vector<int> result = this->page.getRow(this->pagePointer);
    this->pagePointer++;
    if(result.empty()){
        logger.log("Empty result, fetching from next page");
        tableCatalogue.getTable(this->tableName)->getNextPage(this);
        if(!this->pagePointer){
            result = this->page.getRow(this->pagePointer);
            this->pagePointer++;
        }
        else {
            logger.log("No more pages left");
        }
    }
    return result;
}
/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex 
 */
void Cursor::nextPage(int pageIndex)
{
    logger.log("Cursor::nextPage");
    this->page = bufferManager.getPage(this->tableName, pageIndex);
    this->pageIndex = pageIndex;
    this->pagePointer = 0;
}

// MATRIX IMPLEMENTATION
MatrixCursor::MatrixCursor(string matrixName, int maxBlocksPerRow, int matrixPageIndex)
{
    logger.log("MatrixCursor::MatrixCursor");
    this->matrixPage = bufferManager.getMatrixPage(matrixName, maxBlocksPerRow, matrixPageIndex);
    logger.log("matrix page name: " + this->matrixPage.matrixPageName);
    this->matrixPagePointer = 0;
    this->matrixName = matrixName;
    this->matrixRowIndex = matrixPageIndex / maxBlocksPerRow;
    this->matrixColIndex = matrixPageIndex % maxBlocksPerRow;
    this->matrixPageIndex = matrixPageIndex;
    this->maxBlocksPerRow = maxBlocksPerRow;
}

/**
 * @brief This function reads the next row from the matrixPage. The index of the
 * current row read from the matrixPage is indicated by the matrixPagePointer(points to row
 * in matrixPage the cursor is pointing to).
 *
 * @return vector<int> 
 */
vector<int> MatrixCursor::getNext()
{
    logger.log("MatrixCursor::getNext");
    vector<int> result = this->matrixPage.getRow(this->matrixPagePointer);
    this->matrixPagePointer++;
    if(result.empty()){
        matrixCatalogue.getMatrix(this->matrixName)->getNextMatrixPage(this);
        if(!this->matrixPagePointer){
            result = this->matrixPage.getRow(this->matrixPagePointer);
            this->matrixPagePointer++;
        }
    }
    return result;
}

void MatrixCursor::updating(vector<vector<int>> row, int matrixPageIndex)
{
    logger.log("updating..." + to_string(matrixPageIndex));

    this->matrixPage = bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow, matrixPageIndex);

    // this->matrixPage.updateRows(row);
    this->matrixPage.update_rows(row);
}

/**
 * @brief Function that loads Page indicated by pageIndex. Now the cursor starts
 * reading from the new page.
 *
 * @param pageIndex 
 */
void MatrixCursor::nextMatrixPage(int matrixPageIndex)
{
    logger.log("MatrixCursor::nextMatrixPage");
    this->matrixPage = bufferManager.getMatrixPage(this->matrixName, this->maxBlocksPerRow, matrixPageIndex);
    this->matrixPageIndex = matrixPageIndex;
    this->matrixPagePointer = 0;
}
