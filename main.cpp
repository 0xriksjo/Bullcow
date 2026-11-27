#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <getopt.h>
#include "Utils.h"
#include "AlgoUtils.h"
#include "Knuth.h"
#include "DefaultAlgorithm.h"

const int NUM_GAME_MODES = 3;
const int NUM_HISCORES_TO_PRINT = 15;
const int HISCORE_NAME_MAX = 15;
const char *DEFAULT_HISCORE_FILENAME = "hiscores.txt";
const char *CHEAT_MESSAGE_LOSS = "Ye cheatin' scum!";
const char *CHEAT_MESSAGE_WIN = "Ye cheatin' chum!";
const char *WIN_MESSAGE = "Winner Winner Chickpea Dinner!";

using AlgorithmPointer = bullcow_result (*)(bool ComputerOnly, std::vector<int> UniqueCombinations, std::vector<bullcow_guess> &GuessHistory, int SecretNumberIndex, bool RiggedNumber);

struct algorithm_pointer {
    std::string Name;
    AlgorithmPointer Function;
};

struct hiscore_entry {
    std::string Name;
    bullcow_result Result;
};

struct game_options {
    std::string HiscoresFilename;
    std::string Name;
    algorithm_pointer Algorithm;
    int Mode;
    int RunCount;
};

bullcow_result PlayerIsCodeBreaker(std::vector<int> &UniqueCombinations);
std::vector<hiscore_entry>::iterator GetHiscorePosition(std::vector<hiscore_entry> &Hiscores, int TurnCount);
int GetMaxLength(std::vector<hiscore_entry> &Hiscores);
bool AddResultToHiscore(game_options &GameOptions, bullcow_result &GameResult, std::vector<hiscore_entry> &Hiscores);
bool LoadHiscoreFromFile(game_options &GameOptions, std::vector<hiscore_entry> &Hiscore);
void SaveHiscoreToFile(std::string Filename, std::vector<hiscore_entry> &Hiscores);
void ParseCLIOptions(game_options &Options, std::vector<algorithm_pointer> &Algorithms, int ArgCount, char **Args);
void Play(game_options &GameOptions);
void Options(game_options &GameOptions, const std::vector<algorithm_pointer> &Algorithms);
void PrintHelp();
void PrintHiscore(game_options &GameOptions);

inline void DisplayCommands();
inline void PrintFileOpenError(std::string Filename);
inline std::string GetNameForHiscore();

int main(int ArgCount, char **Args)
{
    srand(time(NULL));

    std::vector<algorithm_pointer> Algorithms;
    Algorithms.emplace_back(algorithm_pointer {"default", PlayDefault});
    Algorithms.emplace_back(algorithm_pointer {"knuth", PlayKnuth});

    game_options GameOptions = {
        "",
        "",
        Algorithms[0],
        0,
        NUM_UNIQUE_GUESSES,
    };

    ParseCLIOptions(GameOptions, Algorithms, ArgCount, Args);

    if (GameOptions.Mode != 0) {
        Play(GameOptions);
        return 0;
    }

    ClearScreen();
    std::cout << "Welcome to Bullcow!\n";


    enum Inputs {
        PLAY = 'p',
        HISCORE = 'h',
        OPTIONS = 'o',
        QUIT = 'q',
    };

    bool Running = true;

    while (Running) {
        DisplayCommands();
        char Selection = GetChar();
        switch (ToLower(Selection)) {
            case QUIT:
            case EOF: {
                Running = false;
            } break;
            case PLAY: {
                Play(GameOptions);
            } break;
            case HISCORE: {
                PrintHiscore(GameOptions);
            } break;
            case OPTIONS: {
                Options(GameOptions, Algorithms);
            } break;
            default: {
                std::cerr << "InputError: expected P/H/O/Q!\n";
            } break;
        }

        if (std::cin.eof()) {
            clearerr(stdin);
            std::cin.clear();
            ClearScreen();
        }
    }

    return 0;
}

void Options(game_options &GameOptions, const std::vector<algorithm_pointer> &Algorithms)
{
    std::cout << "Choose an algorithm for the computer to use\n";

    int AlgorithmIndex = 1;

    for (const auto &Algorithm : Algorithms) {
        std::cout << '[' << AlgorithmIndex << "] " << Algorithm.Name << ((GameOptions.Algorithm.Function == Algorithm.Function) ? " (Active)\n" : "\n");
        AlgorithmIndex++;
    }

    int Selection = GetPlayerInputInRange(1, Algorithms.size());

    if (Selection == EOF) {
        return;
    }

    GameOptions.Algorithm = Algorithms[Selection - 1];
}

