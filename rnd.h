/* ��l����(0�`1)��Ԃ��֐� */

double rnd(void)
{
	seed = seed * 1566083941UL + 1UL;	/* 0�`(2^32-1)�̗����𔭐� UL:unsigned(�����Ȃ�) long*/
	return (double)seed / 4294967296.0;	/* 0�`1�ɕϊ����ĕԂ� */
}