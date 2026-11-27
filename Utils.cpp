#include "Utils.h"

std::string GetFilename()
{
    std::string Input;
    bool HasReceivedValidInput = false;

    while (!HasReceivedValidInput) {
        std::cout << "Enter a filename: ";
        std::getline(std::cin, Input);

        if (std::cin.eof()) {
            return "";
        }

        int Length = Input.length();
        if (Length > 0 && Length <= REASONABLE_FILENAME_MAX) {
            HasReceivedValidInput = true;
        }
    }
    return Input;
}

char GetChar()
{
    std::string Input;
    bool HasReceivedValidInput = false;
    while (!HasReceivedValidInput) {
        std::cout << "Enter your selection: ";
        std::getline(std::cin, Input);

        if (std::cin.eof()) {
            return EOF;
        }

        if (Input.length() == 1) {
            HasReceivedValidInput = true;
        } else if (!Input.empty()) {
            std::cerr << "InputError: expected a single character!\n";
        }
    }
    return Input[0];
}
