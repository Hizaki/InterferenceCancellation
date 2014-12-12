/*

	OrthDS.c

	’¼Œð•„†‚ð—p‚¢‚½DS/CDMA’ÊM•ûŽ®‚ÌBER“Á«ƒVƒ~ƒ…ƒŒ[ƒVƒ‡ƒ“ƒvƒƒOƒ‰ƒ€

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define N 100000
#define P 12
#define CODE_LENGTH 32
#define SIR -9
#define NUM 0
#define VTH 8.0

static unsigned long seed = 1;

#include "rnd.h"
#include "nrnd.h"

void MakeMyData(double* InputData, double* pn, double* OutputData);
void MakeOtherData(double* InputData, double* pn, double* OutputData);
void MyDataDemodulation(double* InputData, double* pn, double* OutputData);
void OtherDataDemodulation(double* InputData, double* pn, double* OutputData);
void FlagForCancellation(double* InputData, double* FlagData);
void DataDecision(double* InputData, double* OutputData);

void main(){

	double* MyData = (double*)calloc(CODE_LENGTH, sizeof(double));
	double* OtherData = (double*)calloc(CODE_LENGTH, sizeof(double));

	double MyPN[CODE_LENGTH] =  {-1,1,1,1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,1,-1,1,-1,-1,1,-1,-1,1,-1,-1,1,1,-1,-1,1};
	double OtherPN[CODE_LENGTH] = {1,1,-1,-1,-1,-1,1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,1,-1,-1,1,-1,1,-1,1,1,1,-1,1};

	double* TransmitMyData =  (double*)calloc(CODE_LENGTH, sizeof(double));
	double* TransmitOtherData = (double*)calloc(CODE_LENGTH, sizeof(double));

	double* ReceiveData = (double*)calloc(CODE_LENGTH, sizeof(double));
	double* OutputData = (double*)calloc(CODE_LENGTH, sizeof(double));
	double* SubtractData = (double*)calloc(CODE_LENGTH, sizeof(double));

	double* SubtractOfUser1 = (double*)calloc(CODE_LENGTH, sizeof(double));
	double* SubtractOfUser2 = (double*)calloc(CODE_LENGTH, sizeof(double));

	//double* Flag = (double*)calloc(CODE_LENGTH, sizeof(double));

	double* DecideData = (double*)calloc(CODE_LENGTH, sizeof(double));

	int i, j, k;

	unsigned long MyBER00, OtherBER00;
	unsigned long MyBER01, OtherBER01;
	unsigned long MyBER02, OtherBER02;
	unsigned long MyBER03, OtherBER03;
	unsigned long MyBER04, OtherBER04;
	unsigned long MyBER05, OtherBER05;

	double en;

	double pebs_M00, pebs_M01, pebs_M02, pebs_M03, pebs_M04, pebs_M05;
	double pebs_O00, pebs_O01, pebs_O02, pebs_O03, pebs_O04, pebs_O05;

	double end[P] = {0.0, 5.0, 6.8, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0, 50.0};
	double en2 = pow(10.0, SIR/10.0);

	double ThresholdData[P][11]={{8.0, 8.0, 7.0, 5.5, 6.0, 5.5, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {6.0, 6.5, 4.0, 3.5, 3.0, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {6.5, 5.0, 4.0, 3.5, 3.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {6.0, 4.5, 4.0, 3.0, 3.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {5.5, 4.0, 2.5, 2.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {5.0, 3.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {5.0, 4.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {6.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
								 {7.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0}};


	FILE *fp;

	fp = fopen("BER_Results_-9dB_vth80.csv", "w");

	seed = (unsigned long)time(NULL);

	printf("Eb/No\tMyData00\tMyData01\tMyData02\tOtherData00\tOtherData01\tOtherData02\n");
	fprintf(fp, "Eb/No, MyData00, MyData01, MyData02, MyData03, MyData04, MyData05, OtherData00, OtherData01, OtherData02, OtherData03, OtherData04, OtherData05\n");

	for(i=0 ; i<P ; i++){
		en = pow(10.0, end[i]/10.0);

		for(j=MyBER00=OtherBER00=MyBER01=OtherBER01=MyBER02=OtherBER02=MyBER03=OtherBER03=MyBER04=OtherBER04=MyBER05=OtherBER05=0 ; j<N ; j++){

			//Ž©‹Çƒf[ƒ^‚Ì¶¬(ŠÈ’P‰»‚Ì‚½‚ßall1)
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(rnd()>0.5){
					MyData[k] = 1.0;
				}else{
					MyData[k] = 1.0;
				}
			}

			//Ž©‹Çƒf[ƒ^•Ï’²
			MakeMyData(MyData, MyPN, TransmitMyData);

			//‘¼‹Çƒf[ƒ^‚Ì¶¬
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(rnd()>0.5){
						OtherData[k] = 1.0;
					}else{
						OtherData[k] = -1.0;
					}
			}

			//‘¼‹Çƒf[ƒ^•Ï’²
			MakeOtherData(OtherData, OtherPN, TransmitOtherData);

			//ŽóMM†‚Ìì¬
			for(k=0 ; k<CODE_LENGTH ; k++){
				ReceiveData[k] = TransmitMyData[k] + (TransmitOtherData[k] / sqrt(en2)) + nrnd(0, sqrt((CODE_LENGTH)/(2.0*en)));
			}

			//自局データ(干渉除去なし)BER特性導出	
			MyDataDemodulation(ReceiveData, MyPN, OutputData);
			DataDecision(OutputData, DecideData);

			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != MyData[k]){
					MyBER00++;
				}
			}

			//他局データ(干渉除去なし)BER特性導出
			OtherDataDemodulation(ReceiveData, OtherPN, OutputData);
			DataDecision(OutputData, DecideData);

			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != OtherData[k]){
					OtherBER00++;
				}
			}

			MakeOtherData(DecideData, OtherPN, OutputData);
			
			//Š±Âœ‹Ž
			for(k=0 ; k<CODE_LENGTH ; k++){
				SubtractData[k] = ReceiveData[k] - (OutputData[k]/sqrt(en2)) ;

				SubtractOfUser2[k] = SubtractData[k];
			}

			//Ž©‹ÇM†‚Ì•œ’²
			MyDataDemodulation(SubtractData, MyPN, OutputData);
			//”»’è
			DataDecision(OutputData, DecideData);

			//自局データ(干渉除去1回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != MyData[k]){
					MyBER01++;
				}
			}

			//自局データ生成
			MakeMyData(DecideData, MyPN, OutputData);

			//干渉除去
			for(k=0 ; k<CODE_LENGTH ; k++){
				SubtractData[k] = ReceiveData[k] - OutputData[k];

				SubtractOfUser1[k] = SubtractData[k];
			}

			OtherDataDemodulation(SubtractData, OtherPN, OutputData);
			DataDecision(OutputData, DecideData);

			//他局データ(干渉除去1回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != OtherData[k]){
					OtherBER01++;
				}
			}

			MakeOtherData(DecideData, OtherPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(ThresholdData[P][SIR] != 0.0){
					if(SubtractOfUser2[k] >= ThresholdData[P][SIR]){
						SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
					}else if(SubtractOfUser2[k] <= -ThresholdData[P][SIR]){
						SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
					}else{
						SubtractData[k] = SubtractOfUser2[k];
					}
				}else{
					if(SubtractOfUser2[k] >= 1/sqrt(en2)){
						SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
					}else if(SubtractOfUser2[k] <= -1/sqrt(en2)){
						SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
					}else{
						SubtractData[k] = SubtractOfUser2[k];
					}
				}
			}

			//Ž©‹ÇM†‚Ì•œ’²
			MyDataDemodulation(SubtractData, MyPN, OutputData);
			//”»’è
			DataDecision(OutputData, DecideData);

			//自局データ(干渉除去2回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != MyData[k]){
					MyBER02++;
				}
			}

			MakeMyData(DecideData, MyPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(SubtractOfUser2[k] >= VTH){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else if(SubtractOfUser1[k] <= -VTH){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else{
					SubtractData[k] = SubtractOfUser1[k];
				}
			}

			OtherDataDemodulation(SubtractData, OtherPN, OutputData);
			DataDecision(OutputData, DecideData);

			//他局データ(干渉除去2回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != OtherData[k]){
					OtherBER02++;
				}
			}

/*			MakeOtherData(DecideData, OtherPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(SubtractOfUser2[k] >= 1/sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
				}else if(SubtractOfUser2[k] <= -sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
				}else{
					SubtractData[k] = SubtractOfUser2[k];
				}
			}

			//Ž©‹ÇM†‚Ì•œ’²
			MyDataDemodulation(SubtractData, MyPN, OutputData);
			//”»’è
			DataDecision(OutputData, DecideData);

			//自局データ(干渉除去3回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != MyData[k]){
					MyBER03++;
				}
			}

/*			MakeMyData(DecideData, MyPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(SubtractOfUser2[k] >= 1/sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else if(SubtractOfUser1[k] <= -sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else{
					SubtractData[k] = SubtractOfUser1[k];
				}
			}

			OtherDataDemodulation(SubtractData, OtherPN, OutputData);
			DataDecision(OutputData, DecideData);

			//他局データ(干渉除去3回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != OtherData[k]){
					OtherBER03++;
				}
			}

			/* 4回目 */

