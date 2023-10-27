#include "global.h"
#include <string>

BufferManager::BufferManager()
{
    logger.log("BufferManager::BufferManager");
}

/**
 * @brief Function called to read a page from the buffer manager. If the page is
 * not present in the pool, the page is read and then inserted into the pool.
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::getPage(string tableName, int pageIndex)
{
    logger.log("BufferManager::getPage");
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    logger.log("BufferManager::getPage: " + pageName);
    if (this->inPool(pageName))
        return this->getFromPool(pageName);
    else
        return this->insertIntoPool(tableName, pageIndex);
}

/**
 * @brief Function called to read a matrixPage from the buffer manager. If the matrixPage is
 * not present in the pool, the matrixPage is read and then inserted into the pool.
 *
 * @param matrixName 
 * @param matrixPageIndex 
 * @return Page 
 */
MatrixPage BufferManager::getMatrixPage(string matrixName, int maxBlocksPerRow, int matrixPageIndex)
{
    blockReadCounter++;
    logger.log("BufferManager::getMatrixPage");
    string matrixRowIndex = to_string(matrixPageIndex / maxBlocksPerRow); 
    string matrixColIndex = to_string(matrixPageIndex % maxBlocksPerRow); 
    string matrixPageName = "../data/temp/"+ matrixName + "_MatrixPage" + matrixRowIndex + "_" + matrixColIndex;
    if (this->inMatrixPool(matrixPageName))
        return this->getFromMatrixPool(matrixPageName);
    else
        return this->insertIntoMatrixPool(matrixName, maxBlocksPerRow, matrixPageIndex);
}

/**
 * @brief Checks to see if a page exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::inPool(string pageName)
{
    logger.log("BufferManager::inPool");
    for (auto page : this->pages)
    {
        if (pageName == page.pageName)
            return true;
    }
    return false;
}

/**
 * @brief Checks to see if a matrix exists in the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::inMatrixPool(string matrixPageName)
{
    logger.log("BufferManager::inMatrixPool");
    for (auto matrixPage : this->matrixPages)
    {
        if (matrixPageName == matrixPage.matrixPageName)
            return true;
    }
    return false;
}

/**
 * @brief Removes the page if it exists from the pool
 *
 * @param pageName 
 * @return true 
 * @return false 
 */
bool BufferManager::removeFromPool(string tableName, int pageIndex)
{
    string pageName = "../data/temp/"+tableName + "_Page" + to_string(pageIndex);
    logger.log("BufferManager::removeFromPool" + pageName);
    int N = this->pages.size();
    for (int idx = 0; idx < N; ++idx)
    {
        logger.log(this->pages[idx].pageName);
    }
    for (int idx = 0; idx < N; ++idx)
    {
        if (pageName == this->pages[idx].pageName) {
            // remove this page by shifting all the values 
            // towards front and remove the last entry
            for(int i = idx + 1; i < N; ++i) {
                this->pages[i-1] = this->pages[i];
            }
            this->pages.pop_back();
            for (int idx = 0; idx < N-1; ++idx)
            {
                logger.log(this->pages[idx].pageName);
            }
            return true;
        }
    }
    // page not in pool
    return false;
}

/**
 * @brief If the page is present in the pool, then this function returns the
 * page. Note that this function will fail if the page is not present in the
 * pool.
 *
 * @param pageName 
 * @return Page 
 */
Page BufferManager::getFromPool(string pageName)
{
    logger.log("BufferManager::getFromPool");
    for (auto page : this->pages)
        if (pageName == page.pageName)
            return page;
}

/**
 * @brief If the matrix is present in the pool, then this function returns the
 * matrixPage. Note that this function will fail if the page is not present in the
 * pool.
 *
 * @param pageName 
 * @return Page 
 */
MatrixPage BufferManager::getFromMatrixPool(string matrixPageName)
{
    logger.log("BufferManager::getFromMatrixPool");
    for (auto matrixPage : this->matrixPages)
        if (matrixPageName == matrixPage.matrixPageName)
            return matrixPage;
}

