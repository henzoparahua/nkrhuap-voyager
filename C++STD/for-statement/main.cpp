#include <iostream>
using namespace std;

int main() {
    // The counter variable can be declared in the init-expression.
    for (int i = 0; i < 2; i++) {
        cout << i;
    }
    // The counter variable can be declared outside the for loop.
    int i;
    for (i = 0; i < 2; i++) {
        cout << i;
    }
    // These for loops are the equivalent of a while loop.
    i = 0;
    while (i < 2) {
        cout << i++;
    }
}