/*-------------------------------------------------------*/
/* menu.h       ( NTHU CS MapleBBS Ver 2.36 )            */
/*-------------------------------------------------------*/
/* target : menu                                         */
/* create : 95/03/29                                     */
/* update : 95/12/15                                     */
/*-------------------------------------------------------*/

// wildcat : 分格線用的空選單 :p
int
null_menu()
{
  pressanykey("這是一個空選單 :p ");
  return 0;
}

/* ----------------------------------------------------- */
/* Menu Commands struct                                  */
/* ----------------------------------------------------- */
#if 0
  hialan: MENU 結構大致上使用 desc 而 key 則無用
          在 desc 中可以指定執行完這個指令後跳到下一個選項
          以下是 desc 的使用法:

          /* next/key/description*/
          
          例如:
           "RNew           [閱\讀新信]",
           "RRead          [信件列表]",
           
         則按 n 會跳到閱讀新信, 按 r 會跳到信件列表
         當執行完閱讀新信後, 游標會自動跳到信件列表
#endif

void domenu();
/* ----------------------------------------------------- */
/* The set user menu for administrator                   */
/* ----------------------------------------------------- */


static struct one_key adminuser[] = {
'1',  null_menu,	   	PERM_ADMIN,	"11\033[1;31m注意:確實審核註冊單!!\033[m",0,
'2',  null_menu,    	PERM_ADMIN,	"22\033[1;31m注意:確實紀錄修改原因\033[m",0,
'u',  "SO/admin.so:m_user",	PERM_ACCOUNTS,	"UUser          [使用者資料]",1,
'm',  "SO/register.so:m_newuser",
                        PERM_ACCOUNTS,	"MMake User     [新增使用者]",1,
'f',  "SO/admin.so:search_key_user",
                	PERM_ACCOUNTS,	"FFind User     [搜尋使用者]",1,  
'r',  "SO/admin.so:m_register",   	
                        PERM_ACCOUNTS,	"RRegister      [審核註冊單]",1,
'\0', NULL, 0, NULL, 0};

int
m_user_admin()
{
  domenu(ADMIN, "設定使用者", 'U',adminuser);
  return 0;
}

/* ----------------------------------------------------- */
/* administrator's maintain menu                         */
/* ----------------------------------------------------- */

int XFile();

#ifdef  HAVE_MAILCLEAN
int m_mclean();
#endif

int hialan_test()
{
  win_formchange();
}

