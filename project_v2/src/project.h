#ifndef _PROJECT__H_
#define _PROJECT__H_

// Depicts a certain country and its Olympic medal statistics
struct country
{
    char *name;
    int gold;
    int silver;
    int bronze;
    struct country *next;
};

#endif //! _PROJECT__H_