/* ボックスミュラー法:正規乱数を返す関数(μ=m，σ=s) */

double nrnd(double m, double s)
{
	return s * sqrt(-2.0*log(rnd())) * cos(2.0*M_PI*rnd()) + m;
}