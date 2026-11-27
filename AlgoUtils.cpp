#include <cstdlib>
#include <ctime>
#include "Utils.h"
#include "AlgoUtils.h"
#include <iomanip>
#include <sstream>

int GetResponseFromPlayer(int Guess)
{
    std::cout << "The computer guessed: " << PadToWidth(Guess, SECRET_NUMBER_LENGTH) << " how do you respond?\n";
    std::cout << "Respond in the format BXCY where X and Y (0-4) are the number of Bulls and Cows!\n";

    int Result;
    bool HasReceivedValidInput = false;
    std::string Input;

    while (!HasReceivedValidInput) {
        std::cout << "Response: ";
        std::getline(std::cin, Input);

        if (std::cin.eof()) {
            return EOF;
        }

        if (Input.length() != SECRET_NUMBER_LENGTH) {
            std::cerr << "InputError: invalid string size, expected four characters (BXCY), got " << Input.length() << "!\n";
        } else {
            if (ToLower(Input[0]) == 'b' && ToLower(Input[2]) == 'c') {
                int Test = (0xB << 12) | ((Input[1] - '0') << 8) | (0xC << 4) | (Input[3] - '0');
                if (BULLCOW_RESPONSES[(GetResponseTableIndex(Test))] == Test) {
                    Result = Test;
                    HasReceivedValidInput = true;
                } else {
                    std::cerr << "InputError: invalid amount of bulls and cows!\n";
                }

            } else {
                std::cerr << "InputError: expected BXCY where X is the number of bulls and Y the number of cows!\n";
            }
        }
    }
    return Result;
}

int GetPlayerInputInRange(int Min, int Max)
{
    std::string Input;
    int Result;

    bool HasReceivedValidInput = false;
    while (!HasReceivedValidInput) {
        std::cout << "Enter a number in the range [" << Min << ", " << Max << "]: ";
        std::getline(std::cin, Input);

        if (std::cin.eof()) {
            return EOF;
        }

        if (IsPositiveInteger(Input)) {
            if (Input.length() <= NumberOfDigitsInNumber(INT32_MAX)) {
                int Test = std::stoi(Input);
                if (Test >= Min && Test <= Max) {
                    HasReceivedValidInput = true;
                    Result = Test;
                } else {
                    std::cerr << "InputError: expected a number in the range [" << Min << ", " << Max << "]!\n";
                }
            } else {
                std::cerr << "InputError: don't try to crash this program by entering large numbers!\n";
            }
        } else {
            std::cerr << "InputError: expected a number in the range [" << Min << ", " << Max << "], got non-numeric characters!\n";
        }
    }

    return Result;
}

bool IsPositiveIntegerWithFourUniqueDigits(int Number)
{
    if (Number < LOWEST_GUESS || Number > HIGHEST_GUESS) {
        return false;
    }

    bool SeenDigits[NUM_DIGITS] = {0};
#ifdef LEADING_ZERO_ALLOWED
    if (Number < 1000) {
        SeenDigits[0] = true;
    }
#endif

    while (Number > 0) {
        int Digit = Number % NUM_DIGITS;
        if (SeenDigits[Digit]) {
            return false;
        }
        SeenDigits[Digit] = true;
        Number /= NUM_DIGITS;
    }

    return true;
}

bool VerifyResponses(int SecretNumber, std::vector<bullcow_guess> &GuessHistory)
{
    int Errors = 0;
    for (size_t Index = 0; Index < GuessHistory.size(); Index++) {
        int OldResponse = GuessHistory[Index].Response;
        int NewResponse = GenerateResponse(GuessHistory[Index].Guess, SecretNumber);
        if (NewResponse != OldResponse) {
            std::cout << "Your response on turn " << Index + 1 << " was not correct!\n";
            std::cout << "You responded " << ResponseToString(OldResponse) << " when you should have responded " << ResponseToString(NewResponse) << ".\n";
            Errors++;
        }
    }
    return (Errors == 0);
}

