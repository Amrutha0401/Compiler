#include <bits/stdc++.h> 
using namespace std; 
int nextinstr = 100; 
// Structure to represent Boolean expressions 
struct BoolExpr { 
    vector<int> truelist; 
    vector<int> falselist; 
}; 
// Function to create a new list with one instruction index 
vector<int> makelist(int i) { 
    cout << "makelist(" << i << ") => [" << i << "]\n"; 
    return vector<int>(1, i); 
} 
 
// Merge two lists 
vector<int> merge(vector<int> list1, vector<int> list2) { 
    vector<int> result = list1; 
    result.insert(result.end(), list2.begin(), list2.end()); 
    cout << "merge("; 
    for (int i : list1) cout << i << " "; 
    cout << ", "; 
    for (int i : list2) cout << i << " "; 
    cout << ") => ["; 
    for (int i : result) cout << i << " "; 
    cout << "]\n"; 
    return result; 
} 
 
// Fill target address in jump instructions 
void backpatch(vector<int> lst, int target, map<int, string>& code) { 
    cout << "backpatch(["; 
    for (int i : lst) cout << i << " "; 
    cout << "], " << target << ")\n"; 
    for (int i : lst) code[i] += " " + to_string(target); 
} 
 
int main() { 
    map<int, string> code; 
 
    cout << "\n===== THREE ADDRESS CODE GENERATION USING BACKPATCHING =====\n"; 
    cout << "Expression: ((a < b) or (a == b)) and (c > d)\n\n"; 
 
    // Step 1: (a < b) 
    cout << "Step 1: Evaluate (a < b)\n"; 
    BoolExpr E1; 
    code[nextinstr] = "if a < b goto"; 
    E1.truelist = makelist(nextinstr); 
    nextinstr++; 
    code[nextinstr] = "goto"; 
    E1.falselist = makelist(nextinstr); 
    nextinstr++; 
    cout << "E1.truelist = [100], E1.falselist = [101]\n\n"; 
 
    // Step 2: (a == b) 
    cout << "Step 2: Evaluate (a == b)\n"; 
    BoolExpr E2; 
    code[nextinstr] = "if a == b goto"; 
    E2.truelist = makelist(nextinstr); 
    nextinstr++; 
    code[nextinstr] = "goto"; 
    E2.falselist = makelist(nextinstr); 
    nextinstr++; 
    cout << "E2.truelist = [102], E2.falselist = [103]\n\n"; 
 
    // Step 3: (a<b) or (a==b) 
    cout << "Step 3: Combine E1 and E2 using OR\n"; 
    BoolExpr E3; 
    backpatch(E1.falselist, 102, code); 
    E3.truelist = merge(E1.truelist, E2.truelist); 
    E3.falselist = E2.falselist; 
    cout << "E3.truelist = [100, 102], E3.falselist = [103]\n\n"; 
 
    // Step 4: (c > d) 
    cout << "Step 4: Evaluate (c > d)\n"; 
    BoolExpr E4; 
    code[nextinstr] = "if c > d goto"; 
    E4.truelist = makelist(nextinstr); 
    nextinstr++; 
    code[nextinstr] = "goto"; 
    E4.falselist = makelist(nextinstr); 
    nextinstr++; 
    cout << "E4.truelist = [104], E4.falselist = [105]\n\n"; 
 
    // Step 5: Combine using AND 
    cout << "Step 5: Combine E3 and E4 using AND\n"; 
    BoolExpr Efinal; 
    backpatch(E3.truelist, 104, code); 
    Efinal.truelist = E4.truelist; 
    Efinal.falselist = merge(E3.falselist, E4.falselist); 
    cout << "Efinal.truelist = [104], Efinal.falselist = [103, 105]\n\n"; 
 
    // Print generated TAC 
    cout << "===== GENERATED THREE ADDRESS CODE =====\n"; 
    for (auto &c : code) 
        cout << c.first << ": " << c.second << "\n"; 
 
    // Final lists 
    cout << "\n===== FINAL CONTROL FLOW LISTS =====\n"; 
    cout << "True List  : "; 
    for (int i : Efinal.truelist) cout << i << " "; 
    cout << "\nFalse List : "; 
    for (int i : Efinal.falselist) cout << i << " "; 
    cout << "\n"; 
 
    cout << "\n===== SUMMARY =====\n"; 
    cout << "makelist(i): Creates a new list with instruction i\n"; 
    cout << "merge(p1,p2): Joins two lists into one\n"; 
    cout << "backpatch(list, target): Fills in jump targets for incomplete instructions\n"; 
 
    return 0; 
}
