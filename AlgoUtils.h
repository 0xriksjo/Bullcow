#ifndef ALGO_UTILS_H
#define ALGO_UTILS_H

#include <vector>
#include <iostream>
#include <algorithm>
#include <limits>
#include <string>
#include <cstdint>

#define LEADING_ZERO_ALLOWED
#ifdef LEADING_ZERO_ALLOWED
constexpr int LOWEST_GUESS = 123;
constexpr int NUM_UNIQUE_GUESSES = 5040;
#else
constexpr int LOWEST_GUESS = 1023;
constexpr int NUM_UNIQUE_GUESSES = 4536;

#endif


#ifndef INT32_MAX
#define INT32_MAX = 2147483647
#endif

constexpr int SIZE_BULL_TABLE = 65536;
constexpr int SIZE_COW_TABLE = 1024;
constexpr int HIGHEST_GUESS = 9876;
constexpr int SECRET_NUMBER_LENGTH = 4;
constexpr int NUM_DIGITS = 10;
constexpr int NUM_RESPONSE_COMBINATIONS = 14;
constexpr int BULLCOW_RESPONSES[NUM_RESPONSE_COMBINATIONS] = {0xB0C0, 0xB0C1, 0xB1C0, 0xB1C1, 0xB1C2, 0xB2C0, 0xB0C2, 0xB0C3, 0xB3C0, 0xB1C3, 0xB2C1, 0xB0C4, 0xB2C2, 0xB4C0};
constexpr int RESPONSE_WIN = 0xB4C0;
constexpr int CHEATING_FLAG = 0xABCD;

struct bullcow_guess {
    int Guess;
    int Response;
};

struct bullcow_result {
    int TurnCount;
    int SecretNumber;

    bool operator==(const bullcow_result &Other)
    {
        return (this->SecretNumber == Other.SecretNumber) && (this->TurnCount == Other.TurnCount);
    };
};

constexpr bullcow_result EOF_INDICATOR = {-1, -1};
constexpr bullcow_result CHEATING_INDICATOR = {-2, -2};

int ToPackedBullcowNumber(int Number);
int CountSetBits(int Number);
int GetPlayerInputInRange(int Min, int Max);
int GeneratePseudoRandomNumberFromSet(const std::vector<int> &Set);


std::vector<int> GenerateBullLookupTable();
std::vector<int> GenerateCowLookupTable();
std::vector<int> GenerateAllUniqueCombinations();

std::vector<int> BatchGenerateResponseScores(const std::vector<int> &PossibleSolutions, const int UniqueGuess, const std::vector<int> &BullLookupTable, const std::vector<int> &CowLookupTable);
int GetResponseFromPlayer(int Guess);
int FastGenerateResponse(const int GuessA, const int GuessB, const std::vector<int> &BullLookupTable, const std::vector<int> &CowLookupTable);
int GenerateResponse(const int Guess, const int SecretNumber);


bool inline IsPositiveInteger(std::string String) {
    return !String.empty() && String.find_first_not_of("0123456789") == std::string::npos;
}


inline int GetResponseTableIndex(int Response)
{
    for (int Index = 0; Index < NUM_RESPONSE_COMBINATIONS; ++Index) {
        if (BULLCOW_RESPONSES[Index] == Response) {
            return Index;
        }
    }
    return 0;
}

inline int FastGetResponse(const int Guess, const int SecretNumber, bool ComputerOnly, const std::vector<int> &BullLookupTable, const std::vector<int> &CowLookupTable)
{
    return ComputerOnly ? FastGenerateResponse(Guess, SecretNumber, BullLookupTable, CowLookupTable) : GetResponseFromPlayer(Guess);
}

inline int GetResponse(const int Guess, const int SecretNumber, bool ComputerOnly)
{
    return ComputerOnly ? GenerateResponse(Guess, SecretNumber) : GetResponseFromPlayer(Guess);
}

inline bool IsEqualResponse(int NewGuess, int OldGuess, int Response)
{
    return GenerateResponse(NewGuess, OldGuess) == Response;
}

void IntToArray(int Result[SECRET_NUMBER_LENGTH], int Number);

void PrintGameInfo(int TurnCount, int Guess, int Response);

bool IsPositiveIntegerWithFourUniqueDigits(int Number);

bool VerifyResponses(int SecretNumber, std::vector<bullcow_guess> &GuessHistory);

int GetGuessFromInput();

std::string ResponseToString(int Response);

std::string PadToWidth(int Number, int Width, char PadChar = '0');

int HasPlayerCheated(std::vector<bullcow_guess> &GuessHistory);

#endif
