#ifndef DEFAULT_ALGORITHM_H
#define DEFAULT_ALGORITHM_H

#include <vector>
#include "AlgoUtils.h"

bool IsValidGuess(const int NewGuess, const std::vector<bullcow_guess> &Guesses);

int GenerateGuess(const std::vector<bullcow_guess> &Guesses);

bullcow_result PlayDefault(bool ComputerOnly, std::vector<int> UniqueCombinations, std::vector<bullcow_guess> &GuessHistory, int SecretNumberIndex, bool RiggedNumber);

#endif