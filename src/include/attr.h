/*-------------------------------------------------------*/ 
/* attr.h      ( NTHU CS MapleBBS Ver 3.10 )             */ 
/*-------------------------------------------------------*/ 
/* target : dynamic attribute database                   */ 
/* create : 99/03/11                                     */ 
/* update :   /  /                                       */ 
/*-------------------------------------------------------*/ 
 
 
#ifndef _ATTR_H_ 
#define _ATTR_H_ 
                                 
#if 0
  int key; 
  key < 0 is reserved. 
  key & 0xff == 0 is reserved. 
  0x000000?? < key < 0x0000ff?? is reserved by maple.org 
  sizeof(attr): key & 0xff 
 
  file: $userhome/.ATTR      
#endif

// 五子棋、黑白棋、圍棋
#define ATTR_OTHELLO_TOTAL 	0x00001004 
#define ATTR_FIVE_TOTAL 	0x00001104 
#define ATTR_BLOCK_TOTAL 	0x00001204 
#define ATTR_OTHELLO_WIN 	0x00001404 
#define ATTR_FIVE_WIN 		0x00001504      
#define ATTR_BLOCK_WIN 		0x00001604
/* Thor.991019: 生日及性別血型資訊, format: 19990131MA, M/F, O/A/B/C(即AB) */
#define ATTR_BIRTH_MFB		0x0000010c

#endif

