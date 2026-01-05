#include "Calculator.h"

using namespace std;

int main()
{
    std::string input;
    double result = 0.0;
    char oper = '+';

    cout << "Calculator Console Application" << endl << endl;
    cout << "Please enter the operation to perform. Format: a+b | a-b | a*b | a/b"
        << endl;

    Calculator c;
    while (true)
    {
        cin >> input;
        if (input == "quit")
            break;

        result = c.Calculate(input);
        cout << "Result " << "of " << input << " is: " << result << endl;
    }

    return 0;
}