/*			MakeOtherData(DecideData, OtherPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(SubtractOfUser2[k] >= 1/sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
				}else if(SubtractOfUser2[k] <= -sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
				}else{
					SubtractData[k] = SubtractOfUser2[k];
				}
			}

			//Ž©‹ÇM†‚Ì•œ’²
			MyDataDemodulation(SubtractData, MyPN, OutputData);
			//”»’è
			DataDecision(OutputData, DecideData);

			//自局データ(干渉除去4回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != MyData[k]){
					MyBER04++;
				}
			}

			MakeMyData(DecideData, MyPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(SubtractOfUser2[k] >= 1/sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else if(SubtractOfUser1[k] <= -sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else{
					SubtractData[k] = SubtractOfUser1[k];
				}
			}

			OtherDataDemodulation(SubtractData, OtherPN, OutputData);
			DataDecision(OutputData, DecideData);

			//他局データ(干渉除去4回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != OtherData[k]){
					OtherBER04++;
				}
			}

			/* 5回目 */

/*			MakeOtherData(DecideData, OtherPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(SubtractOfUser2[k] >= 1/sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
				}else if(SubtractOfUser2[k] <= -sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k] / sqrt(en2);
				}else{
					SubtractData[k] = SubtractOfUser2[k];
				}
			}

			//Ž©‹ÇM†‚Ì•œ’²
			MyDataDemodulation(SubtractData, MyPN, OutputData);
			//”»’è
			DataDecision(OutputData, DecideData);

			//自局データ(干渉除去5回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != MyData[k]){
					MyBER05++;
				}
			}

			MakeMyData(DecideData, MyPN, OutputData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH; k++){
				if(SubtractOfUser2[k] >= 1/sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else if(SubtractOfUser1[k] <= -sqrt(en2)){
					SubtractData[k] = ReceiveData[k] - OutputData[k];
				}else{
					SubtractData[k] = SubtractOfUser1[k];
				}
			}

			OtherDataDemodulation(SubtractData, OtherPN, OutputData);
			DataDecision(OutputData, DecideData);

			//他局データ(干渉除去2回目)BER特性導出
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(DecideData[k] != OtherData[k]){
					OtherBER05++;
				}
			}
*/


		}

		pebs_M00 = (double)MyBER00 / (double)(N*32);	pebs_O00 = (double)OtherBER00 / (double)(N*32);
		pebs_M01 = (double)MyBER01 / (double)(N*32);	pebs_O01 = (double)OtherBER01 / (double)(N*32);
		pebs_M02 = (double)MyBER02 / (double)(N*32);	pebs_O02 = (double)OtherBER02 / (double)(N*32);
		pebs_M03 = (double)MyBER03 / (double)(N*32);	pebs_O03 = (double)OtherBER03 / (double)(N*32);
		pebs_M04 = (double)MyBER04 / (double)(N*32);	pebs_O04 = (double)OtherBER04 / (double)(N*32);
		pebs_M05 = (double)MyBER05 / (double)(N*32);	pebs_O05 = (double)OtherBER05 / (double)(N*32);

		printf("%.1f\t%5.6e\t%5.6e\t%5.6e\t%5.6e\t%5.6e\t%5.6e\n\n", end[i], pebs_M00, pebs_M01, pebs_M02, pebs_O00, pebs_O01, pebs_O02);
		fprintf(fp, "%.1f, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e, %5.6e\n", end[i], pebs_M00, pebs_M01, pebs_M02, pebs_M03, pebs_M04, pebs_M05, pebs_O00, pebs_O01, pebs_O02, pebs_O03, pebs_O04, pebs_O05);	
	}

	fclose(fp);
}	

