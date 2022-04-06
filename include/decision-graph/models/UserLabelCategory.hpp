#pragma once

#define USER_LABEL_CATEGORIES_LIST \
    X(MOVEMENT, "Movement") \
    X(GROUND_ATTACKS, "Ground Attacks") \
    X(AERIAL_ATTACKS, "Aerial Attacks") \
    X(SPECIAL_ATTACKS, "Special Attacks") \
    X(GRABS, "Grabs") \
    X(LEDGE, "Ledge") \
    X(DEFENSIVE, "Defensive") \
    X(MISC, "Misc") \
    X(UNLABELED, "Unlabeled")


enum class UserLabelCategory {
#define X(name, desc) name,
    USER_LABEL_CATEGORIES_LIST
#undef X
    COUNT
};
