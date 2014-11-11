/* ˆê—l—”(0`1)‚ğ•Ô‚·ŠÖ” */

double rnd(void)
{
	seed = seed * 1566083941UL + 1UL;	/* 0`(2^32-1)‚Ì—”‚ğ”­¶ UL:unsigned(•„†‚È‚µ) long*/
	return (double)seed / 4294967296.0;	/* 0`1‚É•ÏŠ·‚µ‚Ä•Ô‚· */
}