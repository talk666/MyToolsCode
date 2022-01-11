#ifndef __SM2_HEADER_2011_01_28__
#define __SM2_HEADER_2011_01_28__

#ifdef  __cplusplus
extern "C" {
#endif

#include "miracl/miracl.h""
#include "sm3.h"


unsigned char *sm3_e(unsigned char *userid, int userid_len, unsigned char *xa, int xa_len, unsigned char *ya, int ya_len, unsigned char *msg, int msg_len, unsigned char *e);
/*
���ܣ������û�ID����Կ��������ǩ������ǩ����ϢHASHֵ
[����] userid�� �û�ID
[����] userid_len�� userid���ֽ���
[����] xa�� ��Կ��X����
[����] xa_len: xa���ֽ���
[����] ya�� ��Կ��Y����
[����] ya_len: ya���ֽ���
[����] msg��Ҫǩ������Ϣ
[����] msg_len�� msg���ֽ���
[���] e��32�ֽڣ�����ǩ������ǩ

����ֵ��
		NULL��       ʧ��
		ָ��e��ָ�룺�ɹ�
*/


int sm3_z(unsigned char *userid, int userid_len, unsigned char *xa, int xa_len, unsigned char *ya, int ya_len, unsigned char *z);
/*
���ܣ������û�ID����Կ����Zֵ
[����] userid�� �û�ID
[����] userid_len�� userid���ֽ���
[����] xa�� ��Կ��X����
[����] xa_len: xa���ֽ���
[����] ya�� ��Կ��Y����
[����] ya_len: ya���ֽ���
[���] z��32�ֽ�

����ֵ��
		��1���ڴ治��
		  0���ɹ�
*/

int sm2_keygen(unsigned char *wx, int *wxlen, unsigned char *wy, int *wylen, unsigned char *privkey, int *privkeylen);
/*
���ܣ�����SM2��˽Կ��
[���] wx��   ��Կ��X���꣬����32�ֽ���ǰ���0x00
[���] wxlen: wx���ֽ�����32
[���] wy��   ��Կ��Y���꣬����32�ֽ���ǰ���0x00
[���] wylen: wy���ֽ�����32
[���] privkey��˽Կ������32�ֽ���ǰ���0x00
[���] privkeylen�� privkey���ֽ�����32
����ֵ��
0��ʧ��
1���ɹ�

*/

int sm2_sign(unsigned char *hash, int hashlen, unsigned char *privkey, int privkeylen, unsigned char *cr, int *rlen, unsigned char *cs, int *slen);
/*
���ܣ�SM2ǩ��
[����] hash��    sm3_e()�Ľ��
[����] hashlen�� hash���ֽ�����ӦΪ32
[����] privkey�� ˽Կ
[����] privkeylen�� privkeylen���ֽ���

[���] cr��  ǩ������ĵ�һ���֣�����32�ֽ���ǰ���0x00��
[���] rlen��cr���ֽ�����32
[���] cs��  ǩ������ĵڶ����֣�����32�ֽ���ǰ���0x00��
[���] slen��cs���ֽ�����32
����ֵ��
0��ʧ��
1���ɹ�
*/

int  sm2_verify(unsigned char *hash, int hashlen, unsigned char  *cr, int rlen, unsigned char *cs, int slen, unsigned char *wx, int wxlen, unsigned char *wy, int wylen);
/*
���ܣ���֤SM2ǩ��
[����] hash��    sm3_e()�Ľ��
[����] hashlen�� hash���ֽ�����ӦΪ32
[����] cr��  ǩ������ĵ�һ����
[����] rlen��cr���ֽ���
[����] cs��  ǩ������ĵڶ����֡�
[����] slen��cs���ֽ���
[����] wx��   ��Կ��X����
[����] wxlen: wx���ֽ�����������32�ֽ�
[����] wy��   ��Կ��Y����
[����] wylen: wy���ֽ�����������32�ֽ�

����ֵ��
		0����֤ʧ��
		1����֤ͨ��
*/

int  sm2_encrypt(unsigned char *msg, int msglen, unsigned char *wx, int wxlen, unsigned char *wy, int wylen, unsigned char *outmsg);
/*
���ܣ���SM2��Կ�������ݡ����ܽ�����������ݶ�96�ֽڣ�
[����] msg     Ҫ���ܵ�����
[����] msglen��msg���ֽ���
[����] wx��    ��Կ��X����
[����] wxlen:  wx���ֽ�����������32�ֽ�
[����] wy��    ��Կ��Y����
[����] wylen:  wy���ֽ�����������32�ֽ�

[���] outmsg: ���ܽ�������������ݶ�96�ֽڣ���C1��64�ֽڣ���C3��32�ֽڣ�����ǰ��0x00

����ֵ��
		-1��        ����ʧ��
		msglen+96�� ���ܳɹ�
*/

int  sm2_decrypt(unsigned char *msg, int msglen, unsigned char *privkey, int privkeylen, unsigned char *outmsg);
/*
���ܣ���SM2˽Կ�������ݡ����ܽ��������������96�ֽڣ�
[����] msg     Ҫ���ܵ����ݣ���sm2_encrypt()���ܵĽ����������96�ֽڡ�
[����] msglen��msg���ֽ���
[����] privkey�� ˽Կ
[����] privkeylen�� privkeylen���ֽ���

[���] outmsg: ���ܽ����������������96�ֽڣ�

����ֵ��
		-1��        ����ʧ��
		msglen-96�� ���ܳɹ�
*/

int sm2_keyagreement_a(
    unsigned char *kxa, int kxalen,
    unsigned char *kya, int kyalen,
    unsigned char *xa, int xalen,
    unsigned char *ya, int yalen,
    unsigned char *private_a,   int private_a_len,
    unsigned char *xb, int xblen,
    unsigned char *yb, int yblen,
    unsigned char *ida, int idalen,
    unsigned char *idb, int idblen,
    unsigned char *kxb, int kxblen,
    unsigned char *kyb, int kyblen,
    unsigned char *private_a_tmp,  int private_a_tmp_len,
    unsigned int  keylen,
    unsigned char *keybuf,
    unsigned char *s1,
    unsigned char *sa
);
/*

���ܣ���ԿЭ�̵�A�����ô˺���Э�̳���Կkeybuf��
˵����
[����] (kxa, kya)��A������ʱ��Կ
[����] (xa, ya)��A���Ĺ�Կ
[����] private_a��A����˽Կ
[����] (xb, yb)��B���Ĺ�Կ
[����] ida��A�����û���ʶ
[����] idb��B�����û���ʶ
[����] (kxb, kyb)��B������ʱ��Կ
[����] private_a_tmp��A������ʱ˽Կ
[����] keylen��ҪԼ������Կ�ֽ���

[���] keybuf��Э����Կ���������
[���] s1��A������32�ֽڵ�HASHֵ��Ӧ����sb�����Ϊs1=NULL���������
[���] sa��A��������32�ֽڵ�HASHֵ��Ҫ���͸�B��������֤Э�̵���ȷ�ԡ����Ϊsa=NULL���������


����ֵ��0��ʧ��  1���ɹ�

*/


int sm2_keyagreement_b(
    unsigned char *kxb, int kxblen,
    unsigned char *kyb, int kyblen,
    unsigned char *xb, int xblen,
    unsigned char *yb, int yblen,
    unsigned char *private_b,   int private_b_len,
    unsigned char *xa, int xalen,
    unsigned char *ya, int yalen,
    unsigned char *idb, int idblen,
    unsigned char *ida, int idalen,
    unsigned char *kxa, int kxalen,
    unsigned char *kya, int kyalen,
    unsigned char *private_b_tmp,  int private_b_tmp_len,
    unsigned int  keylen,
    unsigned char *keybuf,
    unsigned char *s2,
    unsigned char *sb
);

/*

���ܣ���ԿЭ�̵�B�����ô˺���Э�̳���Կkeybuf��
˵����
[����] (kxb, kyb)��B������ʱ��Կ
[����] (xb, yb)��B���Ĺ�Կ
[����] private_b��B����˽Կ
[����] (xa, ya)��A���Ĺ�Կ
[����] idb��B�����û���ʶ
[����] ida��A�����û���ʶ
[����] (kxa, kya)��A������ʱ��Կ
[����] private_b_tmp��B������ʱ˽Կ
[����] keylen��ҪԼ������Կ�ֽ���

[���] keybuf��Э����Կ���������
[���] s2��B������32�ֽڵ�HASHֵ��Ӧ����sa�����Ϊs2=NULL���������
[���] sb��B��������32�ֽڵ�HASHֵ��Ҫ���͸�A��������֤Э�̵���ȷ�ԡ����Ϊsb=NULL���������


����ֵ��0��ʧ��  1���ɹ�

*/


int sm2_check_prikey(uint8_t prikey[32]);
int sm2_check_pubkey(uint8_t pubkey[64]);

#ifdef  __cplusplus
}
#endif

#endif
