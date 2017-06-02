// hyper bbs
/*
  suggestions:

  1. use #ifdef HYPER_BBS to wrap the hyperbbs code
  2. 
 */

// please check #ifdef HYPER_BBS

#ifdef HYPER_BBS
#define HB_YN "([200;1432m[300mY[302mes[201m/[200;1431m[300mN[302mo[201m)"
#else
#define HB_YN "(y/n)"
#endif

#ifdef HYPER_BBS
#define HB_SC ";" // SEMICOLON
#define HB_MS "\033[300m"
#define HB_ME0 "\033[301m"
#define HB_ME1 "\033[302m"
#define HB_ME2 "\033[303m"                      
#define HB_E "\033[201m"

#define HB_V_a (600+'a')
#define HB_V_A (600+'A')
#define HB_V_0 (600+'0')
#define HB_V_ENTER (613)

#define HB_V_PGUP (500)
#define HB_V_PGDOWN (501)
#define HB_V_HOME (502)
#define HB_V_END (503)
#define HB_V_UP (504)
#define HB_V_DOWN (505)
#define HB_V_LEFT (506)
#define HB_V_RIGHT (507)
#define HB_V_INSERT (508)
#define HB_V_DELETE (509)

#define HB_S_PGUP "500"
#define HB_S_PGDOWN "501"
#define HB_S_HOME "502"
#define HB_S_END "503"
#define HB_S_UP "504"
#define HB_S_DOWN "505"
#define HB_S_LEFT "506"
#define HB_S_RIGHT "507"
#define HB_S_INSERT "508"
#define HB_S_DELETE "509"
#define HB_S_ENTER "613"
#define HB_S_SPACE "632"
#define HB_S_SLASH "647"
#define HB_S_TAB   "609"
#define HB_S_LBRAKET "691" // [
#define HB_S_RBRAKET "693"// ]

#define HB_CTL_E (5)
#define HB_CTL_P (16)
#define HB_S_CTL_E "605"
#define HB_S_CTL_R "618"
#define HB_S_CTL_X "624"
#define HB_S_CTL_P "616"
#define HB_S_CTL_V "622"
#define HB_S_CTL_Z "626"

#define HB_S_a "697"
#define HB_S_b "698"
#define HB_S_c "699"
#define HB_S_d "700"
#define HB_S_e "701"
#define HB_S_f "702"
#define HB_S_g "703"
#define HB_S_h "704"
#define HB_S_i "705"
#define HB_S_j "706"
#define HB_S_k "707"
#define HB_S_l "708"
#define HB_S_m "709"
#define HB_S_n "710"
#define HB_S_o "711"
#define HB_S_p "712"
#define HB_S_q "713"
#define HB_S_r "714"
#define HB_S_s "715"
#define HB_S_t "716"
#define HB_S_u "717"
#define HB_S_v "718"
#define HB_S_w "719"
#define HB_S_x "720"
#define HB_S_y "721"
#define HB_S_z "722"

#define HB_S_A "665"
#define HB_S_B "666"
#define HB_S_C "667"
#define HB_S_D "668"
#define HB_S_E "669"
#define HB_S_F "670"
#define HB_S_G "671"
#define HB_S_H "672"
#define HB_S_I "673"
#define HB_S_J "674"
#define HB_S_K "675"
#define HB_S_L "676"
#define HB_S_M "677"
#define HB_S_N "678"
#define HB_S_O "679"
#define HB_S_P "680"
#define HB_S_Q "681"
#define HB_S_R "682"
#define HB_S_S "683"
#define HB_S_T "684"
#define HB_S_U "685"
#define HB_S_V "686"
#define HB_S_W "687"
#define HB_S_X "688"
#define HB_S_Y "689"
#define HB_S_Z "690"

#define HB_S_0 "648"
#define HB_S_1 "649"
#define HB_S_2 "650"
#define HB_S_3 "651"
#define HB_S_4 "652"
#define HB_S_5 "653"
#define HB_S_6 "654"
#define HB_S_7 "655"
#define HB_S_8 "656"
#define HB_S_9 "657"

#define HB_BACK "\033[200m\033[444m\033[506m[¡ö]¤W¤@­¶\033[201m"
#endif /* HYPER_BBS */
