#include <iostream>
#include <fstream>
#include <sstream>
#include <regex>
#include <vector>
#include <set>
#include <iomanip>

using namespace std;

vector<string> keywords = {
    "int", "float", "char", "double", "void", "if", "else", "while", "for", "return", "string", "bool"
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

void printToken(const string& type, const string& value) {
    cout << left << setw(18) << type << ": " << value << "\n";
}

int main() {
    ifstream file("test_input.cpp");
    if (!file.is_open()) {
        cerr << "Error: Could not open input file.\n";
        return 1;
    }

    string line;
    bool inMultilineComment = false;
    int lineNo = 0;

    cout << "Detected Tokens:\n----------------\n";

    while (getline(file, line)) {
        ++lineNo;
        size_t i = 0;

        // Handle multi-line comment
        if (inMultilineComment) {
            if (line.find("*/") != string::npos)
                inMultilineComment = false;
            continue;
        }

        if (line.find("/*") != string::npos) {
            inMultilineComment = true;
            continue;
        }

        // Remove single-line comment
        size_t commentPos = line.find("//");
        if (commentPos != string::npos)
            line = line.substr(0, commentPos);

        while (i < line.length()) {
            if (isspace(line[i])) {
                ++i;
                continue;
            }

            if (isSpecialSymbol(line[i])) {
                printToken("Special Symbol", string(1, line[i]));
                ++i;
                continue;
            }

            // Operator handling (1 or 2 characters)
            string op = "";
            op += line[i];
            if (i + 1 < line.length())
                op += line[i + 1];

            if (isOperator(op)) {
                printToken("Operator", op);
                i += 2;
                continue;
            } else if (isOperator(string(1, line[i]))) {
                printToken("Operator", string(1, line[i]));
                ++i;
                continue;
            }

            // Tokenization (identifiers, numbers, etc.)
            string token = "";
            if (isalpha(line[i]) || line[i] == '_' || isdigit(line[i])) {
                while (i < line.length() && (isalnum(line[i]) || line[i] == '_' || line[i] == '.')) {
                    token += line[i++];
                }

                if (isKeyword(token))
                    printToken("Keyword", token);
                else if (isInteger(token))
                    printToken("Integer", token);
                else if (isFloat(token))
                    printToken("Float", token);
                else if (isValidIdentifier(token))
                    printToken("Identifier", token);
                else
                    cerr << "Lexical Error (Line " << lineNo << "): Invalid token '" << token << "'\n";
            }
            else {
                // Invalid single character
                cerr << "Lexical Error (Line " << lineNo << "): Invalid token '" << line[i] << "'\n";
                ++i;
            }
        }
    }

    return 0;
}