void MakeMyData(double* InputData, double* pn, double* OutputData)
{
	int i, j;

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1}
	};

	//‰Šú‰»
	for(i=0 ; i<CODE_LENGTH ; i++){
		OutputData[i] = 0.0;
	}

	//‘—Mƒf[ƒ^‚Ìì¬
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			OutputData[j] += WalshCode[i][j] * pn[j] * InputData[i];
		}
	}


}

void MakeOtherData(double* InputData, double* pn, double* OutputData)
{
	int i, j;
	double shift[CODE_LENGTH][CODE_LENGTH];		//„‰ñƒVƒtƒg’¼ŒðGoldŒn—ñ‚Ìì¬

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1}
	};

	//„‰ñƒVƒtƒg’¼ŒðGoldŒn—ñ‚Ìì¬
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			shift[i][j] = pn[(i+j)%(CODE_LENGTH)];
		}
	}

	//‰Šú‰»
	for(i=0 ; i<CODE_LENGTH ; i++){
		OutputData[i] = 0.0;
	}

	//‘—Mƒf[ƒ^‚Ìì¬
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			OutputData[j] += WalshCode[i][j] * pn[j] * InputData[i];
		}
	}
}	

void MyDataDemodulation(double* InputData, double* pn, double* OutputData)
{
	int i, j;

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1}
	};

	double WhipData[CODE_LENGTH][CODE_LENGTH];
	double IntegralData[CODE_LENGTH];

	//‰Šú‰»
	for(i=0 ; i<CODE_LENGTH ; i++){
		IntegralData[i] = 0.0;
		OutputData[i] = 0.0;
	}

	//ŽóMM†‚ÉWalsh•„†‚ÆPNŒn—ñ‚ÌŠes‚ðæŽZ
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			WhipData[i][j] = InputData[j] * WalshCode[i][j] * pn[j];
		}
	}
	
	//Žæ‚èo‚µ‚½ƒf[ƒ^‚ðÏ•ª
	for(i=0 ; i<CODE_LENGTH; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			IntegralData[i] += WhipData[i][j];
		}
	}

	//Ï•ª‚µ‚½ƒf[ƒ^‚ð•„†’·‚ÅŠ„‚é
	for(i=0 ; i<CODE_LENGTH ; i++){
		OutputData[i] = IntegralData[i] / CODE_LENGTH;
	}	
}

