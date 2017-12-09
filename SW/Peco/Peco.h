#ifndef Peco_h
#define Peco_h

enum pecoStates {
  STARTE_BUERSTEN,
  FAHRE_KREIS_1,
  FAHRE_KREIS_2,
  SUCHE_TOEGGELI,
  DREHE_ZU_TOEGGELI, 
  FAHRE_ZU_TOEGGELI,
  ALLES_STOP
  };


enum SammelfahrtStates{
  AchtEck_gerade_1,
  AchtEck_drehen_1_2,
  AchtEck_gerade_2,
  AchtEck_drehen_2_3,  
  AchtEck_gerade_3,
  AchtEck_drehen_3_4  
  };
#endif
