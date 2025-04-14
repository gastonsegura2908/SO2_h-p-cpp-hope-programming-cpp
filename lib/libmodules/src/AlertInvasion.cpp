/**
 * @file AlertInvasion.cpp
 * @brief temperature alert module
 */

#include "AlertInvasion.h"

std::string TempAlert()
{
    std::array<std::string, ENTRY_SIZE> entries = {"north_entry", "east_entry", "west_entry", "south_entry"};

    srand(time(NULL));

    float Temperature = ((float)rand() / RAND_MAX) * (MAXTEMP - MINTEMP) + MINTEMP;
    int EntryIndex = rand() % ENTRY_SIZE;
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char TimeStamp[SIZETS];
    if (strftime(TimeStamp, sizeof(TimeStamp), "%c", &tm) == 0)
    {
        fprintf(stderr, "strftime returned 0");
        exit(EXIT_FAILURE);
    }

    std::ostringstream FullAlert;
    FullAlert << TimeStamp << ", " << entries[EntryIndex] << ", " << std::fixed << std::setprecision(PRECISION)
              << Temperature;

    if (Temperature >= TEMP)
    {
        std::ostringstream AlertMessage;
        AlertMessage << "Alert of possible infection in " << entries[EntryIndex] << " , " << std::fixed
                     << std::setprecision(PRECISION) << Temperature << "\n";
        return AlertMessage.str();
    }

    return FullAlert.str();
}

