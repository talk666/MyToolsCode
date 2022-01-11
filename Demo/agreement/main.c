#include <stdio.h>
#include <stdlib.h>
#include "sm2.h"
#include "hex_dump.h"
#include "utils.h"

typedef struct ECCrefPublicKey_st
{   
    unsigned int bits;
    unsigned char x[64];//取后32位 xy
    unsigned char y[64];
} ECCrefPublicKey;

typedef struct ECCrefPrivateKey_st
{   
    unsigned int bits;
    unsigned char D[64];
} ECCrefPrivateKey;

int Zayk_KeyAgreement_ECC(uint32_t uiFlag, uint32_t uiKeyNumber,
                             ECCrefPrivateKey *pstSM2PriKeyThis, ECCrefPrivateKey *pstSM2PriKeyTmpThis,
                             ECCrefPublicKey *pstSM2PubKeyThis, ECCrefPublicKey *pstSM2PubKeyTmpThis,
                             ECCrefPublicKey *pstSM2PubKeyThat, ECCrefPublicKey *pstSM2PubKeyTmpThat,
                             uint8_t *pucIDThis, uint32_t uiIDLenThis, uint8_t *pucIDThat, uint32_t uiIDLenThat,
                             uint32_t uiKeyLen, uint8_t *pucKey)
{
    int ret;
    uint32_t Flag = uiFlag;//发起方 1
    uint8_t ucaKeybuff[256];
    
    if (Flag == 1) {

        ret = sm2_keyagreement_a(pstSM2PubKeyTmpThis->x + 32,
                                 32,
                                 pstSM2PubKeyTmpThis->y + 32,
                                 32,
                                 pstSM2PubKeyThis->x + 32,
                                 32,
                                 pstSM2PubKeyThis->y + 32,
                                 32,
                                 pstSM2PriKeyThis->D + 32,
                                 32,
                                 pstSM2PubKeyThat->x + 32,
                                 32,
                                 pstSM2PubKeyThat->y + 32,
                                 32,
                                 pucIDThis,
                                 uiIDLenThis,
                                 pucIDThat,
                                 uiIDLenThat,
                                 pstSM2PubKeyTmpThat->x + 32,
                                 32,
                                 pstSM2PubKeyTmpThat->y + 32,
                                 32,
                                 pstSM2PriKeyTmpThis->D + 32,
                                 32,
                                 uiKeyLen,
                                 pucKey,
                                 NULL,
                                 NULL);
        hex_dump("ucaKeybuff\r\n", 0, pucKey, uiKeyLen);

    } else {
        ret = sm2_keyagreement_b(pstSM2PubKeyTmpThis->x + 32,
                                 32,
                                 pstSM2PubKeyTmpThis->y + 32,
                                 32,
                                 pstSM2PubKeyThis->x + 32,
                                 32,
                                 pstSM2PubKeyThis->y + 32,
                                 32,
                                 pstSM2PriKeyThis->D + 32,
                                 32,
                                 pstSM2PubKeyThat->x + 32,
                                 32,
                                 pstSM2PubKeyThat->y + 32,
                                 32,
                                 pucIDThis,
                                 uiIDLenThis,
                                 pucIDThat,
                                 uiIDLenThat,
                                 pstSM2PubKeyTmpThat->x + 32,
                                 32,
                                 pstSM2PubKeyTmpThat->y + 32,
                                 32,
                                 pstSM2PriKeyTmpThis->D + 32,
                                 32,
                                 uiKeyLen,
                                 pucKey,
                                 NULL,
                                 NULL);
        hex_dump("ucbKeybuff\r\n", 0, pucKey, uiKeyLen);

    }

    if (ret != 1) {
        return -1;
    }

    return 0;
}

