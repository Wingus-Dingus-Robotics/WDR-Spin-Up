#ifndef WDR_COMPETITION_H
#define WDR_COMPETITION_H

// Competition mode enum
typedef enum {Disabled, InitAutonomous, Autonomous, InitDriver, Driver} CompetitionMode_t;

// Competition status functions
void wdrUpdateCompStatus();
CompetitionMode_t wdrGetCompStatus();

#endif // WDR_COMPETITION_H
