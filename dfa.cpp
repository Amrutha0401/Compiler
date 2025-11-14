#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <sstream>
#include <iomanip>  // For table formatting

using namespace std;

// Operators
#define UNION '|'
#define STAR '*'
#define CONCAT '.'

int positionCounter = 1;

// Syntax Tree Node
struct Node {
    char symbol;
    Node *left, *right;

    bool nullable;
    set<int> firstpos, lastpos;
    int pos; // valid only for leaf nodes
    Node(char sym) : symbol(sym), left(nullptr), right(nullptr), nullable(false), pos(0) {}
};

// Globals
map<int, set<int>> followposMap;
map<char, set<int>> positionsOfSymbol;
vector<Node*> positionNodes;

// Helpers
bool isOperator(char c) {
    return c == UNION || c == STAR || c == CONCAT;
}

int precedence(char c) {
    if (c == STAR) return 3;
    if (c == CONCAT) return 2;
    if (c == UNION) return 1;
    return 0;
}

// Add explicit concatenation operator
string addConcat(string regex) {
    string res = "";
    for (size_t i = 0; i < regex.length(); ++i) {
        char c1 = regex[i];
        res += c1;
        if (i + 1 < regex.length()) {
            char c2 = regex[i + 1];
            if ((isalnum(c1) || c1 == '#' || c1 == '*' || c1 == ')') &&
                (isalnum(c2) || c2 == '#' || c2 == '(')) {
                res += CONCAT;
            }
        }
    }
    return res;
}

// Convert infix to postfix
string toPostfix(string regex) {
    regex = addConcat(regex);
    stack<char> op;
    string output = "";

    for (char c : regex) {
        if (isalnum(c) || c == '#') {
            output += c;
        } else if (c == '(') {
            op.push(c);
        } else if (c == ')') {
            while (!op.empty() && op.top() != '(') {
                output += op.top(); op.pop();
            }
            if (!op.empty()) op.pop();
        } else {
            while (!op.empty() && precedence(op.top()) >= precedence(c)) {
                output += op.top(); op.pop();
            }
            op.push(c);
        }
    }
    while (!op.empty()) {
        output += op.top(); op.pop();
    }
    return output;
}

// Build Syntax Tree
Node* buildSyntaxTree(string postfix) {
    stack<Node*> st;

    for (char c : postfix) {
        if (isalnum(c) || c == '#') {
            Node* node = new Node(c);
            node->pos = positionCounter++;
            positionNodes.push_back(node);
            positionsOfSymbol[c].insert(node->pos);
            st.push(node);
        } else if (c == STAR) {
            Node* child = st.top(); st.pop();
            Node* node = new Node(c);
            node->left = child;
            st.push(node);
        } else if (c == UNION || c == CONCAT) {
            Node* right = st.top(); st.pop();
            Node* left = st.top(); st.pop();
            Node* node = new Node(c);
            node->left = left;
            node->right = right;
            st.push(node);
        }
    }
    return st.top();
}

// Compute nullable, firstpos, lastpos
void computeNullableFirstLast(Node* node) {
    if (!node) return;
    if (!node->left && !node->right) {
        node->nullable = (node->symbol == 'ε'); // If you're using Greek epsilon symbol
        if (!node->nullable) {
            node->firstpos.insert(node->pos);
            node->lastpos.insert(node->pos);
        }
        return;
    }

    computeNullableFirstLast(node->left);
    computeNullableFirstLast(node->right);

    if (node->symbol == UNION) {
        node->nullable = node->left->nullable || node->right->nullable;
        node->firstpos.insert(node->left->firstpos.begin(), node->left->firstpos.end());
        node->firstpos.insert(node->right->firstpos.begin(), node->right->firstpos.end());
        node->lastpos.insert(node->left->lastpos.begin(), node->left->lastpos.end());
        node->lastpos.insert(node->right->lastpos.begin(), node->right->lastpos.end());
    } else if (node->symbol == CONCAT) {
        node->nullable = node->left->nullable && node->right->nullable;
        if (node->left->nullable) {
            node->firstpos.insert(node->left->firstpos.begin(), node->left->firstpos.end());
            node->firstpos.insert(node->right->firstpos.begin(), node->right->firstpos.end());
        } else {
            node->firstpos = node->left->firstpos;
        }

        if (node->right->nullable) {
            node->lastpos.insert(node->left->lastpos.begin(), node->left->lastpos.end());
            node->lastpos.insert(node->right->lastpos.begin(), node->right->lastpos.end());
        } else {
            node->lastpos = node->right->lastpos;
        }
    } else if (node->symbol == STAR) {
        node->nullable = true;
        node->firstpos = node->left->firstpos;
        node->lastpos = node->left->lastpos;
    }
}

// Compute followpos
void computeFollowpos(Node* node) {
    if (!node) return;
    computeFollowpos(node->left);
    computeFollowpos(node->right);

    if (node->symbol == CONCAT) {
        for (int i : node->left->lastpos) {
            followposMap[i].insert(node->right->firstpos.begin(), node->right->firstpos.end());
        }
    } else if (node->symbol == STAR) {
        for (int i : node->lastpos) {
            followposMap[i].insert(node->firstpos.begin(), node->firstpos.end());
        }
    }
}

// Format a set as string
string formatSet(const set<int>& s) {
    stringstream ss;
    ss << "{";
    for (auto it = s.begin(); it != s.end(); ++it) {
        ss << *it;
        if (next(it) != s.end()) ss << ",";
    }
    ss << "}";
    return ss.str();
}

// Print node info table
void printNodeTable(const vector<Node*>& nodes) {
    cout << "\n=== Syntax Tree Node Table ===\n";
    cout << left << setw(10) << "Symbol"
         << setw(10) << "Pos"
         << setw(12) << "Nullable"
         << setw(18) << "Firstpos"
         << setw(18) << "Lastpos" << "\n";
    cout << string(68, '-') << "\n";

    for (const Node* n : nodes) {
        cout << left << setw(10) << n->symbol
             << setw(10) << n->pos
             << setw(12) << (n->nullable ? "true" : "false")
             << setw(18) << formatSet(n->firstpos)
             << setw(18) << formatSet(n->lastpos) << "\n";
    }
}

// Print followpos table
void printFollowposTable(const map<int, set<int>>& fmap) {
    cout << "\n=== Followpos Table ===\n";
    cout << left << setw(15) << "Position" << "Followpos\n";
    cout << string(35, '-') << "\n";
    for (map<int, set<int>>::const_iterator it = fmap.begin(); it != fmap.end(); ++it) {
    int pos = it->first;
    const set<int>& follow = it->second;
    cout << left << setw(15) << pos << formatSet(follow) << "\n";
}

}

int main() {
    string input;
    cout << "Enter Regular Expression (with #): ";
    cin >> input;

    string postfix = toPostfix(input);
    Node* root = buildSyntaxTree(postfix);
    computeNullableFirstLast(root);
    computeFollowpos(root);

    printNodeTable(positionNodes);

    cout << "\n=== Root Info ===\n";
    cout << "Root Nullable : " << (root->nullable ? "true" : "false") << "\n";
    cout << "Root Firstpos : " << formatSet(root->firstpos) << "\n";
    cout << "Root Lastpos  : " << formatSet(root->lastpos) << "\n";

    printFollowposTable(followposMap);

    return 0;
}

