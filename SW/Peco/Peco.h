#ifndef Peco_h
#define Peco_h

enum eMainStates {
  INIT,
  DRIVE_AND_COLLECT_1,
  DRIVE_AND_COLLECT_DISTANCE_INIT,
  DRIVE_AND_COLLECT_DISTANCE, 
  DRIVE_TO_MIDDLE,
  FIND_TOP,
  DRIVE_TO_TOP,  
  DRIVE_TO_YELLOW,
  UNLOAD_YELLOW,
  DRIVE_TO_GREEN,
  UNLOAD_GREEN,
  END
  };

enum eRichtungen {
  STOPP,
  VORWAERTS,
  RECHTS,
  LINKS,
  RUECKWAERTS
  };
#endif
