#ifndef KNUTH_H
#define KNUTH_H

#include <algorithm>
#include <vector>
#include "AlgoUtils.h"

struct guess_score {
    int Guess;
    int Value;
};

bullcow_result PlayKnuth(bool ComputerOnly, std::vector<int> UniqueCombinations, std::vector<bullcow_guess> &GuessHistory, int SecretNumberIndex, bool RiggedNumber);

std::vector<int> Minimax(const std::vector<int> &PossibleSolutions, const std::vector<int> &UniqueCombinations, const std::vector<int> &BullLookuptable, const std::vector<int> &CowLookuptable);

void PruneSet(std::vector<int> &Set, const int PreviousGuess, const int Response, const std::vector<int> &BullLookupTable, const std::vector<int> &CowLookupTable);
void EraseGuess(std::vector<int> &AvailableGuesses, int Guess);

int GetMaxScore(const std::vector<int> &GuessScores);
int GetMinScore(const std::vector<guess_score> &GuessScores);
int GetNextGuess(const std::vector<int> &OptimalGuesses, const std::vector<int> &AvailableGuesses, const std::vector<int> &UniqueCombinations);

#endif