/**
 * @brief Inserts page indicated by tableName and pageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted page from the pool and adds
 * the current page at the end. It naturally follows a queue data structure. 
 *
 * @param tableName 
 * @param pageIndex 
 * @return Page 
 */
Page BufferManager::insertIntoPool(string tableName, int pageIndex)
{
    logger.log("BufferManager::insertIntoPool");
    Page page(tableName, pageIndex);
    if (this->pages.size() >= BLOCK_COUNT)    
        pages.pop_front();
    pages.push_back(page);
    logger.log("BufferManager::insertIntoPool: " + page.pageName);
    return page;
}

/**
 * @brief Inserts matrixPage indicated by matrixName and matrixPageIndex into pool. If the
 * pool is full, the pool ejects the oldest inserted matrixPage from the pool and adds
 * the current matrixPage at the end. It naturally follows a queue data structure. 
 *
 * @param matrixName 
 * @param matrixPageIndex 
 * @return MatrixPage 
 */
MatrixPage BufferManager::insertIntoMatrixPool(string matrixName, int maxBlocksPerRow, int matrixPageIndex)
{
    logger.log("BufferManager::insertIntoMatrixPool");
    MatrixPage matrixPage(matrixName, maxBlocksPerRow, matrixPageIndex);
    logger.log("Start");
    logger.log("MatrixPage done | " + matrixName + " | " + matrixPage.matrixPageName);
    if (this->matrixPages.size() >= BLOCK_COUNT)
        matrixPages.pop_front();
    matrixPages.push_back(matrixPage);
    return matrixPage;
}


/**
 * @brief The buffer manager is also responsible for writing pages. This is
 * called when new tables are created using assignment statements.
 *
 * @param tableName 
 * @param pageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writePage(string tableName, int pageIndex, vector<vector<int>> rows, int rowCount)
{
    logger.log("BufferManager::writePage");
    Page page(tableName, pageIndex, rows, rowCount);
    page.writePage();
}

/**
 * @brief The buffer manager is also responsible for writing matrixPages. This is
 * called when new matrices are created using assignment statements.
 *
 * @param matrixName 
 * @param matrixPageIndex 
 * @param rows 
 * @param rowCount 
 */
void BufferManager::writeMatrixPage(string matrixName, int matrixPageIndex, int maxBlocksPerRow, vector<vector<int>> rows, int rowCount)
{
    blockWriteCounter++;
    logger.log("BufferManager::writeMatrixPage");
    MatrixPage matrixPage(matrixName, matrixPageIndex, maxBlocksPerRow, rows, rowCount);
    matrixPage.writeMatrixPage();
}


/**
 * @brief Deletes file names fileName
 *
 * @param fileName 
 */
void BufferManager::deleteFile(string fileName)
{
    
    if (remove(fileName.c_str()))
        logger.log("BufferManager::deleteFile: Err");
    else logger.log("BufferManager::deleteFile: Success");
}

/**
 * @brief Overloaded function that calls deleteFile(fileName) by constructing
 * the fileName from the tableName and pageIndex.
 *
 * @param tableName 
 * @param pageIndex 
 */
void BufferManager::deleteFile(string tableName, int pageIndex)
{
    string fileName = "../data/temp/"+ tableName + "_Page" + to_string(pageIndex);
    logger.log("BufferManager::deleteFile " + fileName);
    this->deleteFile(fileName);
}

/**
 * @brief Overloaded function that calls deleteMatrixFile(fileName) by constructing
 * the fileName from the matrixName and matrixPageIndex.
 *
 * @param matrixName 
 * @param matrixPageIndex 
 */
void BufferManager::deleteMatrixFile(string matrixName, int matrixPageIndex)
{
    logger.log("BufferManager::deleteMatrixFile");
    string fileName = "../data/temp/"+matrixName + "_MatrixPage" + to_string(matrixPageIndex);
    this->deleteFile(fileName);
}