void OtherDataDemodulation(double* InputData, double* pn, double* OutputData)
{
	int i, j;
	double shift[CODE_LENGTH][CODE_LENGTH];		//„‰ñƒVƒtƒg’¼ŒðGoldŒn—ñ‚Ìì¬

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1},
		{1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1},
		{1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1},
		{1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1},
		{1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1},
		{1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1},
		{1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1},
		{1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1},
		{1,1,1,1,1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,1,1,1,1},
		{1,-1,1,-1,1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1},
		{1,1,-1,-1,1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1},
		{1,-1,-1,1,1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1},
		{1,1,1,1,-1,-1,-1,-1,-1,-1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,1,1,1,1,-1,-1,-1,-1},
		{1,-1,1,-1,-1,1,-1,1,-1,1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,1,-1,1,-1,-1,1,-1,1},
		{1,1,-1,-1,-1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,-1,1,1,1,1,-1,-1,1,1,-1,-1,-1,-1,1,1},
		{1,-1,-1,1,-1,1,1,-1,-1,1,1,-1,1,-1,-1,1,-1,1,1,-1,1,-1,-1,1,1,-1,-1,1,-1,1,1,-1}
	};

	double WhipData[CODE_LENGTH][CODE_LENGTH];
	double IntegralData[CODE_LENGTH];

	//„‰ñƒVƒtƒg’¼ŒðGoldŒn—ñ‚Ìì¬
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			shift[i][j] = pn[(i+j)%(CODE_LENGTH)];
		}
	}

	//‰Šú‰»
	for(i=0 ; i<CODE_LENGTH ; i++){
		IntegralData[i] = 0.0;
	}

	//ŽóMM†‚ÉWalsh•„†‚ÆPNŒn—ñ‚ÌŠes‚ðæŽZ
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			WhipData[i][j] = InputData[j] * WalshCode[i][j] * shift[NUM][j];
		}
	}
	
	//Žæ‚èo‚µ‚½ƒf[ƒ^‚ðÏ•ª
	for(i=0 ; i<CODE_LENGTH; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			IntegralData[i] += WhipData[i][j];
		}
	}

	//Ï•ª‚µ‚½ƒf[ƒ^‚ð•„†’·‚ÅŠ„‚é
	for(i=0 ; i<CODE_LENGTH ; i++){
		OutputData[i] = IntegralData[i] / CODE_LENGTH;
	}	
}

void FlagForCancellation(double* InputData, double* FlagData)
{
	int i;

	for(i=0 ; i<CODE_LENGTH ; i++){
		InputData[i] = InputData[i] / CODE_LENGTH;

		if(InputData[i] > 1.0){
			FlagData[i] = 1.0;
		}else if(InputData[i] < -1.0){
			FlagData[i] = -1.0;
		}else{
			FlagData[i] = 0.0;
		}

	}

}

void DataDecision(double* InputData, double* OutputData)
{
	int i;

	for(i=0 ; i<CODE_LENGTH ; i++){
		if(InputData[i] > 0.0){
			OutputData[i] = 1.0;
		}else if(InputData[i] < 0.0){
			OutputData[i] = -1.0;
		}else{
			if(rnd()>0.5){
					OutputData[i] = 1.0;
				}else{
					OutputData[i] = -1.0;
				}
		}
	}
}
