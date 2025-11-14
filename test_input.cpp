#include <iostream>

// This is a single-line comment

/*
   This is a multi-line comment
   It should be ignored by the lexical analyzer
*/

int main() {
    int a = 10, b = 20;
    float result = a + b / 2.5;
    
    if (a < b) {
        std::cout << "A is less than B" << std::endl;
    } else {
        std::cout << "B is less or equal to A" << std::endl;
    }

    return 0;
}

// Invalid tokens below
@invalid_token
