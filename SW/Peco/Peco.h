#ifndef Peco_h
#define Peco_h

enum eMainStates {
  INIT,
  DRIVE_AND_COLLECT,
  UNLOAD_YELLOW,
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