void PrintHelp()
{
    ClearScreen();
    std::cout << "usage: ./bullcow [options]\n";
    std::cout << "-m mode to set the active game mode.\n";
    std::cout << "Available game modes: (1) The program is the codebreaker, (2) The player is the codebreaker, (3) The program plays against itself\n";
    std::cout << "-c N to set the number of times the program will play against itself\n";
    std::cout << "-a algorithm to set the active algorithm.\n";
    std::cout << "Available algorithms: \"default\" and \"knuth\". Note: \"knuth\" is very slow; -c <= 100 is recommended!\n";
    std::cout << "-f filename to set the filename used for the hiscore (default is \"hiscore.txt\")\n";
    std::cout << "-n name to set the name used for the hiscore!\n";
}

void Play(game_options &GameOptions)
{
    if (GameOptions.Mode == 0) {
        std::cout << "Choose a game mode\n";
        std::cout << "(1) The program is the code-breaker\n";
        std::cout << "(2) You are the code-breaker\n";
        std::cout << "(3) The program plays against itself\n";
        std::cout << "Active algorithm: " << GameOptions.Algorithm.Name << "!\n";
    }

    int Selection = GameOptions.Mode != 0 ? GameOptions.Mode : GetPlayerInputInRange(1, NUM_GAME_MODES);
    bool ComputerOnly = (Selection == 3);

    bullcow_result GameResult = {0, 0};
    std::vector<bullcow_guess> GuessHistory = {};
    std::vector<int> UniqueCombinations = GenerateAllUniqueCombinations();

    switch (Selection) {
        case EOF: {
            return;
        }
        case 1: {
            GameResult = GameOptions.Algorithm.Function(ComputerOnly, UniqueCombinations, GuessHistory, 0, false);

            if (GameResult == EOF_INDICATOR) {
                return;
            }

            int HasCheated = (GameResult == CHEATING_INDICATOR) ? HasPlayerCheated(GuessHistory) : !VerifyResponses(GameResult.SecretNumber, GuessHistory);

            switch (HasCheated) {
                case EOF: {
                    return;
                }
                case 0: {
                    std::cout << "The " << GameOptions.Algorithm.Name << " algorithm found your secret number " << PadToWidth(GameResult.SecretNumber, SECRET_NUMBER_LENGTH) << " in " << GameResult.TurnCount << " turns!\n";
                } break;
                case 1: {
                    if (GameResult == CHEATING_INDICATOR) {
                        std::cout << CHEAT_MESSAGE_LOSS << '\n';
                    } else {
                        std::cout << CHEAT_MESSAGE_WIN << '\n';
                    }
                } break;
            }
        } break;
        case 2: {
            GameResult = PlayerIsCodeBreaker(UniqueCombinations);

            if (GameResult == EOF_INDICATOR) {
                return;
            }

            std::vector<hiscore_entry> Hiscores = {};
            if (!GameOptions.HiscoresFilename.empty()) {
                LoadHiscoreFromFile(GameOptions, Hiscores);
            }

            std::cout << WIN_MESSAGE << '\n';
            std::cout << "You found the secret number " << PadToWidth(GameResult.SecretNumber, SECRET_NUMBER_LENGTH) << " in " << GameResult.TurnCount << " turns!\n";

            if (!AddResultToHiscore(GameOptions, GameResult, Hiscores)) {
                return;
            }

            SaveHiscoreToFile(!GameOptions.HiscoresFilename.empty() ? GameOptions.HiscoresFilename : DEFAULT_HISCORE_FILENAME, Hiscores);

        } break;
        case 3: {
            std::vector<hiscore_entry> Statistics = {};

            if (GameOptions.Algorithm.Name == "knuth") {
                std::cout << "This will take a while ...\n";
            }

            for (int SecretNumberIndex = 0; SecretNumberIndex < GameOptions.RunCount; ++SecretNumberIndex) {
                GameResult = GameOptions.Algorithm.Function(ComputerOnly, UniqueCombinations, GuessHistory, SecretNumberIndex, true);
                hiscore_entry Statistic = {GameOptions.Algorithm.Name, GameResult};
                Statistics.emplace_back(Statistic);
                GuessHistory.clear();
            }

            int Total = 0;
            double Average;

            for (const hiscore_entry &Statistic : Statistics) {
                Total += Statistic.Result.TurnCount;
            }

            Average = static_cast<double>(Total) / Statistics.size();

            std::cout << "Finished " << GameOptions.RunCount << " run" << (GameOptions.RunCount > 1 ? "s" : "") << " with an average of " << Average << " guesses!\n";
            std::sort(Statistics.begin(), Statistics.end(), [](const hiscore_entry &EntryA, const hiscore_entry &EntryB) {
                return EntryA.Result.TurnCount > EntryB.Result.TurnCount;
            });
            SaveHiscoreToFile(GameOptions.Algorithm.Name, Statistics);
        } break;
        default: {
            return;
        }
    }
}

