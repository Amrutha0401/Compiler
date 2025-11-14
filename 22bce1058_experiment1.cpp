#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <set>

using namespace std;

vector<string> keywords = {
    "int", "float", "char", "if", "else", "while", "for", "return", "void", "double", "string"
};

set<string> operators = { "+", "-", "*", "/", "=", "==", "!=", "<", ">", "<=", ">=" };
set<char> specialSymbols = { '(', ')', '{', '}', ';', ',' };

bool isKeyword(const string& token) {
    return find(keywords.begin(), keywords.end(), token) != keywords.end();
}

bool isValidIdentifier(const string& token) {
    if (token.empty()) return false;
    if (!isalpha(token[0]) && token[0] != '_') return false;
    for (size_t i = 1; i < token.length(); ++i)
        if (!isalnum(token[i]) && token[i] != '_') return false;
    return true;
}

bool isInteger(const string& token) {
    return regex_match(token, regex("^[0-9]+$"));
}

bool isFloat(const string& token) {
    return regex_match(token, regex("^[0-9]*\\.[0-9]+$"));
}

bool isOperator(const string& token) {
    return operators.find(token) != operators.end();
}

bool isSpecialSymbol(char c) {
    return specialSymbols.find(c) != specialSymbols.end();
}

int main() {
    ifstream file("test_input.cpp");
    if (!file.is_open()) {
        cerr << "Error opening input file.\n";
        return 1;
    }

    string line;
    bool inMultilineComment = false;
    int keywordCount = 0, identifierCount = 0, intCount = 0, floatCount = 0;
    int operatorCount = 0, symbolCount = 0, errorCount = 0;

    while (getline(file, line)) {
        size_t i = 0;

        // Handle multiline comments
        if (inMultilineComment) {
            if (line.find("*/") != string::npos)
                inMultilineComment = false;
            continue;
        }
        if (line.find("/*") != string::npos) {
            inMultilineComment = true;
            continue;
        }

        // Remove single-line comments
        size_t commentPos = line.find("//");
        if (commentPos != string::npos)
            line = line.substr(0, commentPos);

        string token = "";
        while (i < line.length()) {
            char c = line[i];

            if (isspace(c)) {
                ++i;
                continue;
            }

            if (isSpecialSymbol(c)) {
                ++symbolCount;
                ++i;
                continue;
            }

            if (ispunct(c) && !isSpecialSymbol(c)) {
                // Handle multi-character operators
                if (i + 1 < line.length()) {
                    string op = string(1, c) + line[i + 1];
                    if (isOperator(op)) {
                        ++operatorCount;
                        i += 2;
                        continue;
                    }
                }

                string op(1, c);
                if (isOperator(op)) {
                    ++operatorCount;
                    ++i;
                    continue;
                }
            }

            // Build a token
            token = "";
            while (i < line.length() && (isalnum(line[i]) || line[i] == '_' || line[i] == '.')) {
                token += line[i++];
            }

            if (token.empty()) {
                ++i;
                continue;
            }

            if (isKeyword(token))
                ++keywordCount;
            else if (isInteger(token))
                ++intCount;
            else if (isFloat(token))
                ++floatCount;
            else if (isValidIdentifier(token))
                ++identifierCount;
            else {
                cerr << "Lexical Error: Unrecognized token '" << token << "'\n";
                ++errorCount;
            }
        }
    }

    int totalTokens = keywordCount + identifierCount + intCount + floatCount + operatorCount + symbolCount;

    cout << "Token Counts:\n";
    cout << "Keywords       : " << keywordCount << "\n";
    cout << "Identifiers    : " << identifierCount << "\n";
    cout << "Integers       : " << intCount << "\n";
    cout << "Floats         : " << floatCount << "\n";
    cout << "Operators      : " << operatorCount << "\n";
    cout << "Special Symbols: " << symbolCount << "\n";
    cout << "Errors         : " << errorCount << "\n";
    cout << "-----------------------------\n";
    cout << "Total Tokens   : " << totalTokens << "\n";

    return 0;
}
