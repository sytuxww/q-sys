#include "PinYinForKeyBoardPage.h"
//����ʵ��˼·��������� + ���ݷ�ƥ�� 
//���ƺ�������
const unsigned char PY_mb_a     []={"����"};
const unsigned char PY_mb_ai    []={"��������������������������"};
const unsigned char PY_mb_an    []={"������������������"};
const unsigned char PY_mb_ang   []={"������"};
const unsigned char PY_mb_ao    []={"�������������°İ�"};
const unsigned char PY_mb_ba    []={"�˰ͰȰǰɰṴ̋ưʰΰϰѰаӰְհ�"};
const unsigned char PY_mb_bai   []={"�װٰ۰ذڰܰݰ�"};
const unsigned char PY_mb_ban   []={"�����߰����������"};
const unsigned char PY_mb_bang  []={"������������������"};
const unsigned char PY_mb_bao   []={"������������������������������������"};
const unsigned char PY_mb_bei   []={"������������������������������"};
const unsigned char PY_mb_ben   []={"����������"};
const unsigned char PY_mb_beng  []={"�����±ñű�"};
const unsigned char PY_mb_bi    []={"�ƱǱȱ˱ʱɱұرϱձӱѱݱбֱԱͱױ̱αڱܱ�"};
const unsigned char PY_mb_bian  []={"�߱�ޱ���������"};
const unsigned char PY_mb_biao  []={"�����"};
const unsigned char PY_mb_bie   []={"�����"};
const unsigned char PY_mb_bin   []={"����������"};
const unsigned char PY_mb_bing  []={"������������������"};
const unsigned char PY_mb_bo    []={"����������������������������������������"};
const unsigned char PY_mb_bu    []={"��������������������"};
const unsigned char PY_mb_ca    []={"��"};
const unsigned char PY_mb_cai   []={"�²ŲĲƲòɲʲǲȲ˲�"};
const unsigned char PY_mb_can   []={"�βͲвϲѲҲ�"};
const unsigned char PY_mb_cang  []={"�ֲײԲղ�"};
const unsigned char PY_mb_cao   []={"�ٲڲܲ۲�"};
const unsigned char PY_mb_ce    []={"���޲��"};
const unsigned char PY_mb_ceng  []={"�����"};
const unsigned char PY_mb_cha   []={"������������ɲ"};
const unsigned char PY_mb_chai  []={"����"};
const unsigned char PY_mb_chan  []={"�������������������"};
const unsigned char PY_mb_chang []={"������������������������"};
const unsigned char PY_mb_chao  []={"��������������������"};
const unsigned char PY_mb_che   []={"������������"};
const unsigned char PY_mb_chen  []={"�������������³��ĳ�"};
const unsigned char PY_mb_cheng []={"�Ƴųɳʳгϳǳ˳ͳ̳γȳѳҳ�"};
const unsigned char PY_mb_chi   []={"�Գճڳس۳ٳֳ߳޳ݳܳ����"};
const unsigned char PY_mb_chong []={"������"};
const unsigned char PY_mb_chou  []={"�������������"};
const unsigned char PY_mb_chu   []={"����������������������������������"};
const unsigned char PY_mb_chuai []={"��"};
const unsigned char PY_mb_chuan []={"��������������"};
const unsigned char PY_mb_chuang[]={"����������"};
const unsigned char PY_mb_chui  []={"����������"};
const unsigned char PY_mb_chun  []={"��������������"};
const unsigned char PY_mb_chuo  []={"��"};
const unsigned char PY_mb_ci    []={"�ôʴĴɴȴǴŴƴ˴δ̴�"};
const unsigned char PY_mb_cong  []={"�ѴӴҴдϴ�"};
const unsigned char PY_mb_cou   []={"��"};
const unsigned char PY_mb_cu    []={"�ִٴ״�"};
const unsigned char PY_mb_cuan  []={"�ڴܴ�"};
const unsigned char PY_mb_cui   []={"�޴ߴݴ�����"};
const unsigned char PY_mb_cun   []={"����"};
const unsigned char PY_mb_cuo   []={"�������"};
const unsigned char PY_mb_da    []={"�������"};
const unsigned char PY_mb_dai   []={"������������������������"};
const unsigned char PY_mb_dan   []={"������������������������������"};
const unsigned char PY_mb_dang  []={"����������"};
const unsigned char PY_mb_dao   []={"������������������������"};
const unsigned char PY_mb_de    []={"�õµ�"};
const unsigned char PY_mb_deng  []={"�Ƶǵŵȵ˵ʵ�"};
const unsigned char PY_mb_di    []={"�͵̵εҵϵеӵѵյ׵ֵصܵ۵ݵڵ޵�"};
const unsigned char PY_mb_dian  []={"���ߵ�������������"};
const unsigned char PY_mb_diao  []={"�����������"};
const unsigned char PY_mb_die   []={"��������������"};
const unsigned char PY_mb_ding  []={"������������������"};
const unsigned char PY_mb_diu   []={"��"};
const unsigned char PY_mb_dong  []={"��������������������"};
const unsigned char PY_mb_dou   []={"����������������"};
const unsigned char PY_mb_du    []={"�����������¶ĶöʶŶǶȶɶ�"};
const unsigned char PY_mb_duan  []={"�˶̶ζ϶ж�"};
const unsigned char PY_mb_dui   []={"�ѶӶԶ�"};
const unsigned char PY_mb_dun   []={"�ֶضն׶ܶ۶ٶ�"};
const unsigned char PY_mb_duo   []={"��߶�޶��������"};
const unsigned char PY_mb_e     []={"����������������"};
const unsigned char PY_mb_en    []={"��"};
const unsigned char PY_mb_er    []={"����������������"};
const unsigned char PY_mb_fa    []={"����������������"};
const unsigned char PY_mb_fan   []={"����������������������������������"};
const unsigned char PY_mb_fang  []={"���������������·÷ķ�"};
const unsigned char PY_mb_fei   []={"�ɷǷȷƷʷ˷̷ͷϷзη�"};
const unsigned char PY_mb_fen   []={"�ַԷ׷ҷշӷطڷٷ۷ݷܷ޷߷�"};
const unsigned char PY_mb_feng  []={"����������������"};
const unsigned char PY_mb_fo    []={"��"};
const unsigned char PY_mb_fou   []={"��"};
const unsigned char PY_mb_fu    []={"������󸥷�����������������������������������������������������������������������������"};
const unsigned char PY_mb_ga    []={"�¸�"};
const unsigned char PY_mb_gai   []={"�øĸƸǸȸ�"};
const unsigned char PY_mb_gan   []={"�ɸʸ˸θ̸͸ѸϸҸи�"};
const unsigned char PY_mb_gang  []={"�Ըոڸٸظ׸ָ۸�"};
const unsigned char PY_mb_gao   []={"�޸�߸�ݸ�����"};
const unsigned char PY_mb_ge    []={"����������������������"};
const unsigned char PY_mb_gei   []={"��"};
const unsigned char PY_mb_gen   []={"����"};
const unsigned char PY_mb_geng  []={"��������������"};
const unsigned char PY_mb_gong  []={"������������������������������"};
const unsigned char PY_mb_gou   []={"������������������"};
const unsigned char PY_mb_gu    []={"�����ù¹��������ŹȹɹǹƹĹ̹ʹ˹�"};
const unsigned char PY_mb_gua   []={"�Ϲιйѹҹ�"};
const unsigned char PY_mb_guai  []={"�Թչ�"};
const unsigned char PY_mb_guan  []={"�ع۹ٹڹ׹ݹܹ�߹��"};
const unsigned char PY_mb_guang []={"����"};
const unsigned char PY_mb_gui   []={"������������������"};
const unsigned char PY_mb_gun   []={"������"};
const unsigned char PY_mb_guo   []={"������������"};
const unsigned char PY_mb_ha    []={"���"};
const unsigned char PY_mb_hai   []={"��������������"};
const unsigned char PY_mb_han   []={"��������������������������������������"};
const unsigned char PY_mb_hang  []={"������"};
const unsigned char PY_mb_hao   []={"���������úºźƺ�"};
const unsigned char PY_mb_he    []={"�ǺȺ̺ϺκͺӺҺ˺ɺԺкʺغֺպ�"};
const unsigned char PY_mb_hei   []={"�ں�"};
const unsigned char PY_mb_hen   []={"�ۺܺݺ�"};
const unsigned char PY_mb_heng  []={"��ߺ���"};
const unsigned char PY_mb_hong  []={"����������"};
const unsigned char PY_mb_hou   []={"��������"};
const unsigned char PY_mb_hu    []={"������������������������������������"};
const unsigned char PY_mb_hua   []={"������������������"};
const unsigned char PY_mb_huai  []={"����������"};
const unsigned char PY_mb_huan  []={"�����������û»�������������"};
const unsigned char PY_mb_huang []={"�ĻŻʻ˻ƻ̻ͻȻǻɻлλѻ�"};
const unsigned char PY_mb_hui   []={"�һֻӻԻջػ׻ڻܻ������߻޻�ݻٻ�"};
const unsigned char PY_mb_hun   []={"�������"};
const unsigned char PY_mb_huo   []={"�������������"};
const unsigned char PY_mb_ji    []={"���������������������������������������������������������������������ƼǼ��ͼ˼ɼ��ʼ����ȼü̼żļ¼�������"};
const unsigned char PY_mb_jia   []={"�ӼмѼϼҼμԼռ׼ּؼۼݼܼټ޼�Ю"};
const unsigned char PY_mb_jian  []={"����߼����������������������������������������������������"};
const unsigned char PY_mb_jiang []={"��������������������������"};
const unsigned char PY_mb_jiao  []={"���������������������ǽƽʽȽýŽ½��˽ɽнνϽ̽ѽ;���"};
const unsigned char PY_mb_jie   []={"�׽Խӽսҽֽڽٽܽ��ݽ޽ؽ߽����������"};
const unsigned char PY_mb_jin   []={"���������������������������������"};
const unsigned char PY_mb_jing  []={"��������������������������������������������������"};
const unsigned char PY_mb_jiong []={"����"};
const unsigned char PY_mb_jiu   []={"�������žþľ��¾ƾɾʾ̾ξǾȾ;�"};
const unsigned char PY_mb_ju    []={"�ӾоѾԾҾϾֽ۾վ׾ھپؾ�޾ܾ߾����ݾ��۾�"};
const unsigned char PY_mb_juan  []={"��������"};
const unsigned char PY_mb_jue   []={"��������������"};
const unsigned char PY_mb_jun   []={"����������������������"};
const unsigned char PY_mb_ka    []={"������"};
const unsigned char PY_mb_kai   []={"����������"};
const unsigned char PY_mb_kan   []={"��������������"};
const unsigned char PY_mb_kang  []={"��������������"};
const unsigned char PY_mb_kao   []={"��������"};
const unsigned char PY_mb_ke    []={"�����¿ƿÿſĿǿȿɿʿ˿̿Ϳ�"};
const unsigned char PY_mb_ken   []={"�Ͽѿҿ�"};
const unsigned char PY_mb_keng  []={"�Կ�"};
const unsigned char PY_mb_kong  []={"�տ׿ֿ�"};
const unsigned char PY_mb_kou   []={"�ٿڿۿ�"};
const unsigned char PY_mb_ku    []={"�ݿ޿߿����"};
const unsigned char PY_mb_kua   []={"������"};
const unsigned char PY_mb_kuai  []={"�����"};
const unsigned char PY_mb_kuan  []={"���"};
const unsigned char PY_mb_kuang []={"�����������"};
const unsigned char PY_mb_kui   []={"����������������������"};
const unsigned char PY_mb_kun   []={"��������"};
const unsigned char PY_mb_kuo   []={"��������"};
const unsigned char PY_mb_la    []={"��������������"};
const unsigned char PY_mb_lai   []={"������"};
const unsigned char PY_mb_lan   []={"������������������������������"};
const unsigned char PY_mb_lang  []={"��������������"};
const unsigned char PY_mb_lao   []={"������������������"};
const unsigned char PY_mb_le    []={"������"};
const unsigned char PY_mb_lei   []={"����������������������"};
const unsigned char PY_mb_leng  []={"������"};
const unsigned char PY_mb_li    []={"��������������������������������������������������������������������"};
const unsigned char PY_mb_lian  []={"����������������������������"};
const unsigned char PY_mb_liang []={"������������������������"};
const unsigned char PY_mb_liao  []={"������������������������"};
const unsigned char PY_mb_lie   []={"����������"};
const unsigned char PY_mb_lin   []={"������������������������"};
const unsigned char PY_mb_ling  []={"����������������������������"};
const unsigned char PY_mb_liu   []={"����������������������"};
const unsigned char PY_mb_long  []={"��������¡��¤¢£"};
const unsigned char PY_mb_lou   []={"¦¥§¨ª©"};
const unsigned char PY_mb_lu    []={"¶¬®«¯­±²°³½¼¸¹»µ·¾º´"};
const unsigned char PY_mb_luan  []={"������������"};
const unsigned char PY_mb_lue   []={"����"};
const unsigned char PY_mb_lun   []={"��������������"};
const unsigned char PY_mb_luo   []={"������������������������"};
const unsigned char PY_mb_lv    []={"��¿������������������������"};
const unsigned char PY_mb_ma    []={"������������������"};
const unsigned char PY_mb_mai   []={"������������"};
const unsigned char PY_mb_man   []={"����������á������"};
const unsigned char PY_mb_mang  []={"æâäãçå"};
const unsigned char PY_mb_mao   []={"èëìéêîíïðóñò"};
const unsigned char PY_mb_me    []={"ô"};
const unsigned char PY_mb_mei   []={"ûöõü÷ýúøùÿ��þ��������"};
const unsigned char PY_mb_men   []={"������"};
const unsigned char PY_mb_meng  []={"����������������"};
const unsigned char PY_mb_mi    []={"����������������������������"};
const unsigned char PY_mb_mian  []={"������������������"};
const unsigned char PY_mb_miao  []={"����������������"};
const unsigned char PY_mb_mie   []={"����"};
const unsigned char PY_mb_min   []={"������������"};
const unsigned char PY_mb_ming  []={"������������"};
const unsigned char PY_mb_miu   []={"��"};
const unsigned char PY_mb_mo    []={"����ġģĤĦĥĢħĨĩĭİĪįĮīĬ"};
const unsigned char PY_mb_mou   []={"Ĳıĳ"};
const unsigned char PY_mb_mu    []={"ĸĶĵķĴľĿ��ļĹĻ��Ľĺ��"};
const unsigned char PY_mb_na    []={"��������������"};
const unsigned char PY_mb_nai   []={"����������"};
const unsigned char PY_mb_nan   []={"������"};
const unsigned char PY_mb_nang  []={"��"};
const unsigned char PY_mb_nao   []={"����������"};
const unsigned char PY_mb_ne    []={"��"};
const unsigned char PY_mb_nei   []={"����"};
const unsigned char PY_mb_nen   []={"��"};
const unsigned char PY_mb_neng  []={"��"};
const unsigned char PY_mb_ni    []={"����������������������"};
const unsigned char PY_mb_nian  []={"��������������"};
const unsigned char PY_mb_niang []={"����"};
const unsigned char PY_mb_niao  []={"����"};
const unsigned char PY_mb_nie   []={"��������������"};
const unsigned char PY_mb_nin   []={"��"};
const unsigned char PY_mb_ning  []={"��š������Ţ"};
const unsigned char PY_mb_niu   []={"ţŤŦť"};
const unsigned char PY_mb_nong  []={"ũŨŧŪ"};
const unsigned char PY_mb_nu    []={"ūŬŭ"};
const unsigned char PY_mb_nuan  []={"ů"};
const unsigned char PY_mb_nue   []={"űŰ"};
const unsigned char PY_mb_nuo   []={"ŲŵųŴ"};
const unsigned char PY_mb_nv    []={"Ů"};
const unsigned char PY_mb_o     []={"Ŷ"};
const unsigned char PY_mb_ou    []={"ŷŹŸŻżźŽ"};
const unsigned char PY_mb_pa    []={"ſž����������"};
const unsigned char PY_mb_pai   []={"������������"};
const unsigned char PY_mb_pan   []={"����������������"};
const unsigned char PY_mb_pang  []={"����������"};
const unsigned char PY_mb_pao   []={"��������������"};
const unsigned char PY_mb_pei   []={"������������������"};
const unsigned char PY_mb_pen   []={"����"};
const unsigned char PY_mb_peng  []={"����������������������������"};
const unsigned char PY_mb_pi    []={"��������������Ƥ��ƣơ��ƢƥƦƨƧƩ"};
const unsigned char PY_mb_pian  []={"Ƭƫƪƭ"};
const unsigned char PY_mb_piao  []={"ƯƮưƱ"};
const unsigned char PY_mb_pie   []={"ƲƳ"};
const unsigned char PY_mb_pin   []={"ƴƶƵƷƸ"};
const unsigned char PY_mb_ping  []={"ƹƽ��ƾƺƻ��ƿƼ"};
const unsigned char PY_mb_po    []={"����������������"};
const unsigned char PY_mb_pou   []={"��"};
const unsigned char PY_mb_pu    []={"������������������������������"};
const unsigned char PY_mb_qi    []={"������������������������������������������������������������������������"};
const unsigned char PY_mb_qia   []={"��ǡǢ"};
const unsigned char PY_mb_qian  []={"ǧǪǤǨǥǣǦǫǩǰǮǯǬǱǭǳǲǴǷǵǶǸ"};
const unsigned char PY_mb_qiang []={"ǺǼǹǻǿǽǾ��"};
const unsigned char PY_mb_qiao  []={"������������������������������"};
const unsigned char PY_mb_qie   []={"����������"};
const unsigned char PY_mb_qin   []={"����������������������"};
const unsigned char PY_mb_qing  []={"��������������������������"};
const unsigned char PY_mb_qiong []={"����"};
const unsigned char PY_mb_qiu   []={"����������������"};
const unsigned char PY_mb_qu    []={"����������������ȡȢȣȥȤ"};
const unsigned char PY_mb_quan  []={"ȦȫȨȪȭȬȩȧȮȰȯ"};
const unsigned char PY_mb_que   []={"Ȳȱȳȴȸȷȵȶ"};
const unsigned char PY_mb_qun   []={"ȹȺ"};
const unsigned char PY_mb_ran   []={"ȻȼȽȾ"};
const unsigned char PY_mb_rang  []={"ȿ��������"};
const unsigned char PY_mb_rao   []={"������"};
const unsigned char PY_mb_re    []={"����"};
const unsigned char PY_mb_ren   []={"��������������������"};
const unsigned char PY_mb_reng  []={"����"};
const unsigned char PY_mb_ri    []={"��"};
const unsigned char PY_mb_rong  []={"��������������������"};
const unsigned char PY_mb_rou   []={"������"};
const unsigned char PY_mb_ru    []={"��������������������"};
const unsigned char PY_mb_ruan  []={"����"};
const unsigned char PY_mb_rui   []={"������"};
const unsigned char PY_mb_run   []={"����"};
const unsigned char PY_mb_ruo   []={"����"};
const unsigned char PY_mb_sa    []={"������"};
const unsigned char PY_mb_sai   []={"��������"};
const unsigned char PY_mb_san   []={"����ɡɢ"};
const unsigned char PY_mb_sang  []={"ɣɤɥ"};
const unsigned char PY_mb_sao   []={"ɦɧɨɩ"};
const unsigned char PY_mb_se    []={"ɫɬɪ"};
const unsigned char PY_mb_sen   []={"ɭ"};
const unsigned char PY_mb_seng  []={"ɮ"};
const unsigned char PY_mb_sha   []={"ɱɳɴɰɯɵɶɷ��"};
const unsigned char PY_mb_shai  []={"ɸɹ"};
const unsigned char PY_mb_shan  []={"ɽɾɼ��ɺɿ������ɻ������������դ"};
const unsigned char PY_mb_shang []={"����������������"};
const unsigned char PY_mb_shao  []={"����������������������"};
const unsigned char PY_mb_she   []={"������������������������"};
const unsigned char PY_mb_shen  []={"��������������������������������ʲ"};
const unsigned char PY_mb_sheng []={"��������ʤ����ʡʥʢʣ"};
const unsigned char PY_mb_shi   []={"��ʬʧʦʭʫʩʨʪʮʯʱʶʵʰʴʳʷʸʹʼʻʺʿ��������ʾʽ������������������������������������"};
const unsigned char PY_mb_shou  []={"��������������������"};
const unsigned char PY_mb_shu   []={"������������������������������������������������������ˡ����������"};
const unsigned char PY_mb_shua  []={"ˢˣ"};
const unsigned char PY_mb_shuai []={"˥ˤ˦˧"};
const unsigned char PY_mb_shuan []={"˩˨"};
const unsigned char PY_mb_shuang[]={"˫˪ˬ"};
const unsigned char PY_mb_shui  []={"˭ˮ˰˯"};
const unsigned char PY_mb_shun  []={"˱˳˴˲"};
const unsigned char PY_mb_shuo  []={"˵˸˷˶"};
const unsigned char PY_mb_si    []={"˿˾˽˼˹˻˺����������������"};
const unsigned char PY_mb_song  []={"����������������"};
const unsigned char PY_mb_sou   []={"��������"};
const unsigned char PY_mb_su    []={"����������������������"};
const unsigned char PY_mb_suan  []={"������"};
const unsigned char PY_mb_sui   []={"����������������������"};
const unsigned char PY_mb_sun   []={"������"};
const unsigned char PY_mb_suo   []={"����������������"};
const unsigned char PY_mb_ta    []={"����������̡̢̤̣"};
const unsigned char PY_mb_tai   []={"̨̧̥̦̫̭̬̩̪"};
const unsigned char PY_mb_tan   []={"̸̵̷̶̴̮̰̯̲̱̳̹̻̺̼̾̿̽"};
const unsigned char PY_mb_tang  []={"��������������������������"};
const unsigned char PY_mb_tao   []={"����������������������"};
const unsigned char PY_mb_te    []={"��"};
const unsigned char PY_mb_teng  []={"��������"};
const unsigned char PY_mb_ti    []={"������������������������������"};
const unsigned char PY_mb_tian  []={"����������������"};
const unsigned char PY_mb_tiao  []={"������������"};
const unsigned char PY_mb_tie   []={"������"};
const unsigned char PY_mb_ting  []={"��͡����ͤͥͣͦͧ͢"};
const unsigned char PY_mb_tong  []={"ͨͬͮͩͭͯͪͫͳͱͰͲʹ"};
const unsigned char PY_mb_tou   []={"͵ͷͶ͸"};
const unsigned char PY_mb_tu    []={"͹ͺͻͼͽͿ;��������"};
const unsigned char PY_mb_tuan  []={"����"};
const unsigned char PY_mb_tui   []={"������������"};
const unsigned char PY_mb_tun   []={"��������"};
const unsigned char PY_mb_tuo   []={"����������������������"};
const unsigned char PY_mb_wa    []={"��������������"};
const unsigned char PY_mb_wai   []={"����"};
const unsigned char PY_mb_wan   []={"����������������������������������"};
const unsigned char PY_mb_wang  []={"��������������������"};
const unsigned char PY_mb_wei   []={"Σ��΢ΡΪΤΧΥΦΨΩάΫΰαβγέίή��δλζηθξνιμεοκ"};
const unsigned char PY_mb_wen   []={"��������������������"};
const unsigned char PY_mb_weng  []={"������"};
const unsigned char PY_mb_wo    []={"������������������"};
const unsigned char PY_mb_wu    []={"����������������������������������������������������������"};
const unsigned char PY_mb_xi    []={"Ϧϫ����ϣ������Ϣ��Ϥϧϩ����ϬϡϪ��Ϩ����ϥϰϯϮϱϭϴϲϷϵϸ϶"};
const unsigned char PY_mb_xia   []={"ϺϹϻ��Ͽ��ϾϽϼ������"};
const unsigned char PY_mb_xian  []={"ϳ����������������������������������������������������"};
const unsigned char PY_mb_xiang []={"����������������������������������������"};
const unsigned char PY_mb_xiao  []={"����������������С��ТФ��ЧУЦХ"};
const unsigned char PY_mb_xie   []={"ЩШЪЫЭавбгЯЬдйкжмелиз"};
const unsigned char PY_mb_xin   []={"����о����п��н����"};
const unsigned char PY_mb_xing  []={"����������������������������"};
const unsigned char PY_mb_xiong []={"��������������"};
const unsigned char PY_mb_xiu   []={"��������������������"};
const unsigned char PY_mb_xu    []={"��������������������������������������"};
const unsigned char PY_mb_xuan  []={"������������ѡѢѤѣ"};
const unsigned char PY_mb_xue   []={"��ѥѦѨѧѩѪ"};
const unsigned char PY_mb_xun   []={"ѫѬѰѲѮѱѯѭѵѶѴѸѷѳ"};
const unsigned char PY_mb_ya    []={"ѾѹѽѺѻѼ��ѿ����������������"};
const unsigned char PY_mb_yan   []={"������������������������������������������������������������������"};
const unsigned char PY_mb_yang  []={"����������������������������������"};
const unsigned char PY_mb_yao   []={"��������ҢҦҤҥҡң��ҧҨҩҪҫԿ"};
const unsigned char PY_mb_ye    []={"ҬҭүҮҲұҰҵҶҷҳҹҴҺҸ"};
const unsigned char PY_mb_yi    []={"һ����ҽ��ҿҼҾ������������������������������������������������������������������������������������������"};
const unsigned char PY_mb_yin   []={"������������������������������ӡ��"};
const unsigned char PY_mb_ying  []={"ӦӢӤӧӣӥӭӯӫӨөӪӬӮӱӰӳӲ"};
const unsigned char PY_mb_yo    []={"Ӵ"};
const unsigned char PY_mb_yong  []={"ӶӵӸӹӺӷ��ӽӾ��ӿ��Ӽӻ��"};
const unsigned char PY_mb_you   []={"����������������������������������������"};
const unsigned char PY_mb_yu    []={"����������������������������������������������������Ԧ����������ԡԤ������Ԣ��ԣ������ԥ��"};
const unsigned char PY_mb_yuan  []={"ԩԧԨԪԱ԰ԫԭԲԬԮԵԴԳԯԶԷԹԺԸ"};
const unsigned char PY_mb_yue   []={"ԻԼ��������Ծ��Խ"};
const unsigned char PY_mb_yun   []={"������������������������"};
const unsigned char PY_mb_za    []={"������զ"};
const unsigned char PY_mb_zai   []={"����������������"};
const unsigned char PY_mb_zan   []={"��������"};
const unsigned char PY_mb_zang  []={"������"};
const unsigned char PY_mb_zao   []={"����������������������������"};
const unsigned char PY_mb_ze    []={"��������"};
const unsigned char PY_mb_zei   []={"��"};
const unsigned char PY_mb_zen   []={"��"};
const unsigned char PY_mb_zeng  []={"������"};
const unsigned char PY_mb_zha   []={"����������բագէթըե��"};
const unsigned char PY_mb_zhai  []={"իժլ��խծկ"};
const unsigned char PY_mb_zhan  []={"մձճղհնչյոշռսջվ��տպ"};
const unsigned char PY_mb_zhang []={"��������������������������������"};
const unsigned char PY_mb_zhao  []={"��������������������צ"};
const unsigned char PY_mb_zhe   []={"����������������������"};
const unsigned char PY_mb_zhen  []={"��������������������������������֡"};
const unsigned char PY_mb_zheng []={"��������������������֤֣��֢"};
const unsigned char PY_mb_zhi   []={"ְֱֲֳִֵֶַָֹֺֻּֽ֧֥֦֪֭֮֨֯֫֬֩��־������������ֿ������������������"};
const unsigned char PY_mb_zhong []={"����������������������"};
const unsigned char PY_mb_zhou  []={"����������������������������"};
const unsigned char PY_mb_zhu   []={"������������������������������ס��ע��פ��ף��������"};
const unsigned char PY_mb_zhua  []={"ץ"};
const unsigned char PY_mb_zhuai []={"ק"};
const unsigned char PY_mb_zhuan []={"רשת׫׭"};
const unsigned char PY_mb_zhuang[]={"ױׯ׮װ׳״��ײ"};
const unsigned char PY_mb_zhui  []={"׷׵׶׹׺׸"};
const unsigned char PY_mb_zhun  []={"׻׼"};
const unsigned char PY_mb_zhuo  []={"׿׾׽��������������"};
const unsigned char PY_mb_zi    []={"����������������������������"};
const unsigned char PY_mb_zong  []={"��������������"};
const unsigned char PY_mb_zou   []={"��������"};
const unsigned char PY_mb_zu    []={"����������������"};
const unsigned char PY_mb_zuan  []={"׬����"};
const unsigned char PY_mb_zui   []={"��������"};
const unsigned char PY_mb_zun   []={"����"};
const unsigned char PY_mb_zuo   []={"��������������"};
//================================================================================================
//��ѯ����������
PY_indexdef PY_index_a[]={
	{"     ",PY_mb_a},
	{"i    ",PY_mb_ai},
	{"n    ",PY_mb_an},
	{"ng   ",PY_mb_ang},
	{"o    ",PY_mb_ao}
};  
PY_indexdef PY_index_b[]={
	{"a    ",PY_mb_ba},
	{"ai   ",PY_mb_bai},
	{"an   ",PY_mb_ban},
	{"ang  ",PY_mb_bang},
	{"ao   ",PY_mb_bao},
	{"ei   ",PY_mb_bei},
	{"en   ",PY_mb_ben},
	{"eng  ",PY_mb_beng},
	{"i    ",PY_mb_bi},
	{"ian  ",PY_mb_bian},
	{"iao  ",PY_mb_biao},
	{"ie   ",PY_mb_bie},
	{"in   ",PY_mb_bin},
	{"ing  ",PY_mb_bing},
	{"o    ",PY_mb_bo},
	{"u    ",PY_mb_bu}
};
PY_indexdef PY_index_c[]={
	{"a    ",PY_mb_ca},
	{"ai   ",PY_mb_cai},
	{"an   ",PY_mb_can},
	{"ang  ",PY_mb_cang},
	{"ao   ",PY_mb_cao},
	{"e    ",PY_mb_ce},
	{"eng  ",PY_mb_ceng},
	{"ha   ",PY_mb_cha},
	{"hai  ",PY_mb_chai},
	{"han  ",PY_mb_chan},
	{"hang ",PY_mb_chang},
	{"hao  ",PY_mb_chao},
	{"he   ",PY_mb_che},
	{"hen  ",PY_mb_chen},
	{"heng ",PY_mb_cheng},
	{"hi   ",PY_mb_chi},
	{"hong ",PY_mb_chong},
	{"hou  ",PY_mb_chou},
	{"hu   ",PY_mb_chu},
	{"huai ",PY_mb_chuai},
	{"huan ",PY_mb_chuan},
	{"huang",PY_mb_chuang},
	{"hui  ",PY_mb_chui},
	{"hun  ",PY_mb_chun},
	{"huo  ",PY_mb_chuo},
	{"i    ",PY_mb_ci},
	{"ong  ",PY_mb_cong},
	{"ou   ",PY_mb_cou},
	{"u    ",PY_mb_cu},
	{"uan  ",PY_mb_cuan},
	{"ui   ",PY_mb_cui},
	{"un   ",PY_mb_cun},
	{"uo   ",PY_mb_cuo}
};							
PY_indexdef PY_index_d[]={
	{"a    ",PY_mb_da},
	{"ai   ",PY_mb_dai},
	{"an   ",PY_mb_dan},
	{"ang  ",PY_mb_dang},
	{"ao   ",PY_mb_dao},
	{"e    ",PY_mb_de},
	{"eng  ",PY_mb_deng},
	{"i    ",PY_mb_di},
	{"ian  ",PY_mb_dian},
	{"iao  ",PY_mb_diao},
	{"ie   ",PY_mb_die},
	{"ing  ",PY_mb_ding},
	{"iu   ",PY_mb_diu},
	{"ong  ",PY_mb_dong},
	{"ou   ",PY_mb_dou},
	{"u    ",PY_mb_du},
	{"uan  ",PY_mb_duan},
	{"ui   ",PY_mb_dui},
	{"un   ",PY_mb_dun},
	{"uo   ",PY_mb_duo}
};
PY_indexdef PY_index_e[]={
	{"     ",PY_mb_e},
	{"n    ",PY_mb_en},
	{"r    ",PY_mb_er}
};
PY_indexdef PY_index_f[]={
	{"a    ",PY_mb_fa},
	{"an   ",PY_mb_fan},
	{"ang  ",PY_mb_fang},
	{"ei   ",PY_mb_fei},
	{"en   ",PY_mb_fen},
	{"eng  ",PY_mb_feng},
	{"o    ",PY_mb_fo},
	{"ou   ",PY_mb_fou},
	{"u    ",PY_mb_fu}
};
PY_indexdef PY_index_g[]={
	{"a    ",PY_mb_ga},
	{"ai   ",PY_mb_gai},
	{"an   ",PY_mb_gan},
	{"ang  ",PY_mb_gang},
	{"ao   ",PY_mb_gao},
	{"e    ",PY_mb_ge},
	{"ei   ",PY_mb_gei},
	{"en   ",PY_mb_gen},
	{"eng  ",PY_mb_geng},
	{"ong  ",PY_mb_gong},
	{"ou   ",PY_mb_gou},
	{"u    ",PY_mb_gu},
	{"ua   ",PY_mb_gua},
	{"uai  ",PY_mb_guai},
	{"uan  ",PY_mb_guan},
	{"uang ",PY_mb_guang},
	{"ui   ",PY_mb_gui},
	{"un   ",PY_mb_gun},
	{"uo   ",PY_mb_guo}
};
PY_indexdef PY_index_h[]={
	{"a    ",PY_mb_ha},
	{"ai   ",PY_mb_hai},
	{"an   ",PY_mb_han},
	{"ang  ",PY_mb_hang},
	{"ao   ",PY_mb_hao},
	{"e    ",PY_mb_he},
	{"ei   ",PY_mb_hei},
	{"en   ",PY_mb_hen},
	{"eng  ",PY_mb_heng},
	{"ong  ",PY_mb_hong},
	{"ou   ",PY_mb_hou},
	{"u    ",PY_mb_hu},
	{"ua   ",PY_mb_hua},
	{"uai  ",PY_mb_huai},
	{"uan  ",PY_mb_huan},
	{"uang ",PY_mb_huang},
	{"ui   ",PY_mb_hui},
	{"un   ",PY_mb_hun},
	{"uo   ",PY_mb_huo}
};
PY_indexdef PY_index_j[]={
	{"i    ",PY_mb_ji},
	{"ia   ",PY_mb_jia},
	{"ian  ",PY_mb_jian},
	{"iang ",PY_mb_jiang},
	{"iao  ",PY_mb_jiao},
	{"ie   ",PY_mb_jie},
	{"in   ",PY_mb_jin},
	{"ing  ",PY_mb_jing},
	{"iong ",PY_mb_jiong},
	{"iu   ",PY_mb_jiu},
	{"u    ",PY_mb_ju},
	{"uan  ",PY_mb_juan},
	{"ue   ",PY_mb_jue},
	{"un   ",PY_mb_jun}
};
PY_indexdef PY_index_k[]={
	{"a    ",PY_mb_ka},
	{"ai   ",PY_mb_kai},
	{"an   ",PY_mb_kan},
	{"ang  ",PY_mb_kang},
	{"ao   ",PY_mb_kao},
	{"e    ",PY_mb_ke},
	{"en   ",PY_mb_ken},
	{"eng  ",PY_mb_keng},
	{"ong  ",PY_mb_kong},
	{"ou   ",PY_mb_kou},
	{"u    ",PY_mb_ku},
	{"ua   ",PY_mb_kua},
	{"uai  ",PY_mb_kuai},
	{"uan  ",PY_mb_kuan},
	{"uang ",PY_mb_kuang},
	{"ui   ",PY_mb_kui},
	{"un   ",PY_mb_kun},
	{"uo   ",PY_mb_kuo}
};
PY_indexdef PY_index_l[]={
	{"a    ",PY_mb_la},
	{"ai   ",PY_mb_lai},
	{"an   ",PY_mb_lan},
	{"ang  ",PY_mb_lang},
	{"ao   ",PY_mb_lao},
	{"e    ",PY_mb_le},
	{"ei   ",PY_mb_lei},
	{"eng  ",PY_mb_leng},
	{"i    ",PY_mb_li},
	{"ian  ",PY_mb_lian},
	{"iang ",PY_mb_liang},
	{"iao  ",PY_mb_liao},
	{"ie   ",PY_mb_lie},
	{"in   ",PY_mb_lin},
	{"ing  ",PY_mb_ling},
	{"iu   ",PY_mb_liu},
	{"ong  ",PY_mb_long},
	{"ou   ",PY_mb_lou},
	{"u    ",PY_mb_lu},
	{"uan  ",PY_mb_luan},
	{"ue   ",PY_mb_lue},
	{"un   ",PY_mb_lun},
	{"uo   ",PY_mb_luo},
	{"v    ",PY_mb_lv}
};
PY_indexdef PY_index_m[]={
	{"a    ",PY_mb_ma},
	{"ai   ",PY_mb_mai},
	{"an   ",PY_mb_man},
	{"ang  ",PY_mb_mang},
	{"ao   ",PY_mb_mao},
	{"e    ",PY_mb_me},
	{"ei   ",PY_mb_mei},
	{"en   ",PY_mb_men},
	{"eng  ",PY_mb_meng},
	{"i    ",PY_mb_mi},
	{"ian  ",PY_mb_mian},
	{"iao  ",PY_mb_miao},
	{"ie   ",PY_mb_mie},
	{"in   ",PY_mb_min},
	{"ing  ",PY_mb_ming},
	{"iu   ",PY_mb_miu},
	{"o    ",PY_mb_mo},
	{"ou   ",PY_mb_mou},
	{"u    ",PY_mb_mu}
};
PY_indexdef PY_index_n[]={
	{"a    ",PY_mb_na},
	{"ai   ",PY_mb_nai},
	{"an   ",PY_mb_nan},
	{"ang  ",PY_mb_nang},
	{"ao   ",PY_mb_nao},
	{"e    ",PY_mb_ne},
	{"ei   ",PY_mb_nei},
	{"en   ",PY_mb_nen},
	{"eng  ",PY_mb_neng},
	{"i    ",PY_mb_ni},
	{"ian  ",PY_mb_nian},
	{"iang ",PY_mb_niang},
	{"iao  ",PY_mb_niao},
	{"ie   ",PY_mb_nie},
	{"in   ",PY_mb_nin},
	{"ing  ",PY_mb_ning},
	{"iu   ",PY_mb_niu},
	{"ong  ",PY_mb_nong},
	{"u    ",PY_mb_nu},
	{"uan  ",PY_mb_nuan},
	{"ue   ",PY_mb_nue},
	{"uo   ",PY_mb_nuo},
	{"v    ",PY_mb_nv}
};
PY_indexdef PY_index_o[]={
	{"     ",PY_mb_o},
	{"u    ",PY_mb_ou}
};
PY_indexdef PY_index_p[]={
	{"a    ",PY_mb_pa},
	{"ai   ",PY_mb_pai},
	{"an   ",PY_mb_pan},
	{"ang  ",PY_mb_pang},
	{"ao   ",PY_mb_pao},
	{"ei   ",PY_mb_pei},
	{"en   ",PY_mb_pen},
	{"eng  ",PY_mb_peng},
	{"i    ",PY_mb_pi},
	{"ian  ",PY_mb_pian},
	{"iao  ",PY_mb_piao},
	{"ie   ",PY_mb_pie},
	{"in   ",PY_mb_pin},
	{"ing  ",PY_mb_ping},
	{"o    ",PY_mb_po},
	{"ou   ",PY_mb_pou},
	{"u    ",PY_mb_pu}
};
PY_indexdef PY_index_q[]={
	{"i    ",PY_mb_qi},
	{"ia   ",PY_mb_qia},
	{"ian  ",PY_mb_qian},
	{"iang ",PY_mb_qiang},
	{"iao  ",PY_mb_qiao},
	{"ie   ",PY_mb_qie},
	{"in   ",PY_mb_qin},
	{"ing  ",PY_mb_qing},
	{"iong ",PY_mb_qiong},
	{"iu   ",PY_mb_qiu},
	{"u    ",PY_mb_qu},
	{"uan  ",PY_mb_quan},
	{"ue   ",PY_mb_que},
	{"un   ",PY_mb_qun}
};
PY_indexdef PY_index_r[]={
	{"an   ",PY_mb_ran},
	{"ang  ",PY_mb_rang},
	{"ao   ",PY_mb_rao},
	{"e    ",PY_mb_re},
	{"en   ",PY_mb_ren},
	{"eng  ",PY_mb_reng},
	{"i    ",PY_mb_ri},
	{"ong  ",PY_mb_rong},
	{"ou   ",PY_mb_rou},
	{"u    ",PY_mb_ru},
	{"uan  ",PY_mb_ruan},
	{"ui   ",PY_mb_rui},
	{"un   ",PY_mb_run},
	{"uo   ",PY_mb_ruo}
};
PY_indexdef PY_index_s[]={
	{"a    ",PY_mb_sa},
	{"ai   ",PY_mb_sai},
	{"an   ",PY_mb_san},
	{"ang  ",PY_mb_sang},
	{"ao   ",PY_mb_sao},
	{"e    ",PY_mb_se},
	{"en   ",PY_mb_sen},
	{"eng  ",PY_mb_seng},
	{"ha   ",PY_mb_sha},
	{"hai  ",PY_mb_shai},
	{"han  ",PY_mb_shan},
	{"hang ",PY_mb_shang},
	{"hao  ",PY_mb_shao},
	{"he   ",PY_mb_she},
	{"hen  ",PY_mb_shen},
	{"heng ",PY_mb_sheng},
	{"hi   ",PY_mb_shi},
	{"hou  ",PY_mb_shou},
	{"hu   ",PY_mb_shu},
	{"hua  ",PY_mb_shua},
	{"huai ",PY_mb_shuai},
	{"huan ",PY_mb_shuan},
	{"huang",PY_mb_shuang},
	{"hui  ",PY_mb_shui},
	{"hun  ",PY_mb_shun},
	{"huo  ",PY_mb_shuo},
	{"i    ",PY_mb_si},
	{"ong  ",PY_mb_song},
	{"ou   ",PY_mb_sou},
	{"u    ",PY_mb_su},
	{"uan  ",PY_mb_suan},
	{"ui   ",PY_mb_sui},
	{"un   ",PY_mb_sun},
	{"uo   ",PY_mb_suo}
};
PY_indexdef PY_index_t[]={
	{"a    ",PY_mb_ta},
	{"ai   ",PY_mb_tai},
	{"an   ",PY_mb_tan},
	{"ang  ",PY_mb_tang},
	{"ao   ",PY_mb_tao},
	{"e    ",PY_mb_te},
	{"eng  ",PY_mb_teng},
	{"i    ",PY_mb_ti},
	{"ian  ",PY_mb_tian},
	{"iao  ",PY_mb_tiao},
	{"ie   ",PY_mb_tie},
	{"ing  ",PY_mb_ting},
	{"ong  ",PY_mb_tong},
	{"ou   ",PY_mb_tou},
	{"u    ",PY_mb_tu},
	{"uan  ",PY_mb_tuan},
	{"ui   ",PY_mb_tui},
	{"un   ",PY_mb_tun},
	{"uo   ",PY_mb_tuo}
};
PY_indexdef PY_index_w[]={
	{"a    ",PY_mb_wa},
	{"ai   ",PY_mb_wai},
	{"an   ",PY_mb_wan},
	{"ang  ",PY_mb_wang},
	{"ei   ",PY_mb_wei},
	{"en   ",PY_mb_wen},
	{"eng  ",PY_mb_weng},
	{"o    ",PY_mb_wo},
	{"u    ",PY_mb_wu}
};
PY_indexdef PY_index_x[]={
	{"i    ",PY_mb_xi},
	{"ia   ",PY_mb_xia},
	{"ian  ",PY_mb_xian},
	{"iang ",PY_mb_xiang},
	{"iao  ",PY_mb_xiao},
	{"ie   ",PY_mb_xie},
	{"in   ",PY_mb_xin},
	{"ing  ",PY_mb_xing},
	{"iong ",PY_mb_xiong},
	{"iu   ",PY_mb_xiu},
	{"u    ",PY_mb_xu},
	{"uan  ",PY_mb_xuan},
	{"ue   ",PY_mb_xue},
	{"un   ",PY_mb_xun}
};
PY_indexdef PY_index_y[]={
	{"a    ",PY_mb_ya},
	{"an   ",PY_mb_yan},
	{"ang  ",PY_mb_yang},
	{"ao   ",PY_mb_yao},
	{"e    ",PY_mb_ye},
	{"i    ",PY_mb_yi},
	{"in   ",PY_mb_yin},
	{"ing  ",PY_mb_ying},
	{"o    ",PY_mb_yo},
	{"ong  ",PY_mb_yong},
	{"ou   ",PY_mb_you},
	{"u    ",PY_mb_yu},
	{"uan  ",PY_mb_yuan},
	{"ue   ",PY_mb_yue},
	{"un   ",PY_mb_yun}
};
PY_indexdef PY_index_z[]={
	{"a    ",PY_mb_za},
	{"ai   ",PY_mb_zai},
	{"an   ",PY_mb_zan},
	{"ang  ",PY_mb_zang},
	{"ao   ",PY_mb_zao},
	{"e    ",PY_mb_ze},
	{"ei   ",PY_mb_zei},
	{"en   ",PY_mb_zen},
	{"eng  ",PY_mb_zeng},
	{"ha   ",PY_mb_zha},
	{"hai  ",PY_mb_zhai},
	{"han  ",PY_mb_zhan},
	{"hang ",PY_mb_zhang},
	{"hao  ",PY_mb_zhao},
	{"he   ",PY_mb_zhe},
	{"hen  ",PY_mb_zhen},
	{"heng ",PY_mb_zheng},
	{"hi   ",PY_mb_zhi},
	{"hong ",PY_mb_zhong},
	{"hou  ",PY_mb_zhou},
	{"hu   ",PY_mb_zhu},
	{"hua  ",PY_mb_zhua},
	{"huai ",PY_mb_zhuai},
	{"huan ",PY_mb_zhuan},
	{"huang",PY_mb_zhuang},
	{"hui  ",PY_mb_zhui},
	{"hun  ",PY_mb_zhun},
	{"huo  ",PY_mb_zhuo},
	{"i    ",PY_mb_zi},
	{"ong  ",PY_mb_zong},
	{"ou   ",PY_mb_zou},
	{"u    ",PY_mb_zu},
	{"uan  ",PY_mb_zuan},
	{"ui   ",PY_mb_zui},
	{"un   ",PY_mb_zun},
	{"uo   ",PY_mb_zuo}
};
PY_indexdef PY_index_null[]={
	{"     ",0}
};
//ƴ������������������� ������ƴ���ĵ�һ����ĸ����һ�����  
PY_indexdef *const PY_index_pointer[27]={
	PY_index_a,PY_index_b,PY_index_c,PY_index_d,PY_index_e,PY_index_f,PY_index_g,PY_index_h,
	PY_index_null,PY_index_j,PY_index_k,PY_index_l,PY_index_m,PY_index_n,PY_index_o,PY_index_p,
	PY_index_q,PY_index_r,PY_index_s,PY_index_t,PY_index_null,PY_index_null,PY_index_w,PY_index_x,
	PY_index_y,PY_index_z,PY_index_null
};
//ƴ����ѯ���� 
//����:ƴ���ַ��� 
//���:�������һ��ƥ��ɹ������ƺ������� ������һ����ĸ����ƥ�����ѯʧ�ܷ���0 
const unsigned char * py_to_str(unsigned char input_py_val[]){
	PY_indexdef *p1,*p2,*p3;
	unsigned char i; 
	
	if ( input_py_val[0]<'a' || input_py_val[0]>'z' ||input_py_val[0]=='i'||input_py_val[0]=='u'||input_py_val[0]=='v')   
		return 0;
							
	p1 = PY_index_pointer[input_py_val[0] - 'a'];//����ƴ���ĵ�һ����ĸ��λ����Ӧ�Ĳ�ѯ����������
	p3 = PY_index_pointer[input_py_val[0] - 'a' +1];//p3��Ϊ����ָ�� ֻ�����︳ֵ ����ѯ��Χ�޶���һ��ƴ�������������			
	p2 = p1; //p2�����������һ�β�ѯ���
									
	while (1){ 									
		if (p1->matchstr[0] == input_py_val[1]){
			p2 = p1;  
			break;	//��ʱ p1 ָ���˲�ѯ�����������е�һ��ƥ���˵ڶ�����ĸ�Ĳ�ѯ������
		}
		else{ 
			p1++;
			if( p1 == p3 )
				return(p2->pmaybe);
		} 	
	}	
	i=3;
	//i���������ݲ�ѯʱ�Ľ��� 
	//֮���Կ����ý��ݷ���ѯ����Ϊ��ѯ������������ض�����˳��
	//i���ڼ���ʾ����ƥ��ƴ���ĵڼ���ƴ����ĸ
	while (1){										
		if ( (p1->matchstr[i-2] == input_py_val[i - 1]) ) {
			p2 = p1;
			i++;
		} 
		else{ 
			p1++;
			if( p1 == p3 )
				return(p2->pmaybe);
		}
	}
}

