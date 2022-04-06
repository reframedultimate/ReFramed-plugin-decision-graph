#include "decision-graph/models/UserLabelsModel.hpp"
#include "decision-graph/listeners/UserLabelsListener.hpp"
#include <cstdio>
#include <memory>

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
bool UserLabelsModel::loadMotionLabels(const char* fileName)
{
    FILE* fp = fopen(fileName, "r");
    if (fp == nullptr)
        return false;

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

        auto motionMapResult = motionLabels.motionMap.insertNew(motion, -1);
        if (motionMapResult == motionLabels.motionMap.end())
        {
            fprintf(stderr, "Duplicate motion value: %s\n", line);
            continue;
        }

        auto labelMapResult = motionLabels.labelMap.insertNew(label, -1);
        if (labelMapResult == motionLabels.labelMap.end())
        {
            fprintf(stderr, "Duplicate motion label: %s\n", labelStr);
            continue;
        }

        motionLabels.entries.emplace(motion, label);
        motionMapResult->value() = motionLabels.entries.count() - 1;
        labelMapResult->value() = motionLabels.entries.count() - 1;
    }
    fclose(fp);

    fprintf(stderr, "Loaded %d motion labels\n", motionLabels.entries.count());

    /*
    insertUser("nair", "attack_air_n");
    insertUser("nair", "landing_air_n");
    insertUser("uair", "attack_air_hi");
    insertUser("uair", "landing_air_hi");
    insertUser("bair", "attack_air_b");
    insertUser("bair", "landing_air_b");
    insertUser("dair", "attack_air_lw");
    insertUser("dair", "landing_air_lw");
    insertUser("dair", "attack_air_lw_hit");
    insertUser("fair", "attack_air_f");
    insertUser("fair", "landing_air_f");
    insertUser("grab", "catch");
    insertUser("utilt", "attack_hi3");
    insertUser("dtilt", "attack_lw3");
    insertUser("shield", "guard_on");
    insertUser("dash", "dash");
    insertUser("dash", "turn_dash");
    insertUser("walk", "walk_slow");
    insertUser("walk", "walk_middle");
    insertUser("usmash", "attack_hi4");
    insertUser("usmash", "attack_hi4_hold");
    insertUser("turnaround", "turn");
    insertUser("dthrow", "throw_lw");

    insertUser("qa1", "special_air_hi_start");
    insertUser("qa1", "special_air_hi1");
    insertUser("qa1", "special_air_hi_end");
    insertUser("qa2", "special_air_hi2");

    insertUser("qa", "special_air_hi_start");
    insertUser("qa", "special_air_hi1");
    insertUser("qa", "special_air_hi_end");
    insertUser("qa", "special_air_hi2");

    insertUser("thunder", "special_air_lw");
    insertUser("thunder", "special_air_lw_hit");
    insertUser("bluethunder", "special_air_lw_hit");*/

    return true;
}

// ----------------------------------------------------------------------------
const char* UserLabelsModel::motionToLabel(rfcommon::FighterMotion motion) const
{
    auto it = motionLabels.motionMap.find(motion);
    if (it == motionLabels.motionMap.end())
        return nullptr;
    const auto& s = motionLabels.entries[it->value()].label;
    return s.count() ? s.cStr() : nullptr;
}

// ----------------------------------------------------------------------------
const char* UserLabelsModel::motionToUserLabel(rfcommon::FighterMotion motion, rfcommon::FighterID fighterID) const
{
    if (fighterID.value() >= fighterUserLabels.count())
        return nullptr;

    const FighterUserLabels& fighter = fighterUserLabels[fighterID.value()];
    auto it = fighter.motionMap.find(motion);
    if (it == fighter.motionMap.end())
        return nullptr;
    const auto& s = fighter.entries[it->value()].userLabel;
    return s.count() ? s.cStr() : nullptr;
}

// ----------------------------------------------------------------------------
rfcommon::SmallVector<rfcommon::FighterMotion, 4> UserLabelsModel::userLabelToMotion(const char* userLabel, rfcommon::FighterID fighterID) const
{
    rfcommon::SmallVector<rfcommon::FighterMotion, 4> result;

    if (fighterID.value() >= fighterUserLabels.count())
        return result;

    const FighterUserLabels& fighter = fighterUserLabels[fighterID.value()];
    auto it = fighter.userMap.find(userLabel);
    if (it == fighter.userMap.end())
        return {};

    for (int i : it->value())
        result.push(fighter.entries[i].motion);
    return result;
}

// ----------------------------------------------------------------------------
rfcommon::FighterMotion UserLabelsModel::labelToMotion(const char* label) const
{
    auto it = motionLabels.labelMap.find(label);
    if (it == motionLabels.labelMap.end())
        return 0;
    return motionLabels.entries[it->value()].motion;
}
