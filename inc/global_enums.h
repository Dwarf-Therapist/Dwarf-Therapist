#ifndef GLOBAL_ENUMS_H
#define GLOBAL_ENUMS_H

typedef enum {
    triple_negative=-3,
    double_negative=-2,
    negative=-1,
    average=0,
    positive=1,
    double_positive=2,
    triple_positive=3
} ASPECT_TYPE;

typedef enum{
    none=-1, //custom
    semi_wild=0,
    trained=1,
    well_trained=2,
    skillfully_trained=3,
    expertly_trained=4,
    exceptionally_trained=5,
    masterfully_trained=6,
    domesticated=7,
    unknown_trained=8,
    wild_untamed=9,
    hostile=10 //custom
} ANIMAL_TYPE;


#endif // GLOBAL_ENUMS_H