bullcow_result PlayerIsCodeBreaker(std::vector<int> &UniqueCombinations)
{
    bullcow_result Result = {0, 0};
    (void) UniqueCombinations;
    // int SecretNumber = 1234
    int SecretNumber = GeneratePseudoRandomNumberFromSet(UniqueCombinations);

    bool HasWon = false;
    int Guess;
    int Response;

    while (!HasWon) {
        Result.TurnCount++;
        Guess = GetGuessFromInput();

        if (std::cin.eof()) {
            return EOF_INDICATOR;
        }

        Response = GenerateResponse(Guess, SecretNumber);
        PrintGameInfo(Result.TurnCount, Guess, Response);

        if (Response == RESPONSE_WIN) {
            HasWon = true;
            Result.SecretNumber = Guess;
        }
    }

    return Result;
}

void ParseCLIOptions(game_options &GameOptions, std::vector<algorithm_pointer> &Algorithms, int ArgCount, char **Args)
{
    int Option;
    while ((Option = getopt(ArgCount, Args, "a:f:hm:n:c:")) != -1) {
        switch (Option) {
            case 'a': {
                for (const algorithm_pointer &Algorithm : Algorithms) {
                    if (optarg == Algorithm.Name) {
                        GameOptions.Algorithm = Algorithm;
                    }
                }
                if (GameOptions.Algorithm.Name != optarg) {
                    std::cerr << "InputError: unknown algorithm \"" << optarg << "\" (use -? for a list of available algorithms!\n)";
                    exit(EXIT_FAILURE);
                }
            } break;

            case 'n': {
                int Length = StringLength(optarg);
                if (Length <= HISCORE_NAME_MAX) {
                    GameOptions.Name = optarg;
                } else {
                    std::cerr << "InputError: name must not exceed " << HISCORE_NAME_MAX << " characters!\n";
                    exit(EXIT_FAILURE);
                }
            } break;

            case 'f': {
                int Length = StringLength(optarg);
                if (Length > 0 && Length <= 255)
                    GameOptions.HiscoresFilename = optarg;
            } break;

            case 'h': {
                PrintHiscore(GameOptions);
                exit(EXIT_SUCCESS);
            }

            case 'm': {
                std::string ArgString(optarg);
                if (IsPositiveInteger(ArgString) && ArgString.length() <= NumberOfDigitsInNumber(INT32_MAX)) {
                    // slut
                    int GameMode = std::stoi(ArgString);
                    if (GameMode >= 1 && GameMode <= NUM_GAME_MODES) {
                        GameOptions.Mode = GameMode;
                        break;
                    }
                }
                std::cerr << "InputError: expected a number in the range [1, " << NUM_GAME_MODES << "]!\n";
                exit(EXIT_FAILURE);
            }
            case 'c': {
                std::string ArgString(optarg);
                if (IsPositiveInteger(ArgString) && ArgString.length() <= NumberOfDigitsInNumber(INT32_MAX)) {
                    int RunCount = std::stoi(ArgString);
                    if (RunCount >= 1 && RunCount <= NUM_UNIQUE_GUESSES) {
                        GameOptions.RunCount = RunCount;
                        break;
                    }
                }
                std::cerr << "InputError: expected a number in the range [1, " << NUM_UNIQUE_GUESSES << "]!\n";
                exit(EXIT_FAILURE);
            } break;
            case '?': {
                switch (optopt) {
                    case '?': {
                        PrintHelp();
                        exit(EXIT_SUCCESS);
                    }

                    case 'f': {
                        std::cerr << "InputError: expected -f filename\n";
                        exit(EXIT_FAILURE);
                    }

                    case 'a': {
                        std::cerr << "InputError: expected -a algorithm (use -? for a list of available algorithms!)\n";
                        exit(EXIT_FAILURE);
                    }

                    case 'm': {
                        std::cerr << "InputError: expected -m mode (use -? for a list of available modes!)\n";
                        exit(EXIT_FAILURE);
                    }

                    case 'c': {
                        std::cerr << "InputError: expected -c N (use -? for more information!)\n";
                        exit(EXIT_FAILURE);
                    }

                    default: {
                        std::cerr << "InputError: unknown option -" << static_cast<char>(optopt) << ". Use -? for available options!\n";
                        exit(EXIT_FAILURE);
                    }
                }
            }
            default: {
            } break;
        }
    }
}

std::vector<hiscore_entry>::iterator GetHiscorePosition(std::vector<hiscore_entry> &Hiscores, int TurnCount)
{
    if (Hiscores.empty()) {
        return Hiscores.begin();
    }

    for (std::vector<hiscore_entry>::iterator HiscoreIter = Hiscores.begin(); HiscoreIter != Hiscores.end(); ++HiscoreIter) {
        if (HiscoreIter->Result.TurnCount > TurnCount) {
            return HiscoreIter;
        }
    }

    return Hiscores.end();
}

