#include "global.h"
/**
 * @brief
 * SYNTAX: SOURCE filename
 */
bool syntacticParseSOURCE() {
  logger.log("syntacticParseSOURCE");
  if (tokenizedQuery.size() != 2) {
    cout << "SYNTAX ERROR" << endl;
    return false;
  }
  parsedQuery.queryType = SOURCE;
  parsedQuery.sourceFileName = tokenizedQuery[1];
  return true;
}

bool semanticParseSOURCE() {
  logger.log("semanticParseSOURCE");
  if (!isQueryFile(parsedQuery.sourceFileName)) {
    cout << "SEMANTIC ERROR: File doesn't exist" << endl;
    return false;
  }
  return true;
}

// void doCommand()
// {
//     logger.log("doCommand");
//     if (syntacticParse() && semanticParse())
//         executeCommand();
//     return;
// }

void executeSOURCE() {

  regex delim("[^\\s,]+");
  logger.log("executeSOURCE");
  logger.log("Opening file: ");
  string filename = "../data/" + parsedQuery.sourceFileName + ".ra";
  logger.log(filename); 
  ifstream inputfile(filename);
  for (string inputline; getline(inputfile, inputline);) {
    tokenizedQuery.clear();
    // logger.log(inputline);
    auto words_begin =
        std::sregex_iterator(inputline.begin(), inputline.end(), delim);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i)
      tokenizedQuery.emplace_back((*i).str());

    if (tokenizedQuery.size() == 1 && tokenizedQuery.front() == "QUIT") {
      break;
    }

    if (tokenizedQuery.empty()) {
      continue;
    }

    // cout << "Query: \n";
    // for(string s: tokenizedQuery) {
    //   cout << s << "\n";
    // }
    if (tokenizedQuery.size() == 1) {
      cout << "SYNTAX ERROR" << endl;
      continue;
    }

    logger.log("doCommand");
    if (syntacticParse() && semanticParse())
      executeCommand();
  }
  inputfile.close();
  return;
}


