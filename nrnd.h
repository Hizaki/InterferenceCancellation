/* �{�b�N�X�~�����[�@:���K������Ԃ��֐�(��=m�C��=s) */

double nrnd(double m, double s)
{
	return s * sqrt(-2.0*log(rnd())) * cos(2.0*M_PI*rnd()) + m;
}