int GetGuessFromInput()
{
    int Result;
    std::string Input;
    bool HasReceivedValitInput = false;

    while (!HasReceivedValitInput) {
#ifdef LEADING_ZERO_ALLOWED
        std::cout << "Enter a number with four unique digits (0XYZ is allowed!): ";
#else
        std::cout << "Enter a number with four unique digits: ";
#endif
        std::getline(std::cin, Input);

        if (std::cin.eof()) {
            return EOF;
        }

        if (IsPositiveInteger(Input) && Input.length() == 4) {
            if (IsPositiveIntegerWithFourUniqueDigits(std::stoi(Input))) {
                Result = std::stoi(Input);
                HasReceivedValitInput = true;
            } else {
                std::cerr << "InputError: expected a number with four unique digits!\n";
            }
        } else {
            std::cerr << "InputError: expected a number with four unique digits, got non-numeric character(s)!\n";
        }
    }

    return Result;
}

void PrintGameInfo(int TurnCount, int Guess, int Response)
{
    std::cout << "Turn: " << TurnCount << '\n';
    std::cout << "Guess:    " << PadToWidth(Guess, SECRET_NUMBER_LENGTH) << '\n';
    std::cout << "Response: " << ResponseToString(Response) << '\n';
}

int GeneratePseudoRandomNumberFromSet(const std::vector<int> &Set)
{
    int Result;
    Result = Set[rand() % Set.size()];
    return Result;
}

std::vector<int> GenerateAllUniqueCombinations()
{
    std::vector<int> Result = {};
    Result.reserve(NUM_UNIQUE_GUESSES);
    for (int Number = LOWEST_GUESS; Number <= HIGHEST_GUESS; Number++) {
        if (IsPositiveIntegerWithFourUniqueDigits(Number)) {
            Result.emplace_back(Number);
        }
    }

    return Result;
}

// https://www.tanaka.ecc.u-tokyo.ac.jp/ktanaka/moo/find_sol.c,


std::vector<int> GenerateBullLookupTable()
{
    std::vector<int> Result(SIZE_BULL_TABLE, 0);
    Result.reserve(SIZE_BULL_TABLE);
    const int NIBBLE_SIZE = 4;
    const int NIBBLES_PER_NUMBER = NIBBLE_SIZE;

    for (int Number = 0; Number < SIZE_BULL_TABLE; Number++) {
        int Bulls = 0;
        for (int Nibble = 0; Nibble < NIBBLES_PER_NUMBER; Nibble++) {
            if (((Number >> (Nibble * NIBBLE_SIZE)) & 0xF) == 0) {
                Bulls++;
            }
        }
        Result[Number] = Bulls;
    }

    return Result;
}

std::vector<int> GenerateCowLookupTable()
{
    std::vector<int> Result(SIZE_COW_TABLE, 0);
    Result.reserve(SIZE_COW_TABLE);

    for (int Number = 0; Number < SIZE_COW_TABLE; Number++) {
        // int Cows = 0;
        // for (int Digit = 0; Digit < NUM_DIGITS; Digit++) {
        //     if ((Number >> Digit) & 0x1) {
        //         Cows++;
        //     }
        // }
        // Result[Number] = Cows;
        Result[Number] = CountSetBits(Number);
    }
    return Result;
}

// https://arxiv.org/pdf/2207.04845

int ToBullcowNumber(int Number)
{
    int Result = 0;
    int DigitsToShift = 3;
    int ShiftFactor = 4;

    for (int DigitIndex = 0; DigitIndex <= DigitsToShift; ++DigitIndex) {
        int Digit = Number % 10;
        Result |= Digit << (ShiftFactor * DigitIndex);
        Result ^= 1 << (25 - Digit);
        Number /= 10;
    }

    return Result;
}

int CountSetBits(int Number)
{
    return __builtin_popcount(Number);
}

