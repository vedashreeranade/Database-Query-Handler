#include "tableCatalogue.h"

using namespace std;

enum QueryType
{
    CLEAR,
    CROSS,
    DISTINCT,
    EXPORT,
    INDEX,
    JOIN,
    LIST,
    LOAD,
    PRINT,
    PROJECTION,
    RENAME,
    SELECTION,
    SORT,
    SOURCE,

    LOAD_MATRIX,
    PRINT_MATRIX,
    TRANSPOSE_MATRIX,  
    EXPORT_MATRIX, 
    RENAME_MATRIX,
    CHECKSYMMETRY,
    COMPUTE,
    
    // INPLACESORT,
    ORDERBY,
    GROUPBY,

    UNDETERMINED
};

enum BinaryOperator
{
    LESS_THAN,
    GREATER_THAN,
    LEQ,
    GEQ,
    EQUAL,
    NOT_EQUAL,
    NO_BINOP_CLAUSE
};

enum SortingStrategy
{
    ASC,
    DESC,
    NO_SORT_CLAUSE
};

enum SelectType
{
    COLUMN,
    INT_LITERAL,
    NO_SELECT_CLAUSE
};

class ParsedQuery
{

public:
    QueryType queryType = UNDETERMINED;

    string clearRelationName = "";

    string crossResultRelationName = "";
    string crossFirstRelationName = "";
    string crossSecondRelationName = "";

    string distinctResultRelationName = "";
    string distinctRelationName = "";

    string exportRelationName = "";

    IndexingStrategy indexingStrategy = NOTHING;
    string indexColumnName = "";
    string indexRelationName = "";

    BinaryOperator joinBinaryOperator = NO_BINOP_CLAUSE;
    string joinResultRelationName = "";
    string joinFirstRelationName = "";
    string joinSecondRelationName = "";
    string joinFirstColumnName = "";
    string joinSecondColumnName = "";

    string loadRelationName = "";
    string printRelationName = "";

    string projectionResultRelationName = "";
    vector<string> projectionColumnList;
    string projectionRelationName = "";

    string renameFromColumnName = "";
    string renameToColumnName = "";
    string renameRelationName = "";

    /************** variable for matrix rename *********/
    string renameCurrentMatrix = "";
    string renameNewMatrix = "";
    /************** ************************** *********/

    SelectType selectType = NO_SELECT_CLAUSE;
    BinaryOperator selectionBinaryOperator = NO_BINOP_CLAUSE;
    string selectionResultRelationName = "";
    string selectionRelationName = "";
    string selectionFirstColumnName = "";
    string selectionSecondColumnName = "";
    int selectionIntLiteral = 0;

    // SortingStrategy sortingStrategy = NO_SORT_CLAUSE;
    vector<SortingStrategy> sortingStrategy;
    // string sortResultRelationName = "";
    // string sortColumnName = "";
    vector<string> sortColumnNames;
    string sortRelationName = "";

    string groupbyResultRelationName = "";
    string groupbyColumnName = "";
    string groupbyRelationName = "";
    string groupbyGroupingColumnName = "";
    int groupbyConditionValue = 0;
    string groupbyAggregateFunc1 = "";
    string groupbyAggregateFunc2 = "";
    BinaryOperator groupbyBinaryOperator = NO_BINOP_CLAUSE;

    string orderbyColumnName = "";
    string orderbyResultRelationName = "";
    string orderbyRelationName = "";

    string sourceFileName = "";

    string transposeRelationName = "";
    string checkSymmetryRelationName = "";
    string computeRelationName = "";

    ParsedQuery();
    void clear();
};

bool syntacticParse();
bool syntacticParseCLEAR();
bool syntacticParseCROSS();
bool syntacticParseDISTINCT();
bool syntacticParseEXPORT();
bool syntacticParseINDEX();
bool syntacticParseJOIN();
bool syntacticParseLIST();
bool syntacticParseLOAD();
bool syntacticParsePRINT();
bool syntacticParsePROJECTION();
bool syntacticParseRENAME();
bool syntacticParseSELECTION();
bool syntacticParseSORT();
bool syntacticParseSOURCE();

bool syntacticParseLOAD_MATRIX();
bool syntacticParsePRINT_MATRIX();
bool syntacticParseRENAME_MATRIX();
bool syntacticParseEXPORT_MATRIX();
bool syntacticParseTRANSPOSE_MATRIX();
bool syntacticParseCOMPUTE();
bool syntacticParseCHECKSYMMETRY();

// bool syntacticParseINPLACESORT();
bool syntacticParseORDERBY();
bool syntacticParseGROUPBY();

bool isFileExists(string tableName);
bool isQueryFile(string fileName);
