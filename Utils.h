#ifndef UTILS_H
#define UTILS_H

#include <iostream>

constexpr int REASONABLE_FILENAME_MAX = 255;

std::string GetFilename();
bool IsValidFilename(const char *Filename);
char GetChar();

inline size_t NumberOfDigitsInNumber(int Number)
{
    size_t Count = 0;
    while (Number > 0) {
        ++Count;
        Number /= 10;
    }

    return Count;
}

inline int StringLength(const char *String)
{
    const char *CharIndex = String;
    int Count = 0;

    while (*CharIndex++) {
        ++Count;
    }

    return Count;
}

inline char ToLower(char Char)
{
    if (Char >= 'A' && Char <= 'Z') {
        return Char | 32;
    }

    return Char;
}

inline void ClearScreen()
{
    std::cout << std::string(100, '\n');
}

inline void PrintFileOpenError(std::string Filename)
{
    std::cerr << "FileError: unable to open " << Filename << "!\n";
}

template <typename FileStream>
inline bool IsOpenFile(std::string Filename, FileStream &Handle)
{
    if (!Handle) {
        PrintFileOpenError(Filename);
        return false;
    }

    return true;
}


#endif