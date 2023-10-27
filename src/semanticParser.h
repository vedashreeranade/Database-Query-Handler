#include "syntacticParser.h"

bool semanticParse();

bool semanticParseCLEAR();
bool semanticParseCROSS();
bool semanticParseDISTINCT();
bool semanticParseEXPORT();
bool semanticParseINDEX();
bool semanticParseJOIN();
bool semanticParseLIST();
bool semanticParseLOAD();
bool semanticParsePRINT();
bool semanticParsePROJECTION();
bool semanticParseRENAME();
bool semanticParseSELECTION();
bool semanticParseSORT();
bool semanticParseSOURCE();
bool semanticParseORDERBY();
bool semanticParseGROUPBY();

/********************* Matrix functions ************************/
bool semanticParseLOAD_MATRIX();
bool semanticParsePRINT_MATRIX();
bool semanticParseTRANSPOSE_MATRIX();
bool semanticParseEXPORT_MATRIX();
bool semanticParseRENAME_MATRIX();
bool semanticParseCHECKSYMMETRY();
bool semanticParseCOMPUTE();

// bool semanticParseINPLACESORT(); 

