#ifndef Peco_h
#define Peco_h

enum eMainStates {
  INIT,
  DRIVE_AND_COLLECT_1,
  DRIVE_AND_COLLECT_2,
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
