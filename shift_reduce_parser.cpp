#include <iostream>
#include <stack>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Production {
    vector<string> rhs;
    string lhs;
};

vector<string> tokenize(const string &str) {
    stringstream ss(str);
    string token;
    vector<string> tokens;
    while (ss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

bool matchProduction(stack<string> &stk, const Production &prod) {
    int n = prod.rhs.size();
    if (stk.size() < n) return false;

    vector<string> topElems(n);
    stack<string> temp = stk;
    for (int i = n - 1; i >= 0; i--) {
        topElems[i] = temp.top();
        temp.pop();
    }
    for (int i = 0; i < n; i++) {
        if (topElems[i] != prod.rhs[i])
            return false;
    }
    return true;
}

void doReduce(stack<string> &stk, const Production &prod) {
    int n = prod.rhs.size();
    for (int i = 0; i < n; i++)
        stk.pop();
    stk.push(prod.lhs);
}

string stackToString(stack<string> stk) {
    vector<string> elems;
    while (!stk.empty()) {
        elems.push_back(stk.top());
        stk.pop();
    }
    reverse(elems.begin(), elems.end());
    string result;
    for (auto &e : elems) result += e + " ";
    if (!result.empty()) result.pop_back(); 
    return result;
}

string inputBufferToString(const vector<string> &tokens, int ip) {
    string result;
    for (int i = ip; i < (int)tokens.size(); i++) {
        result += tokens[i] + " ";
    }
    if (!result.empty()) result.pop_back();
    return result;
}

int main() {
    vector<Production> productions;
    int n;

    cout << "Enter number of productions in the grammar: ";
    cin >> n;
    cin.ignore();

    cout << "Enter productions one per line in the form: LHS -> RHS (space separated)" << endl;
    cout << "Example: E -> E + E" << endl;
    cout << "(Use spaces to separate symbols in RHS.)" << endl;

    for (int i = 0; i < n; i++) {
        string line;
        getline(cin, line);

        size_t arrowPos = line.find("->");
        if (arrowPos == string::npos) {
            cout << "Invalid production format. Use LHS -> RHS\n";
            i--;
            continue;
        }

        string lhs = line.substr(0, arrowPos);
        lhs.erase(remove(lhs.begin(), lhs.end(), ' '), lhs.end()); 

        string rhsStr = line.substr(arrowPos + 2);
        vector<string> rhs = tokenize(rhsStr);

        if (lhs.empty() || rhs.empty()) {
            cout << "Empty LHS or RHS not allowed\n";
            i--;
            continue;
        }

        productions.push_back({rhs, lhs});
    }

    cout << "\nEnter input strings to parse (tokens separated by spaces)." << endl;
    cout << "Enter '0' to quit.\n";

    while (true) {
        cout << "\nInput string(Enter 0 to exit): ";
        string input;
        getline(cin, input);

        if (input == "0") {
            cout << "Exiting parser.\n";
            break;
        }

        vector<string> tokens = tokenize(input);
        tokens.push_back("$");

        stack<string> stk;
        stk.push("$");

        int ip = 0; 

        cout << left
             << setw(25) << "Stack"
             << setw(25) << "Input Buffer"
             << "Action"
             << "\n";
        cout << string(65, '-') << "\n";

        bool error_occurred = false;
        bool accepted = false;

        while (true) {
            string stackStr = stackToString(stk);
            string inputBufStr = inputBufferToString(tokens, ip);
            string actionStr;

            if (stk.size() == 2 && stk.top() == productions[0].lhs && tokens[ip] == "$") {
                actionStr = "Accept";
                cout << left << setw(25) << stackStr << setw(25) << inputBufStr << actionStr << "\n";
                accepted = true;
                break;
            }

            bool reduced = false;
            for (auto &prod : productions) {
                if (matchProduction(stk, prod)) {
                    actionStr = "Reduce by: " + prod.lhs + " ->";
                    for (auto &s : prod.rhs) actionStr += " " + s;
                    cout << left << setw(25) << stackStr << setw(25) << inputBufStr << actionStr << "\n";
                    doReduce(stk, prod);
                    reduced = true;
                    break;
                }
            }

            if (reduced) continue;

            if (tokens[ip] != "$") {
                actionStr = "Shift " + tokens[ip];
                cout << left << setw(25) << stackStr << setw(25) << inputBufStr << actionStr << "\n";
                stk.push(tokens[ip]);
                ip++;
            } else {
                actionStr = "Error: Unable to parse input";
                cout << left << setw(25) << stackStr << setw(25) << inputBufStr << actionStr << "\n";
                error_occurred = true;
                break;
            }
        }

        if (!accepted && !error_occurred) {  
            cout << "Parsing failed.\n";
        }
    }

    return 0;
}
