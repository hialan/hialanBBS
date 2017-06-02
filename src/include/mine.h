// 踩地雷遊戲 Ver 1.0
// mine.h 【類別原型宣告】

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TRUE	1
#define	FALSE	0

// 地雷圖的大小
#define X_MAX	30
#define	Y_MAX	15

// 地雷的定義
#define	DEF_MINE		-1	// 地雷代號

// MineStruct 中的 Status 定義
#define	NO_TOUCH		0	// 未踩過
#define TOUCH_OK		1	// 安全的踩過
#define TOUCH_ERR		2	// 踩到地雷
#define	MARK_MINE		3	// 被標示為地雷

struct MineStruct
{
	char	MineOrNo;		// 地雷資料,記錄該位置是地雷還是數字
	char	Status;			// 地雷狀態,記錄遊戲中該位置的狀態
};

// 踩地雷遊戲的類別定義
class MineGame
{
	private:
		MineStruct MineMap[X_MAX][Y_MAX];	// 地雷圖
		int TotalMine;				// 全部地雷數		
		int MarkedMine;				// 已標示的地雷數
		int BuryOneMine(int, int);		// 埋設地雷
		int OpenBlankBlock(int, int);		// 開啟空白區塊
	public:
		MineGame();
		MineGame(int);
		int InitMineMap(int);		// 開啟一個地雷圖
		int MarkMine(int, int);		// 標示地雷
		int UnMarkMine(int, int);	// 拿掉地雷標示
		int OpenMine(int, int);		// 踩地雷
		int ReadMineStatus(int, int);	// 讀取地雷狀態
		int ReadMineData(int, int);	// 讀取地雷資料
		int CountMarkedMine();		// 讀取被標示的地雷數
		int CheckMineMap();		// 檢查地雷圖示是否符合
		int OpenNearMine(int &, int &);	// 快速踩周圍的地雷
		~MineGame();
};

