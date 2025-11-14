#include <iostream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;

vector<string> tokenize(const string& str, char delimiter) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimiter)) {
        if (!token.empty()) {
            tokens.push_back(token);
        }
    }
    return tokens;
}

string trim(const string& str) {
    // This handles regular spaces, tabs, etc.
    const string WHITESPACE = " \t\n\r\f\v";
    size_t first = str.find_first_not_of(WHITESPACE);
    if (string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(WHITESPACE);
    return str.substr(first, (last - first + 1));
}


class OperatorPrecedenceParser {
private:
    map<string, vector<vector<string>>> grammar;
    set<string> non_terminals;
    set<string> terminals;
    string start_symbol;

    map<string, set<string>> leading;
    map<string, set<string>> trailing;
    map<string, map<string, char>> precedence_table;

    void parse_grammar(const string& grammar_str) {
        size_t arrow_pos = grammar_str.find("->");
        if (arrow_pos == string::npos) {
            cout << "Invalid grammar format. Use 'NT -> productions'." << endl;
            return;
        }
        string lhs_str = trim(grammar_str.substr(0, arrow_pos));
        string rhs_str = trim(grammar_str.substr(arrow_pos + 2));

        start_symbol = lhs_str;
        non_terminals.insert(lhs_str);

        vector<string> productions = tokenize(rhs_str, '|');
        for (const auto& prod_str : productions) {
            string trimmed_prod = trim(prod_str);
            vector<string> prod_tokens = tokenize(trimmed_prod, ' ');
            grammar[lhs_str].push_back(prod_tokens);
            for (const auto& token : prod_tokens) {
                 bool is_non_terminal = true;
                 for(char c : token){
                     if(!isupper(c)){
                        is_non_terminal = false;
                        break;
                     }
                 }
                if (is_non_terminal && !token.empty()) {
                    non_terminals.insert(token);
                } else if (!token.empty()) {
                    terminals.insert(token);
                }
            }
        }
    }

    void compute_leading_trailing() {
        for (const auto& nt : non_terminals) {
            leading[nt] = set<string>();
            trailing[nt] = set<string>();
        }

        bool changed = true;
        while (changed) {
            changed = false;
            for (const auto& pair : grammar) {
                string nt = pair.first;
                for (const auto& prod : pair.second) {
                    if (!prod.empty()) {
                        // LEADING
                        string first_symbol = prod[0];
                        size_t old_leading_size = leading[nt].size();
                        if (terminals.count(first_symbol)) {
                            leading[nt].insert(first_symbol);
                        } else if(non_terminals.count(first_symbol)) { 
                            leading[nt].insert(leading[first_symbol].begin(), leading[first_symbol].end());
                            if (prod.size() > 1 && terminals.count(prod[1])) {
                                leading[nt].insert(prod[1]);
                            }
                        }
                        if (leading[nt].size() > old_leading_size) changed = true;
                    }

                     if (!prod.empty()) {
                        // TRAILING
                        string last_symbol = prod.back();
                        size_t old_trailing_size = trailing[nt].size();
                        if (terminals.count(last_symbol)) {
                             trailing[nt].insert(last_symbol);
                        } else if (non_terminals.count(last_symbol)) {
                            trailing[nt].insert(trailing[last_symbol].begin(), trailing[last_symbol].end());
                            if (prod.size() > 1 && terminals.count(prod[prod.size() - 2])) {
                                trailing[nt].insert(prod[prod.size() - 2]);
                            }
                        }
                        if (trailing[nt].size() > old_trailing_size) changed = true;
                    }
                }
            }
        }
    }

    void build_precedence_table() {
        set<string> all_terminals = terminals;
        all_terminals.insert("$");
        for(const auto& t1 : all_terminals) {
            for(const auto& t2 : all_terminals) {
                precedence_table[t1][t2] = ' '; 
            }
        }

        for (const auto& pair : grammar) {
            for (const auto& prod : pair.second) {
                for (size_t i = 0; i < prod.size() - 1; ++i) {
                    string s1 = prod[i];
                    string s2 = prod[i+1];
                    if (terminals.count(s1) && terminals.count(s2)) precedence_table[s1][s2] = '=';
                    if (i < prod.size() - 2 && terminals.count(s1) && non_terminals.count(s2) && terminals.count(prod[i+2])) precedence_table[s1][prod[i+2]] = '=';
                    if (terminals.count(s1) && non_terminals.count(s2)) {
                        for (const auto& term : leading[s2]) precedence_table[s1][term] = '<';
                    }
                    if (non_terminals.count(s1) && terminals.count(s2)) {
                         for (const auto& term : trailing[s1]) precedence_table[term][s2] = '>';
                    }
                }
            }
        }
        
        map<string, int> prec;
        prec["+"] = 1; prec["-"] = 1; prec["*"] = 2; prec["/"] = 2;
        vector<string> operators = {"+", "-", "*", "/"};

        for (const auto& op1 : operators) {
            for (const auto& op2 : operators) {
                precedence_table[op1][op2] = (prec[op1] >= prec[op2]) ? '>' : '<';
            }
        }

        for(const auto& op : operators){
            precedence_table[op]["("] = '<';
            precedence_table[op]["id"] = '<';
            precedence_table[")"][op] = '>';
            precedence_table["id"][op] = '>';
        }

        precedence_table["("]["("] = '<';
        precedence_table["("]["id"] = '<';
        precedence_table[")"][")"] = '>';
        precedence_table["id"][")"] = '>';
        precedence_table["("][")"] = '=';

        for (const auto& term : leading[start_symbol]) precedence_table["$"][term] = '<';
        for (const auto& term : trailing[start_symbol]) precedence_table[term]["$"] = '>';
         for (const auto& op : operators){
            precedence_table[")"][op] = '>';
            precedence_table[op][")"] = '>';
            precedence_table["$"][")"] = ' ';
            precedence_table["("]["$"] = ' ';
         }
    }

public:
    void setup() {
        cout << "Enter the grammar :" << endl;
        string grammar_line;
        getline(cin, grammar_line);
        parse_grammar(grammar_line);
        compute_leading_trailing();
        build_precedence_table();
        print_leading_trailing();
        print_precedence_table();
    }
    
    void print_leading_trailing() {
        cout << "\n--- LEADING and TRAILING Sets ---\n";
        cout << left << setw(15) << "Non-Terminal" << setw(30) << "LEADING" << setw(30) << "TRAILING" << endl;
        cout << string(75, '-') << endl;
        for (const auto& nt : non_terminals) {
            cout << left << setw(15) << nt;
            string lead_str = "{ ";
            for (const auto& term : leading[nt]) lead_str += term + ", ";
            if(lead_str.length() > 2) { lead_str.pop_back(); lead_str.pop_back(); }
            lead_str += " }";
            cout << left << setw(30) << lead_str;
            string trail_str = "{ ";
            for (const auto& term : trailing[nt]) trail_str += term + ", ";
            if(trail_str.length() > 2) { trail_str.pop_back(); trail_str.pop_back(); }
            trail_str += " }";
            cout << left << setw(30) << trail_str << endl;
        }
    }

    void print_precedence_table() {
        cout << "\n--- Operator Precedence Table ---\n";
        vector<string> term_list = {"id", "+", "-", "*", "/", "(", ")", "$"};
        cout << setw(6) << " ";
        for (const auto& term : term_list) {
            cout << setw(6) << term;
        }
        cout << endl << string(6 * (term_list.size() + 1), '-') << endl;

        for (const auto& row_term : term_list) {
            cout << setw(6) << row_term;
            for (const auto& col_term : term_list) {
                char rel = ' ';
                if(precedence_table.count(row_term) && precedence_table.at(row_term).count(col_term)){
                    rel = precedence_table.at(row_term).at(col_term);
                }
                cout << setw(6) << rel;
            }
            cout << endl;
        }
    }

    void parse(const string& input_str) {
        cout << "\n--- Parsing Input String: \"" << input_str << "\" ---\n";
        cout << left << setw(45) << "STACK" << setw(40) << "INPUT" << "ACTION" << endl;
        cout << string(95, '-') << endl;

        vector<string> input_buffer = tokenize(input_str, ' ');
        input_buffer.push_back("$");

        vector<string> stack;
        stack.push_back("$");

        while (true) {
            string stack_str;
            for(const auto& s : stack) stack_str += s + " ";
            string input_str_rem;
            for(const auto& s : input_buffer) input_str_rem += s + " ";
            cout << left << setw(45) << stack_str << setw(40) << input_str_rem;

            string top_terminal = "$";
            for (int i = stack.size() - 1; i >= 0; --i) {
                if (terminals.count(stack[i]) || stack[i] == "$") {
                    top_terminal = stack[i];
                    break;
                }
            }
            string current_input = input_buffer.front();

            if (top_terminal == "$" && current_input == "$") {
                cout << "Accept" << endl;
                cout << "\nString successfully parsed.\n" << endl;
                return;
            }

            char relation = ' ';
            if(precedence_table.count(top_terminal) && precedence_table.at(top_terminal).count(current_input)) {
                relation = precedence_table.at(top_terminal).at(current_input);
            }

            if (relation == '<' || relation == '=') {
                cout << "Shift" << endl;
                stack.push_back(current_input);
                input_buffer.erase(input_buffer.begin());
            } else if (relation == '>') {
                cout << "Reduce";
                
                // ... inside the "Reduce" block
                int handle_start_index = 1; // Default to the position after '$'
                
                // Find the top-most terminal on the stack to start our search from
                int top_terminal_idx = -1;
                for(int i = stack.size() - 1; i >= 0; --i) {
                    if(terminals.count(stack[i])) {
                        top_terminal_idx = i;
                        break;
                    }
                }

                if (top_terminal_idx != -1) {
                    // Scan backwards from the top-most terminal to find the '<' relationship
                    // which marks the beginning of the handle.
                    for (int i = top_terminal_idx; i > 0; --i) {
                        if (terminals.count(stack[i])) {
                            // Find the terminal just before the current one
                            string prev_terminal = "$";
                            int prev_terminal_idx = 0;
                            for (int j = i - 1; j >= 0; --j) {
                                if (terminals.count(stack[j]) || stack[j] == "$") {
                                    prev_terminal = stack[j];
                                    prev_terminal_idx = j;
                                    break;
                                }
                            }
                            
                            if (precedence_table.at(prev_terminal).at(stack[i]) == '<') {
                                // The handle starts right AFTER the previous terminal.
                                handle_start_index = prev_terminal_idx + 1; // <-- CORRECTED LOGIC
                                break;
                            }
                        }
                    }
                }

                string handle_str;
                for(size_t i = handle_start_index; i < stack.size(); ++i) handle_str += stack[i] + " ";
                cout << " (handle: " << handle_str << ")" << endl;

                stack.erase(stack.begin() + handle_start_index, stack.end());
                stack.push_back(start_symbol);

            } else {
                 cout << "Error" << endl;
                 cout << "\nSyntax Error: No relation between stack top terminal '" << top_terminal << "' and input '" << current_input << "'" << endl;
                 cout << "\nString rejected.\n" << endl;
                 return;
            }
        }
    }
};

int main() {
    OperatorPrecedenceParser parser;
    parser.setup();

    while (true) {
        cout << "\nEnter an input string to parse (or '0' to exit):" << endl;
        string input_line;
        getline(cin, input_line);

        if (input_line == "0") {
            break;
        }
        parser.parse(input_line);
    }

    return 0;
}

