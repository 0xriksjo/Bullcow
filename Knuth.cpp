#include "AlgoUtils.h"
#include "Knuth.h"

bullcow_result PlayKnuth(bool ComputerOnly, std::vector<int> UniqueCombinations, std::vector<bullcow_guess> &GuessHistory, int SecretNumberIndex, bool RiggedNumber)
{

    std::vector<int> PossibleSolutions = UniqueCombinations;
    const std::vector<int> BullLookupTable = GenerateBullLookupTable();
    const std::vector<int> CowLookupTable = GenerateCowLookupTable();
    std::vector<int> OptimalGuesses;
    bullcow_result Result = {0, 0};

    int SecretNumber = RiggedNumber ? UniqueCombinations[SecretNumberIndex] : ComputerOnly ? GeneratePseudoRandomNumberFromSet(UniqueCombinations) : 0;

    int Guess = LOWEST_GUESS;
    int Response;

    while (Guess != CHEATING_FLAG) {
        ++Result.TurnCount;

        Response = FastGetResponse(Guess, SecretNumber, ComputerOnly, BullLookupTable, CowLookupTable);

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

        GuessHistory.emplace_back(bullcow_guess {Guess, Response});

        EraseGuess(UniqueCombinations, Guess);
        EraseGuess(PossibleSolutions, Guess);
        PruneSet(PossibleSolutions, Guess, Response, BullLookupTable, CowLookupTable);
        OptimalGuesses = Minimax(PossibleSolutions, UniqueCombinations, BullLookupTable, CowLookupTable);
        Guess = GetNextGuess(OptimalGuesses, PossibleSolutions, UniqueCombinations);
    }

    return CHEATING_INDICATOR;
}

void PruneSet(std::vector<int> &Set, const int PreviousGuess, const int Response, const std::vector<int> &BullLookupTable, const std::vector<int> &CowLookupTable)
{
    Set.erase(std::remove_if(Set.begin(), Set.end(), [&](const int TestGuess) {
                  return FastGenerateResponse(TestGuess, PreviousGuess, BullLookupTable, CowLookupTable) != Response;
              }),
              Set.end());

}

void EraseGuess(std::vector<int> &Set, int Guess)
{
    for (auto Iterator = Set.begin(); Iterator != Set.end(); Iterator++) {
        if (*Iterator == Guess) {
            Set.erase(Iterator);
            return;
        }
    }
}

std::vector<int> Minimax(const std::vector<int> &PossibleSolutions, const std::vector<int> &UniqueCombinations, const std::vector<int> &BullLookuptable, const std::vector<int> &CowLookuptable)
{
    if (PossibleSolutions.empty()) {
        return {};  
    }

    if (PossibleSolutions.size() == 1) {
        return std::vector<int> {PossibleSolutions[0]};
    }

    std::vector<int> Result;

    std::vector<guess_score> GuessScores;
    GuessScores.reserve(UniqueCombinations.size());
    std::vector<int> ResponseScores;

    for (size_t GuessIndex = 0; GuessIndex < UniqueCombinations.size(); ++GuessIndex) {
        ResponseScores = BatchGenerateResponseScores(PossibleSolutions, UniqueCombinations[GuessIndex], BullLookuptable, CowLookuptable);
       
        int Max = GetMaxScore(ResponseScores);
        GuessScores.emplace_back(guess_score {UniqueCombinations[GuessIndex], Max});
    }

    int Min = GetMinScore(GuessScores);

    for (const guess_score &Score : GuessScores) {
        if (Score.Value == Min) {
            Result.emplace_back(Score.Guess);
        }
    }

    return Result;
}

int GetMaxScore(const std::vector<int> &GuessScoreCounts)
{
    int Max = 0;
    for (const int ScoreCount : GuessScoreCounts) {
        if (ScoreCount > Max) {
            Max = ScoreCount;
        }
    }

    return Max;
}

int GetMinScore(const std::vector<guess_score> &GuessScores)
{
    int Min = std::numeric_limits<int>::max();
    for (const guess_score &Score : GuessScores) {
        if (Score.Value < Min) {
            Min = Score.Value;
        }
    }

    return Min;
}

int GetNextGuess(const std::vector<int> &OptimalGuesses, const std::vector<int> &PossibleSolutions, const std::vector<int> &UniqueCombinations)
{
    int Result = CHEATING_FLAG;

    for (size_t Index = 0; Index < OptimalGuesses.size(); ++Index) {
        if (std::find(PossibleSolutions.begin(), PossibleSolutions.end(), OptimalGuesses[Index]) != PossibleSolutions.end()) {
            return OptimalGuesses[Index];
        }
    }

    for (size_t Index = 0; Index < OptimalGuesses.size(); ++Index) {
        if (std::find(UniqueCombinations.begin(), UniqueCombinations.end(), OptimalGuesses[Index]) != UniqueCombinations.end()) {
            return OptimalGuesses[Index];
        }
    }
    
    return Result;
}