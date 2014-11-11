/* 一様乱数(0〜1)を返す関数 */

double rnd(void)
{
	seed = seed * 1566083941UL + 1UL;	/* 0〜(2^32-1)の乱数を発生 UL:unsigned(符号なし) long*/
	return (double)seed / 4294967296.0;	/* 0〜1に変換して返す */
}