int FastGenerateResponse(const int GuessA, const int GuessB, const std::vector<int> &BullLookupTable, const std::vector<int> &CowLookupTable)
{
    int BullcowNumberA = ToBullcowNumber(GuessA);
    int BullcowNumberB = ToBullcowNumber(GuessB);

    // https://arxiv.org/pdf/2207.04845

    int Bulls = BullLookupTable[(BullcowNumberA ^ BullcowNumberB) & 0xFFFF];
    int Cows = CowLookupTable[(BullcowNumberA & BullcowNumberB) >> 16];

    int Result = (0xB << 12);

    Result = Result | (Bulls << 8) | (0xC << 4) | (Cows - Bulls);
    return Result;
}

std::vector<int> BatchGenerateResponseScores(const std::vector<int> &PossibleSolutions, const int UniqueGuess, const std::vector<int> &BullLookupTable, const std::vector<int> &CowLookupTable)
{
    int TestGuessBinary, Bulls, Cows, Response;
    std::vector<int> Result(NUM_RESPONSE_COMBINATIONS, 0);
    int UniqueGuessBinary = ToBullcowNumber(UniqueGuess);

    for (size_t TestGuess = 0; TestGuess < PossibleSolutions.size(); ++TestGuess) {
        TestGuessBinary = ToBullcowNumber(PossibleSolutions[TestGuess]);
        Bulls = BullLookupTable[(UniqueGuessBinary ^ TestGuessBinary) & 0xFFFF];
        Cows = CowLookupTable[(UniqueGuessBinary & TestGuessBinary) >> 16];
        Response = (0xB << 12) | (Bulls << 8) | (0xC << 4) | (Cows - Bulls);
        Result[GetResponseTableIndex(Response)]++;
    }

    return Result;
}

void IntToArray(int Result[SECRET_NUMBER_LENGTH], int Number)
{
    int DigitIndex = SECRET_NUMBER_LENGTH - 1;

    if (Number < 1000) {
        Result[0] = 0;
    }

    while (Number > 0) {
        int Digit = Number % NUM_DIGITS;
        Result[DigitIndex--] = Digit;
        Number /= 10;
    }
}

int GenerateResponse(const int GuessA, const int GuessB)
{
    int Result = (0xB << 12);

    int ArrayA[SECRET_NUMBER_LENGTH], ArrayB[SECRET_NUMBER_LENGTH];
    IntToArray(ArrayA, GuessA);
    IntToArray(ArrayB, GuessB);

    int Bulls = 0, Cows = 0;

    for (int DigitIndexA = 0; DigitIndexA < SECRET_NUMBER_LENGTH; ++DigitIndexA) {
        for (int DigitIndexB = 0; DigitIndexB < SECRET_NUMBER_LENGTH; ++DigitIndexB) {
            Bulls += (ArrayA[DigitIndexA] == ArrayB[DigitIndexB]) && (DigitIndexA == DigitIndexB);
            Cows += (ArrayA[DigitIndexB] == ArrayB[DigitIndexA]) && (DigitIndexA != DigitIndexB);
        }
    }

    Result = Result | (Bulls << 8) | (0xC << 4) | Cows;
    return Result;
}

std::string PadToWidth(int Number, int Width, char PadChar)
{
    std::string Result = std::to_string(Number);
    int Length = Result.length();
    if (Length < Width) {
        Result.insert(Result.begin(), Width - Length, PadChar);
    }

    return Result;
}

std::string ResponseToString(int Response)
{
    std::ostringstream StringStream;
    StringStream << std::uppercase << std::hex << Response;

    return StringStream.str();
}

int HasPlayerCheated(std::vector<bullcow_guess> &GuessHistory)
{
    std::cout << "The Bullcow Machine suspects you were cheating.\n";
    std::cout << "What number were you thinking of?\n";
    int SecretNumber = GetGuessFromInput();

    if (SecretNumber == EOF) {
        return EOF;
    }

    if (!VerifyResponses(SecretNumber, GuessHistory)) {
        return 1;
    }

    return 0;
}
