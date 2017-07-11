#pragma once
//#define GAME_panda
//#define GAME_paiYouQuan
//#define GAME_365 
#define GAME_MJ_NANJING
//#define GAME_MJ_CHAOYANG
//#define GAME_MJ_HONGHE
//#define GAME_MJ_YANGZHOU
//#define GAME_IN_REVIEW 

#if defined( GAME_panda)

#define Gotype_DevID "73d1ec14-84c9-46de-b6d7-fd9abbb4eb6d"
#define Gotype_ProdID "73d1ec14-84c9-46de-b6d7-fd9abbb4eb6d"
#define Wechat_notifyUrl ""
#define Wechat_MchID ""
#define Wechat_appID ""
#define Wechat_MchKey ""
#define Game_DB_Name "pandaniuniu"
#define Log_DB_Name "pandaniuniulog"

#elif defined(GAME_paiYouQuan)

#define Gotype_DevID "e87f31bb-e86c-4d87-a3f3-57b3da76b3d6"
#define Gotype_ProdID "abffee4b-deea-4e96-ac8d-b9d58f246c3f" 
#define Wechat_notifyUrl "http://abc.paiyouquan.com:5007/vxpay.php"
#define Wechat_MchID "1308480601"
#define Wechat_appID "wx66f2837c43330a7b"
#define Wechat_MchKey "E97ED2537D229C0E967042D2E7F1C936"
#define Game_DB_Name "taxpokerdb"
#define Log_DB_Name "taxpokerlogdb"

#elif defined(GAME_365)

#define Gotype_DevID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
#define Gotype_ProdID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
//#define Wechat_notifyUrl "http://abc.paiyouquan.com:5016/vxpay.php"  // online editon 
#define Wechat_notifyUrl "http://abc.paiyouquan.com:7006/vxpay.php"  // mj test online edtion 
//#define Wechat_MchID "1385365702"
//#define Wechat_appID "wxae3a38cb9960bc84"
//#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENU"
#define Wechat_MchID "1401077402"
#define Wechat_appID "wxdfc2ca3aac7aedfa"
#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENA"
#define Game_DB_Name "niuniu365"
#define Log_DB_Name "niuniulog"

#elif defined(GAME_MJ_NANJING)

#define Gotype_DevID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
#define Gotype_ProdID "4f466e4d-ece6-4139-986a-2ad79e3ab74f"
//#define Wechat_notifyUrl "http://abc.paiyouquan.com:5016/vxpay.php"  // online editon 
#define Wechat_notifyUrl "http://njmj.paiyouquan.com:7006/vxpay.php"  // mj test online edtion 
//#define Wechat_MchID "1385365702"
//#define Wechat_appID "wxae3a38cb9960bc84"
//#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENU"
#define Wechat_MchID "1401856702"
#define Wechat_appID "wxdaff45ff145c0350"
#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENA"
#define Game_DB_Name "niuniu365"
#define Log_DB_Name "niuniulog"

#elif defined(GAME_MJ_CHAOYANG)

#define Gotype_DevID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
#define Gotype_ProdID "4f466e4d-ece6-4139-986a-2ad79e3ab74f"
//#define Wechat_notifyUrl "http://abc.paiyouquan.com:5016/vxpay.php"  // online editon 
#define Wechat_notifyUrl "http://cymj.paiyouquan.com:7006/vxpay.php"  // mj test online edtion 
//#define Wechat_MchID "1385365702"
//#define Wechat_appID "wxae3a38cb9960bc84"
//#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENU"
#define Wechat_MchID "1401856702"
#define Wechat_appID "wxdaff45ff145c0350"
#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENA"
#define Game_DB_Name "cymj"
#define Log_DB_Name "niuniulog"

#elif defined(GAME_MJ_HONGHE)

#define Gotype_DevID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
#define Gotype_ProdID "4f466e4d-ece6-4139-986a-2ad79e3ab74f"
//#define Wechat_notifyUrl "http://abc.paiyouquan.com:5016/vxpay.php"  // online editon 
#define Wechat_notifyUrl "http://hhmj.paiyouquan.com:7006/vxpay.php"  // mj test online edtion 
//#define Wechat_MchID "1385365702"
//#define Wechat_appID "wxae3a38cb9960bc84"
//#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENU"
#define Wechat_MchID "1401856702"
#define Wechat_appID "wxdaff45ff145c0350"
#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENA"
#define Game_DB_Name "hhmj"
#define Log_DB_Name "niuniulog"

#elif defined(GAME_MJ_YANGZHOU)

#define Gotype_DevID "8276e6d3-5e96-44e2-9243-97a40f019bb3"
#define Gotype_ProdID "4f466e4d-ece6-4139-986a-2ad79e3ab74f"
//#define Wechat_notifyUrl "http://abc.paiyouquan.com:5016/vxpay.php"  // online editon 
#define Wechat_notifyUrl "http://yzmj.paiyouquan.com:7007/vxpay.php"  // mj test online edtion 
//#define Wechat_MchID "1385365702"
//#define Wechat_appID "wxae3a38cb9960bc84"
//#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENU"
#define Wechat_MchID "1401856702"
#define Wechat_appID "wxdaff45ff145c0350"
#define Wechat_MchKey "NUN5DKS5MJW4UBVJIL1G2XUQ66LU2ENA"
#define Game_DB_Name "yzmj"
#define Log_DB_Name "niuniulog"

#else

#define Gotype_DevID ""
#define Gotype_ProdID ""
#define Wechat_notifyUrl "http://abc.paiyouquan.com:5006/vxpay.php"
#define Wechat_MchID ""
#define Wechat_appID ""
#define Wechat_MchKey ""
#define Game_DB_Name ""
#define Log_DB_Name ""

#endif 