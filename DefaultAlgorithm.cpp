#include <iostream>
#include "DefaultAlgorithm.h"

bool IsValidGuess(const int NewGuess, const std::vector<bullcow_guess> &GuessHistory)
{
    for (const bullcow_guess &OldGuess : GuessHistory) {
        if (!(IsEqualResponse(NewGuess, OldGuess.Guess, OldGuess.Response))) {
            return false;
        }
    }
    return true;
}

int GenerateGuess(const std::vector<bullcow_guess> &GuessHistory)
{
    if (GuessHistory.empty()) {
        return LOWEST_GUESS;
    }
    int Result = GuessHistory.back().Guess + 1;

    while ((!IsPositiveIntegerWithFourUniqueDigits(Result) || !IsValidGuess(Result, GuessHistory)) && Result <= HIGHEST_GUESS) {
        ++Result;
    }

    if (Result == HIGHEST_GUESS + 1) {
        return CHEATING_FLAG;
    }

    return Result;
}

bullcow_result PlayDefault(bool ComputerOnly, std::vector<int> UniqueCombinations, std::vector<bullcow_guess> &GuessHistory, int SecretNumberIndex, bool RiggedNumber)
{
    bullcow_result Result = {0, 0};

    int SecretNumber = RiggedNumber ? UniqueCombinations[SecretNumberIndex] : ComputerOnly ? GeneratePseudoRandomNumberFromSet(UniqueCombinations) : LOWEST_GUESS;
    int Guess = GenerateGuess(GuessHistory);
    int Response;

    while (Guess != CHEATING_FLAG) {
        ++Result.TurnCount;
        
        Response = GetResponse(Guess, SecretNumber, ComputerOnly);
        GuessHistory.emplace_back(bullcow_guess {Guess, Response});

        if (std::cin.eof()) {
            return EOF_INDICATOR;
        }

        if (!ComputerOnly) {
            PrintGameInfo(Result.TurnCount, Guess, Response);
        }

        if (Response == RESPONSE_WIN) {
            Result.SecretNumber = Guess;
            return Result;
        }

        Guess = GenerateGuess(GuessHistory);
    }

    return CHEATING_INDICATOR;
}