static struct one_key adminlist[] = {
'H',  hialan_test, 0, "HHialan test", 0,
'U',  m_user_admin, PERM_ACCOUNTS,	"UUser Menu     [使用者設定]",0,
'N',  "SO/admin.so:m_newbrd",
                PERM_BOARD,     "NNew Board     [開啟新看板]",1,
'S',  "SO/admin.so:m_board",
                PERM_BOARD,     "SSet Board     [ 設定看板 ]",1,
'X',  XFile,        PERM_SYSOP,     "XXfile         [修改系統檔]",0,
'C',  "SO/admin.so:reload_cache",
                PERM_SYSOP,     "CCache Reload  [ 更新狀態 ]",1,
'G',  "SO/admin.so:adm_givegold", 
                PERM_SYSOP,     "GGive $$       [ 發放獎金 ]",1,

#ifdef HAVE_MAILCLEAN
//'M',  "SO/admin.so:m_mclean, PERM_BBSADM, "MMail Clean    清理使用者個人信箱",1,
#endif
'M',  "SO/admin.so:x_reg",
                PERM_ACCOUNTS,  "MMerge         [審核修理機]",1,
'\0', NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* class menu                                            */
/* ----------------------------------------------------- */

int board(), local_board(), Boards(), ReadSelect() ,
    New(),Favor(),favor_edit(),good_board(),voteboard();

static struct one_key classlist[] = {
'V',  voteboard, PERM_BASIC,"VVoteBoard    [看板連署系統]",0,
'C',      board, 0,         "CClass        [本站分類看板]",0,
'N',        New, 0,         "NNew          [所有看板列表]",0,
'L',local_board, 0,         "LLocal        [站內看板列表]",0,
'G', good_board, 0,         "GGoodBoard    [  優良看板  ]",0,
'B',      Favor, PERM_BASIC,"BBoardFavor   [我的最愛看板]",0,
'F', favor_edit, PERM_BASIC,"FFavorEdit    [編輯我的最愛]",0,
'S', ReadSelect, 0,         "SSelect       [  選擇看板  ]",0,
0,NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* Ptt money menu                                        */
/* ----------------------------------------------------- */

static struct one_key finlist[] = {
'0',  "SO/buy.so:bank",     0,      "00Bank           \033[1;36m神殿銀行\033[m",1,
'1',  "SO/song.so:ordersong",0,     "11OrderSong      \033[1;33m神殿點歌機\033[m     $5g/首",1,
'2',  "SO/buy.so:p_cloak",  0,      "22Cloak          臨時隱身/現身  $2g/次     (現身免費)",1,
'3',  "SO/buy.so:p_from",   0,      "33From           修改故鄉       $5g/次",1,
'4',  "SO/buy.so:p_exmail", 0,      "44BuyMailbox     購買信箱上限   $100g/封",1,
'5',  "SO/buy.so:p_spmail", 0,	"55VenMailbox     賣出信箱上限   $80g/封",1,
'6',  "SO/buy.so:p_fcloak", 0,      "66UltimaCloak    終極隱身大法   $500g      可永久隱形",1,
'7',  "SO/buy.so:p_ffrom",  0,      "77PlaceBook      故鄉修改寶典   $1000g     User名單按F可改故鄉",1,
'8',  "SO/buy.so:p_ulmail", 0,      "88NoLimit        信箱無上限     $100000g   信箱上限無限制",1,
0, NULL, 0, NULL,0};

int
finance()
{
  domenu(FINANCE, "金融中心", '0', finlist);
  return 0;
}

/* ----------------------------------------------------- */
/* Talk menu                                             */
/* ----------------------------------------------------- */

int t_users(), t_idle(), t_query(), t_pager();
// t_talk();
/* Thor: for ask last call-in message */
int t_display();
int t_bmw();

static struct one_key talklist[] = {

'L',  t_users,      0,              "LList          [線上名單]",0,
'P',  t_pager,      PERM_BASIC,     "PPager         [切換狀態]",0,
'I',  t_idle,       0,              "IIdle          [鎖定螢幕]",0,
'Q',  t_query,      0,              "QQueryUser     [查詢User]",0,
/*
'T',  t_talk,       PERM_PAGE,      "TTalk          [找人聊天]",0,
 */
'C',  "SO/chat.so:t_chat",PERM_CHAT,"CChatRoom      [連線聊天]",1,
'D',  t_display,    0,              "DDisplay       [水球回顧]",0,
'W',  t_bmw,        PERM_PAGE,      "WWrite         [水球回顧]", 0,
'X',  XFile,        PERM_XFILE,     "XXfile         [修改系統檔]",0,
0, NULL, 0, NULL,0};

/*-------------------------------------------------------*/
/* powerbook menu                                        */
/* ----------------------------------------------------- */

int my_gem();

static struct one_key powerlist[] = {
'G',  my_gem,              0,       "GGem           [我的精華]",0,
'-',  null_menu,           0,	"------ 答錄機 功\能 ------",0,
'P',  "SO/pnote.so:p_read",0,       "PPhone Answer  [聽取留言]",1,
'C',  "SO/pnote.so:p_call",0,       "CCall phone    [送出留言]",1,
'R',  "SO/pnote.so:p_edithint",0,   "RRecord        [錄歡迎詞]",1,

0, NULL, 0, NULL,0};

int
PowerBook()
{
  domenu(POWERBOOK, "萬用手冊", 'G', powerlist);
  return 0;
}


/* ----------------------------------------------------- */
/* User menu                                             */
/* ----------------------------------------------------- */

extern int u_editfile();
int u_info(), u_cloak(), u_list(), u_habit(),
    PowerBook(), ListMain(), change_lightbar();

#ifdef REG_MAGICKEY
int u_verify();
#endif

static struct one_key userlist[] = {
'P',  PowerBook,    PERM_BASIC,	 "PPowerBook     [萬用手冊]",0,
'I',  u_info,       PERM_BASIC,  "IInfo          [修改資料]",0,
'H',  u_habit,      0, 	         "HHabit         [喜好設定]",0,
'L',  ListMain,     PERM_LOGINOK,"LList          [設定名單]",0, 
'F',  u_editfile,   PERM_LOGINOK,"FFileEdit      [個人檔案]",0,
'C',  u_cloak,      PERM_CLOAK,  "CCloak         [隱形密法]",0,

#ifdef REG_FORM
'R', "SO/register.so:u_register",PERM_POST,"RRegister      [填註冊單]",1,
#endif

#ifdef REG_MAGICKEY
'V',  u_verify, 	PERM_BASIC,	"VVerify        [填註冊碼]",0,
#endif

'U',  u_list,       PERM_BASIC,     "UUsers         [註冊名單]",0,
  
'B', change_lightbar,         0,	"BBar Change    [光棒顏色]",0,
0, NULL, 0, NULL,0};


/* ----------------------------------------------------- */
/* service menu                                          */
/* ----------------------------------------------------- */
int note();

static struct one_key servicelist[] = {
'F',  finance,      PERM_LOGINOK,   "FFinance       [商品大街]",0,
  
'V',  "SO/vote.so:all_vote",
                PERM_LOGINOK,   "VVote          [投票中心]",1,
'N',  note,         PERM_LOGINOK,   "NNote          [寫留言板]",0,
0,  NULL, 0, NULL,0};

/* ----------------------------------------------------- */
/* mail menu                                             */
/* ----------------------------------------------------- */
int m_new(), m_read(), m_send(),m_sysop(),mail_mbox(),mail_all(),
    setforward(),mail_list();

#ifdef INTERNET_PRIVATE_EMAIL
int m_internet();
#endif

static struct one_key maillist[] = {
'N',  m_new,        PERM_READMAIL,  "RNew           [閱\讀新信]",0,
'R',  m_read,       PERM_READMAIL,  "RRead          [信件列表]",0,
'S',  m_send,       PERM_BASIC,     "SSend          [站內寄信]",0,
'M',  mail_list,    PERM_BASIC,     "MMailist       [群組寄信]",0,
'I',  m_internet,   PERM_INTERNET,  "IInternet      [網路郵件]",0,
'F',  setforward,   PERM_LOGINOK,   "FForward       [收信轉寄]",0,
'O',  m_sysop,      0,              "OOp Mail       [諂媚站長]",0,
'Z',  mail_mbox,    PERM_INTERNET,  "ZZip           [打包資料]",0,
'A',  mail_all,     PERM_SYSOP,     "AAll           [系統通告]",0,
0, NULL, 0, NULL,0};



/* ----------------------------------------------------- */
/* main menu                                             */
/* ----------------------------------------------------- */

static int
admin()
{
  domenu(ADMIN, "站長老大", 'X', adminlist);
  return 0;
}

static int
BOARD()
{
  domenu(CLASS, "看板列表", 'G', classlist);
  return 0;
}

static int
Mail()
{
  domenu(MAIL, "郵件選單", 'R', maillist);
  return 0;
}

int
static Talk()
{
  domenu(TMENU, "聊天選單", 'L', talklist);
  return 0;
}

static int
User()
{
  domenu(UMENU, "個人設定", 'H', userlist);
  return 0;
}


static int
Service()
{
  domenu(PMENU, "各種服務", 'F', servicelist);
  return 0;
}


int Announce(), Boards(), Goodbye(), board(), Favor();


struct one_key cmdlist[] = {
'A',  Announce,     0,              "AAnnounce      [天地精華]",0,
'B',  BOARD,        0,              "BBoard         [看板功\能]",0,
'C',  board,        0,              "CClass         [分類看板]",0,
'F',  Favor,        PERM_BASIC,     "FFavor         [我的最愛]",0,
'M',  Mail,         PERM_BASIC,     "MMail          [福音快遞]",0,
'T',  Talk,         0,              "TTalk          [談天說地]",0,
'U',  User,         0,              "UUser          [個人工具]",0,
'S',  Service,      PERM_BASIC,     "SService       [各種服務]",0,
'0',  admin,        PERM_ADMIN,     "00Admin        [系統管理]",0,
'G',  Goodbye,      0,              "GGoodbye       [有緣千里]",0,
0, NULL, 0, NULL,0};
/* INDENT ON */