int main()
{   
    int ret;
    // ECCrefPublicKey public;
    // memset(&public, 'B', sizeof(ECCrefPublicKey));
    // ECCrefPrivateKey private;
    // memset(&public, 'C', sizeof(ECCrefPrivateKey));


    // uint8_t *pucIDThis ="53706F6E736F72";
    // uint8_t *pucIDThat ="53706F6E736F7227";

    // uint8_t pucKey[256];
    printf("main start\r\n");
    unsigned char ida[32];
    unsigned char da[32];
    unsigned char pa[64];
    unsigned char rda[32];
    unsigned char rpa[64];
    unsigned char idb[32];
    unsigned char db[32];
    unsigned char pb[64];
    unsigned char rdb[32];
    unsigned char rpb[64];
    unsigned char key[32];

    unsigned char keya[32];
    unsigned char keyb[32];
    unsigned int uiIdLen;
    unsigned int uiKeyLen;
    int iDataLen;

    ECCrefPublicKey stEccPubKeya;
    ECCrefPrivateKey stEccPriKeya;
    ECCrefPublicKey stEccPubKeyta;
    ECCrefPrivateKey stEccPriKeyta;

    ECCrefPublicKey stEccPubKeyb;
    ECCrefPrivateKey stEccPriKeyb;
    ECCrefPublicKey stEccPubKeytb;
    ECCrefPrivateKey stEccPriKeytb;

    

    #if 0
    // 发起方ID
    char *str_ida = "80CE56A72DFA5D3B2BE85F3371A4D94030E38C5F6EF4E1010CA1ADBCC81EE8C3";
    // 发起方私钥
    char *str_da = "D54C2CE798B7FD0AEE0B0A8472D21E9FABB3086A213B12168F7E5A6131BB6D71";
    // 发起方公钥
    char *str_pa = "1EB0B42E81961E09E468A76E1C91906A4A8CB8A09CFA3A6E71E721FA9F0978FB"
                    "D41BB83F41F4C7BDDB22A23BFBF5A51ABE51317C05AD0AD779D210C7A937DE4A";
    // 发起方临时私钥
    char *str_rda = "F876F730D7F1C734E12BCCD3D56F89FF1511A8E542739BEDC56AE029C1D81554";
    // 发起方临时公钥
    char *str_rpa = "98A8EFF9EF27E47455C745AA4778E9020FC6CD9A7AFDAE22D9AA3DF77BA65CED"
                    "16CC6D8191529043A3BDE9CFD999A6DC34C9D4811F0D87589595EF76D5D84577";

    // 响应方ID
    char *str_idb = "D42EEA8E1429B1D311833378576D86B2C0DBD722D13CD1895648144F718C8983";
    // 响应方私钥
    char *str_db = "3A22FC939F7EBE1968DC817C80E52867DD9E3212853570AFA5F28E5AB1428BB0";
    // 响应方公钥
    char *str_pb = "BE84B6D4C7DCA415FE1C6EE35F15BD1A05CE1361600D6D35C27D69231364E272"
                    "A56E895FB80CBD5177938BB18F7D2EB805E9409FC8637665C0B8229705F9E235";
    // 响应方临时私钥
    char *str_rdb = "4599DE0EB5D504C99593C73CF2B1501F1A042015C21956C64C6181374B7994C9";
    // 响应方临时公钥;
    char *str_rpb = "FF55CDD081CBE4427FA2EA3EE5C0D381C499F66744ACA8049D2EDE712E570E82"
                    "38CA5F2CF3192F9AB571FB29A97E2B4132A363B330C5A1031D6A0AD1582CBA6F";
    // char *str_std_s1sb = "d3a0fe15dee185ceae907a6b595cc32a266ed7b3367e9983a896dc32fa20f8eb";
    // char *str_std_s2sa = "18c7894b3816df16cf07b05c5ec0bef5d655d58f779cc1b400a4f3884644db88";
    // 协商密钥
    char *str_std_key = "04686E465C90CD9A45A0F9E38E76649ABC1DB5CBF4DBE021BE8EAEF030903319";
    #else
    char *str_ida = "31323334353637383132333435363738";
    char *str_da  = "81eb26e941bb5af16df116495f90695272ae2cd63d6c4ae1678418be48230029";
    char *str_pa = "160e12897df4edb61dd812feb96748fbd3ccf4ffe26aa6f6db9540af49c942324a7dad08bb9a459531694beb20aa489"
                    "d6649975e1bfcf8c4741b78b4b223007f";
    char *str_rda = "d4de15474db74d06491c440d305e012400990f3e390c7e87153c12db2ea60bb3";
    char *str_rpa = "64ced1bdbc99d590049b434d0fd73428cf608a5db8fe5ce07f15026940bae40e376629c7ab21e7db260922499ddb11"
                    "8f07ce8eaae3e7720afef6a5cc062070c0";
    char *str_idb = "31323334353637383132333435363738";
    char *str_db  = "785129917d45a9ea5437a59356b82338eaadda6ceb199088f14ae10defa229b5";
    char *str_pb = "6ae848c57c53c7b1b5fa99eb2286af078ba64c64591b8b566f7357d576f16dfbee489d771621a27b36c5c7992062e9c"
                    "d09a9264386f3fbea54dff69305621c4d";
    char *str_rdb = "7e07124814b309489125eaed101113164ebf0f3458c5bd88335c1f9d596243d6";
    char *str_rpb = "acc27688a6f7b706098bc91ff3ad1bff7dc2802cdb14ccccdb0a90471f9bd7072fedac0494b2ffc4d6853876c79b8f"
                    "301c6573ad0aa50f39fc87181e1a1b46fe";
    // char *str_std_s1sb = "d3a0fe15dee185ceae907a6b595cc32a266ed7b3367e9983a896dc32fa20f8eb";
    // char *str_std_s2sa = "18c7894b3816df16cf07b05c5ec0bef5d655d58f779cc1b400a4f3884644db88";
    char *str_std_key = "6c89347354de2484c60b4ab1fde4c6e5";
    #endif

    memset(&stEccPubKeya, 0, sizeof(stEccPubKeya));
    memset(&stEccPubKeyb, 0, sizeof(stEccPubKeya));
    memset(&stEccPriKeya, 0, sizeof(stEccPriKeya));
    memset(&stEccPriKeyb, 0, sizeof(stEccPriKeyb));
    stEccPubKeya.bits = 256;
    stEccPubKeyb.bits = 256;
    stEccPriKeya.bits = 256;
    stEccPriKeyb.bits = 256;

    memset(&stEccPubKeyta, 0, sizeof(stEccPubKeya));
    memset(&stEccPubKeytb, 0, sizeof(stEccPubKeya));
    memset(&stEccPriKeyta, 0, sizeof(stEccPriKeya));
    memset(&stEccPriKeytb, 0, sizeof(stEccPriKeyb));
    stEccPubKeyta.bits = 256;
    stEccPubKeytb.bits = 256;
    stEccPriKeyta.bits = 256;
    stEccPriKeytb.bits = 256;

    hex_string_to_array(str_ida, ida, &iDataLen);
    hex_string_to_array(str_da, da, &iDataLen);
    memcpy(&stEccPriKeya.D[32], da, 32);
    hex_string_to_array(str_pa, pa, &iDataLen);
    memcpy(&stEccPubKeya.x[32], pa, 32);
    memcpy(&stEccPubKeya.y[32], &pa[32], 32);
    hex_string_to_array(str_rda, rda, &iDataLen);
    memcpy(&stEccPriKeyta.D[32], rda, 32);
    hex_string_to_array(str_rpa, rpa, &iDataLen);
    memcpy(&stEccPubKeyta.x[32], rpa, 32);
    memcpy(&stEccPubKeyta.y[32], &rpa[32], 32);
    hex_string_to_array(str_idb, idb, &uiIdLen);
    hex_string_to_array(str_db, db, &iDataLen);
    memcpy(&stEccPriKeyb.D[32], db, 32);
    hex_string_to_array(str_pb, pb, &iDataLen);
    memcpy(&stEccPubKeyb.x[32], pb, 32);
    memcpy(&stEccPubKeyb.y[32], &pb[32], 32);
    hex_string_to_array(str_rdb, rdb, &iDataLen);
    memcpy(&stEccPriKeytb.D[32], rdb, 32);
    hex_string_to_array(str_rpb, rpb, &iDataLen);
    memcpy(&stEccPubKeytb.x[32], rpb, 32);
    memcpy(&stEccPubKeytb.y[32], &rpb[32], 32);
    // hex_string_to_array(str_std_s1sb, s1sb, &iDataLen);
    // hex_string_to_array(str_std_s2sa, s2sa, &iDataLen);
    hex_string_to_array(str_std_key, key, &uiKeyLen);

    printf("sm2 key agreement test ");

    ret =  Zayk_KeyAgreement_ECC(  1,
                                   0,
                                   &stEccPriKeya,
                                   &stEccPriKeyta,
                                   &stEccPubKeya,
                                   &stEccPubKeyta,
                                   &stEccPubKeyb,
                                   &stEccPubKeytb,
                                   ida,
                                   uiIdLen,
                                   idb,
                                   uiIdLen,
                                   uiKeyLen,
                                   keya);
    if(ret != 0)
    {
        printf("agreement fail!\r\n");
    }
    else
    {
        printf("agreement secceed!\r\n");
    }

    ret =  Zayk_KeyAgreement_ECC(  2,
                                   0,
                                   &stEccPriKeyb,
                                   &stEccPriKeytb,
                                   &stEccPubKeyb,
                                   &stEccPubKeytb,
                                   &stEccPubKeya,
                                   &stEccPubKeyta,
                                   idb,
                                   uiIdLen,
                                   ida,
                                   uiIdLen,
                                   uiKeyLen,
                                   keyb);
    if(ret != 0)
    {
        printf("agreement fail!\r\n");
    }
    else
    {
        printf("agreement secceed!\r\n");
    }
   

    return 0;
}
