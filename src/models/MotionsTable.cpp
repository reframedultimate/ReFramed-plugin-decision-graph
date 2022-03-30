#include "decision-graph/models/MotionsTable.hpp"
#include <cstdio>

// ----------------------------------------------------------------------------
static uint64_t hexStringToValue(const char* hex, int* error)
{
    uint64_t value = 0;

    if (hex[0] == '0' && (hex[1] == 'x' || hex[1] == 'X'))
        hex += 2;

    for (; *hex; ++hex)
    {
        value <<= 4;
        if (*hex >= '0' && *hex <= '9')
            value |= *hex - '0';
        else if (*hex >= 'A' && *hex <= 'F')
            value |= *hex - 'A' + 10;
        else if (*hex >= 'a' && *hex <= 'f')
            value |= *hex - 'a' + 10;
        else
        {
            *error = 1;
            return 0;
        }
    }

    return value;
}

// ----------------------------------------------------------------------------
MotionsTable MotionsTable::load()
{
    MotionsTable table;

#if defined(_WIN32)
    FILE* fp = fopen("share\\reframed\\data\\plugin-decision-graph\\ParamLabels.csv", "r");
#else
    FILE* fp = fopen("share/reframed/data/plugin-decision-graph/ParamLabels.csv", "r");
#endif

    char line[128];
    while (fgets(line, sizeof(line), fp))
    {
        // Split string at comma
        char* delim = line;
        for (; *delim != ',' && *delim; ++delim) {}
        if (!*delim)
            continue;
        *delim = '\0';
        char* labelStr = delim + 1;

        // Remove newline and/or carriage return
        for (delim++; *delim != '\r' && *delim != '\n' && *delim; ++delim) {}
        *delim = '\0';

        int error = 0;
        rfcommon::FighterMotion::Type motionValue = hexStringToValue(line, &error);
        if (motionValue == 0)
        {
            if (error)
                fprintf(stderr, "Failed to parse \"%s\" into hex value\n", line);
            else
                fprintf(stderr, "Invalid hex value \"%s\"\n", line);
            continue;
        }

        rfcommon::FighterMotion motion(motionValue);
        rfcommon::SmallString<31> label(labelStr);

        auto motionMapResult = table.motionMap.insertNew(motion, -1);
        if (motionMapResult == table.motionMap.end())
        {
            fprintf(stderr, "Duplicate motion value: %s\n", line);
            continue;
        }

        auto labelMapResult = table.labelMap.insertNew(label, -1);
        if (labelMapResult == table.labelMap.end())
        {
            fprintf(stderr, "Duplicate motion label: %s\n", labelStr);
            continue;
        }

        table.entries.emplace(motion, label);
        motionMapResult->value() = table.entries.count() - 1;
        labelMapResult->value() = table.entries.count() - 1;
    }

    fprintf(stderr, "Loaded %d motion labels\n", table.entries.count());

    return table;
}