std::string GetNameForHiscore()
{
    std::string Input;
    bool HasReceivedValidInput = false;

    while (!HasReceivedValidInput) {
        std::cout << "Enter your name: ";
        std::getline(std::cin, Input);

        if (std::cin.eof()) {
            return "";
        }

        if (Input.empty() || Input.size() <= HISCORE_NAME_MAX) {
            HasReceivedValidInput = true;
        } else {
            std::cerr << "InputError: expected a [1, " << HISCORE_NAME_MAX << "] length string!\n";
        }
    }

    return Input;
}

inline void DisplayCommands()
{
    std::cout << "[P]lay\n";
    std::cout << "[H]iscores\n";
    std::cout << "[O]ptions\n";
    std::cout << "[Q]uit\n";
    std::cout << "Entering [Ctrl + D] will send you back to the main menu wherever input is expected (acts as QUIT on the main menu!)\n";
}

void PrintHiscore(game_options &GameOptions)
{
    std::vector<hiscore_entry> Hiscores;

    if (!LoadHiscoreFromFile(GameOptions, Hiscores)) {
		std::cerr << "FileError: no hiscores file to to load from\n";
        return;
    }

    if (Hiscores.empty()) {
        std::cerr << "FileError: the hiscore is empty\n";
        return;
    }

    if (Hiscores.size() > 0) {
        int MaxLength = GetMaxLength(Hiscores);

        std::cout << "RANK" << "  " << std::setw(MaxLength) << std::left << "NAME" << " " << "CODE" << "  " << '#' << '\n';
        for (size_t HiscoreIndex = 0; HiscoreIndex < Hiscores.size() && HiscoreIndex < NUM_HISCORES_TO_PRINT; ++HiscoreIndex) {
            std::cout << HiscoreIndex + 1 << "    " << std::setw(MaxLength) << std::left << Hiscores[HiscoreIndex].Name << "  " << Hiscores[HiscoreIndex].Result.SecretNumber << "  " << Hiscores[HiscoreIndex].Result.TurnCount << '\n';
        }
    }
}

int GetMaxLength(std::vector<hiscore_entry> &Hiscores)
{
    int MaxLength = Hiscores[0].Name.length();
    for (size_t HiscoreIndex = 1; HiscoreIndex < Hiscores.size() && HiscoreIndex < NUM_HISCORES_TO_PRINT; ++HiscoreIndex) {
        int Length = Hiscores[HiscoreIndex].Name.length();
        if (Length > MaxLength) {
            MaxLength = Length;
        }
    }
    return MaxLength;
}

bool AddResultToHiscore(game_options &GameOptions, bullcow_result &GameResult, std::vector<hiscore_entry> &Hiscores)
{
    if (Hiscores.empty()) {
        LoadHiscoreFromFile(GameOptions, Hiscores);
    }

    if (GameOptions.Name.empty()) {
        GameOptions.Name = GetNameForHiscore();
        if (GameOptions.Name == "EOF") {
            GameOptions.Name = "";
            return false;
        }
    }

    hiscore_entry HiscoreEntry = {GameOptions.Name, GameResult};
    GameOptions.Name = "";
    std::vector<hiscore_entry>::iterator HiscoreIter = GetHiscorePosition(Hiscores, GameResult.TurnCount);

    std::cout << "Your position on the hiscore is " << std::distance(Hiscores.begin(), HiscoreIter) + 1 << "!\n";

    Hiscores.insert(HiscoreIter, HiscoreEntry);

    return true;
}

void SaveHiscoreToFile(std::string Filename, std::vector<hiscore_entry> &Hiscores)
{
    std::ofstream HiscoreHandle(Filename);

    if (!IsOpenFile(Filename, HiscoreHandle)) {
        return;
    }

    int MaxLength = GetMaxLength(Hiscores);
    int Width = SECRET_NUMBER_LENGTH + MaxLength;
    for (const hiscore_entry &Score : Hiscores) {
        HiscoreHandle << std::setw(Width) << std::left << Score.Name << "  " << PadToWidth(Score.Result.SecretNumber, 4) << "  " << Score.Result.TurnCount << '\n';
    }
}

bool LoadHiscoreFromFile(game_options &GameOptions, std::vector<hiscore_entry> &Hiscores)
{
    if (GameOptions.HiscoresFilename.empty()) {
        GameOptions.HiscoresFilename = GameOptions.HiscoresFilename.empty() ? DEFAULT_HISCORE_FILENAME : GameOptions.HiscoresFilename;
    }

    std::ifstream HiscoreHandle(GameOptions.HiscoresFilename);

    if (!HiscoreHandle) {
        return false;
    }

    hiscore_entry Score;

    while (HiscoreHandle >> Score.Name >> Score.Result.SecretNumber >> Score.Result.TurnCount) {
        Hiscores.emplace_back(Score);
    }

    return true;
}
