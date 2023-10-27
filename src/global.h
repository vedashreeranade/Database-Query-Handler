#include "executor.h"

extern float BLOCK_SIZE;
extern uint BLOCK_COUNT;
extern uint PRINT_COUNT;
extern vector<string> tokenizedQuery;
extern ParsedQuery parsedQuery;
extern TableCatalogue tableCatalogue;
extern BufferManager bufferManager;

extern MatrixCatalogue matrixCatalogue;
extern uint blockReadCounter;
extern uint blockWriteCounter;
