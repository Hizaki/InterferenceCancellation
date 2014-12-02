/*
	直交符号を用いたDS/CDMA通信方式+他局再生による他局間干渉除去のBER特性を導出するプログラム
	簡単化のために自局入力データは全て1とする．

	他局信号再生が行えるのは基地局のみである．
	このプログラムは基地局でのBER特性を導出する．
*/

// ヘッダファイルのインクルード
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define N 100000		// 試行回数
#define P 11				// Eb/Noの点の数
#define NUM 0			// 巡回シフト回数
#define CODE_LENGTH 32	// 符号長
#define SIR -10			// 信号対干渉電力比 
#define SIRVTH -10		// スレッシュホールドレベルの設定

// 乱数の初期値設定
static unsigned long seed = 1;

//ユーザ作成ヘッダファイル
#include "rnd.h"		// 一様乱数生成関数
#include "nrnd.h"		// 正規乱数を返す関数

void MakeMyData(double* InputData, double* pn, double* OutputData);
void MakeOtherData(double* InputData, double* pn, double* OutputData);
void RegenerationOtherData(double* InputData, double* pn, double* OutputData);
void Demodulation(double* InputData, double* pn, double* OutputData);
void Demodulation2(double* InputData, double* pn, double* OutputData);

void main() {
	double* MyData = (double*)calloc(CODE_LENGTH, sizeof(double));		// 自局データ
	double* OtherData = (double*)calloc(CODE_LENGTH, sizeof(double));	// 他局データ
	double MyPn[CODE_LENGTH] =  {-1,1,1,1,-1,1,1,-1,-1,1,1,1,1,-1,-1,-1,1,-1,1,-1,-1,1,-1,-1,1,-1,-1,1,1,-1,-1,1};
	double OtherPn[CODE_LENGTH] = {1,1,-1,-1,-1,-1,1,-1,-1,1,-1,-1,1,-1,1,-1,1,1,-1,1,1,-1,-1,1,-1,1,-1,1,1,1,-1,1};

	double* TransmitMyData =  (double*)calloc(CODE_LENGTH, sizeof(double)); 	// 自局送信データ
	double* TransmitOtherData = (double*)calloc(CODE_LENGTH, sizeof(double)); 	// 他局送信データ

	double* ReceiveData = (double*)calloc(CODE_LENGTH, sizeof(double));			// 受信信号データ
	double* SubtractData = (double*)calloc(CODE_LENGTH, sizeof(double));		// 減算データ

	double* RegenerateMyData = (double*)calloc(CODE_LENGTH, sizeof(double));		// 自局再生による自局復調データ
	double* RegenerateOtherData = (double*)calloc(CODE_LENGTH, sizeof(double));		// 他局再生による他局復調データ

	double* OutputData = (double*)calloc(CODE_LENGTH, sizeof(double));			// 出力信号データ

	//double* MySuccessData = (double*)calloc(CODE_LENGTH, sizeof(double));			// [自局]減算成功データ
	//double* OtherSuccessData = (double*)calloc(CODE_LENGTH, sizeof(double));		// [他局]減算成功データ

	int i, j, k;
	unsigned long n, nn;
	double en, pebs;
	double end[P] = {0.0, 5.0, 10.0, 15.0, 20.0, 25.0, 30.0, 35.0, 40.0, 45.0, 50.0};
	double en2 = pow(10.0, SIR/10.0);

	double* flag = (double*)calloc(CODE_LENGTH, sizeof(double));		// 干渉除去が成功しているかのフラグ
	
	seed = (unsigned long)time(NULL);

	//自局データの作成
	for(i=0 ; i<CODE_LENGTH ; i++){
		MyData[i] = 1.0;
	}

	//BER特性の導出
	for(i=0 ; i<P ; i++){
		en = pow(10.0, end[i]/10.0);

		for(j=n=0 ; j<N; j++){

			//自局送信データの作成
			MakeMyData(MyData, MyPn, TransmitMyData);

			for(k=0 ; k<CODE_LENGTH ; k++){	
				if(rnd()>0.5){
					OtherData[k] = 1.0;
				}else{
					OtherData[k] = -1.0;
				}
			}

			MakeOtherData(OtherData, OtherPn, TransmitOtherData);

			//受信信号の作成
			for(k=0 ; k<CODE_LENGTH ; k++){
				ReceiveData[k] = TransmitMyData[k] + (TransmitOtherData[k] / sqrt(en2)) + nrnd(0, sqrt((CODE_LENGTH)/(2.0*en)));
			}

			//他局信号の再生
			RegenerationOtherData(ReceiveData, OtherPn, RegenerateOtherData);

			for(k=0 ; k<CODE_LENGTH ; k++){
				SubtractData[k] = ReceiveData[k] - (RegenerateOtherData[k] / sqrt(en2));
			}

			//自局1回目判定
			Demodulation(SubtractData, MyPn, OutputData);

			//自局信号の再生
			MakeMyData(OutputData, MyPn, RegenerateMyData);

			for(k=0 ; k<CODE_LENGTH ; k++){
				SubtractData[k] = ReceiveData[k] - RegenerateMyData[k];
			}

			//他局1回目判定
			Demodulation2(SubtractData, OtherPn, OutputData);

			/* 2回目 */

			//他局信号の再生
			MakeOtherData(OutputData, OtherPn, RegenerateOtherData);

			//3値判定
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(RegenerateOtherData[k] > 1){
					SubtractData[k] = ReceiveData[k] - (RegenerateOtherData[k]/sqrt(en2));
					flag[k] = 0.0;
				}else if(RegenerateOtherData[k] < -1){
					SubtractData[k] = ReceiveData[k] - (RegenerateOtherData[k]/sqrt(en2));
					flag[k] = 0.0;
				}else{
					SubtractData[k] = ReceiveData[k];
					flag[k] = 1.0;
				}
			}

			//flag=0の部分に関しては2倍引くか足す
			for(k=0 ; k<CODE_LENGTH ; k++){
				if(flag[k] == 0){
					if(SubtractData[k] >= 1/pow(10.0, SIRVTH/10.0)){
						SubtractData[k] -= 2 * 1/sqrt(en2);
					}else if(SubtractData[k] <= -1/pow(10.0, SIRVTH/10.0)){
						SubtractData[k] += 2 * 1/sqrt(en2);
					}else{

					}
				}else{

				}
			}

			//自局2回目判定
			Demodulation(SubtractData, MyPn, OutputData);

			for(k=0; k<CODE_LENGTH ; k++){
				if(OutputData[k] != MyData[k]){
					n++;
				}
			}
		}

		pebs = (double)n / (double)(N*32);

		printf("Error = %d\n", n);
		printf("All = %d\n", N*32);
		printf("%9.6f\t%15.6e\n\n", end[i], pebs);
	}

	free(MyData);
	free(OtherData);
	free(TransmitMyData);
	free(TransmitOtherData);
	free(ReceiveData);
	free(SubtractData);
	free(RegenerateMyData);
	free(RegenerateOtherData);
	free(OutputData);
}

void MakeMyData(double* InputData, double* pn, double* OutputData)
{
	int i, j;

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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

	double WalshMultiplexingData[CODE_LENGTH][CODE_LENGTH];

	for(i=0 ; i<CODE_LENGTH ; i++){
		OutputData[i] = 0.0;
	}

	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			WalshMultiplexingData[i][j] = InputData[i] * pn[j] * WalshCode[i][j];
		}
	}

	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			OutputData[j] += WalshMultiplexingData[i][j];
		}
	}

	/*for(i=0 ; i<CODE_LENGTH ; i++){
		printf("%3d\t=%lf\n", i, OutputData[i]);
	}*/
}

void MakeOtherData(double* InputData, double* pn, double* OutputData)
{
	int i, j;
	double shift[CODE_LENGTH][CODE_LENGTH];		//巡回シフト直交Gold系列の作成

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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

	double WalshMultiplexingData[CODE_LENGTH][CODE_LENGTH];

	//巡回シフト直交Gold系列の作成
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			shift[i][j] = pn[(i+j)%(CODE_LENGTH)];
			//printf("%.f", shift[i][j]);
		}
		//printf("\n");
	}

	for(i=0 ; i<CODE_LENGTH ; i++){
		OutputData[i] = 0.0;
	}

	for(i=0 ; i<CODE_LENGTH; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			WalshMultiplexingData[i][j] = InputData[i] * shift[NUM][j] * WalshCode[i][j];
		}
	}

	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			OutputData[j] += WalshMultiplexingData[i][j];
		}
	}

	//for(i=0 ; i<CODE_LENGTH ; i++){
	//	printf("%3d\t=%lf\n", i, OutputData[i]);
	//}
}

void RegenerationOtherData(double* InputData, double* pn, double* OutputData)
{
	int i, j;

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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

	double shift[CODE_LENGTH][CODE_LENGTH];
	double DespreadData[CODE_LENGTH][CODE_LENGTH];
	double IntegratedData[CODE_LENGTH];
	double WalshMultiplexingData[CODE_LENGTH][CODE_LENGTH];
	double DecideData[CODE_LENGTH];

	//巡回シフト直交Gold系列の作成
	for(i=0 ; i<CODE_LENGTH; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			shift[i][j] = pn[(i+j)%(CODE_LENGTH)];
		}
	}

	//初期化
	for(i=0 ; i<CODE_LENGTH; i++){
		IntegratedData[i] = 0.0;
		OutputData[i] = 0.0;
	}

	//データの分離と逆拡散処理
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			DespreadData[i][j] = InputData[j] * shift[NUM][j] * WalshCode[i][j];
		}
	}

	//データを積分
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			IntegratedData[i] += DespreadData[i][j];
		}
	}

	//スレッシュホールドレベル0でデータの判定
	for(i=0 ; i<CODE_LENGTH; i++){
		if(IntegratedData[i] > 0.0){
			DecideData[i] = 1.0;
		}else if(IntegratedData[i] < 0.0){
			DecideData[i] = -1.0;
		}else{
			if(rnd()>0.5){
				DecideData[i] = 1.0;
			}else{
				DecideData[i] = -1.0;
			}
		}
	}

	//Walsh符号とPN系列をデータに乗算
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			WalshMultiplexingData[i][j] = DecideData[i] * shift[NUM][j] * WalshCode[i][j];
		}
	}

	// 同じビット列を加算
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			OutputData[j] += WalshMultiplexingData[i][j];
		}
	}
}

void Demodulation(double* InputData, double* pn, double* OutputData)
{
	int i, j;

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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

	double IntegratedData[CODE_LENGTH];
	double DespreadData[CODE_LENGTH][CODE_LENGTH];

	for(i=0 ; i<CODE_LENGTH ; i++){
		IntegratedData[i] = 0.0;
		OutputData[i] = 0.0;
	}

	//データの分離と逆拡散処理
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			DespreadData[i][j] = InputData[j] * pn[j] * WalshCode[i][j];
		}
	}

	//データの積分
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH; j++){
			IntegratedData[i] += DespreadData[i][j];
		}
	}

	//スレッシュホールドレベル0でデータの判定
	for(i=0 ; i<CODE_LENGTH; i++){
		if(IntegratedData[i] > 0.0){
			OutputData[i] = 1.0;
		}else if(IntegratedData[i] < 0.0){
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

void Demodulation2(double* InputData, double* pn, double* OutputData)
{
	int i, j;

	double WalshCode[CODE_LENGTH][CODE_LENGTH] = {
		{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
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

	double shift[CODE_LENGTH][CODE_LENGTH];
	double IntegratedData[CODE_LENGTH];
	double DespreadData[CODE_LENGTH][CODE_LENGTH];

	for(i=0 ; i<CODE_LENGTH ; i++){
		IntegratedData[i] = 0.0;
		OutputData[i] = 0.0;
	}

	//巡回シフト直交Gold系列の作成
	for(i=0 ; i<CODE_LENGTH; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			shift[i][j] = pn[(i+j)%(CODE_LENGTH)];
		}
	}

	//データの分離と逆拡散処理
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH ; j++){
			DespreadData[i][j] = InputData[j] * shift[NUM][j] * WalshCode[i][j];
		}
	}

	//データの積分
	for(i=0 ; i<CODE_LENGTH ; i++){
		for(j=0 ; j<CODE_LENGTH; j++){
			IntegratedData[i] += DespreadData[i][j];
		}
	}

	//スレッシュホールドレベル0でデータの判定
	for(i=0 ; i<CODE_LENGTH; i++){
		if(IntegratedData[i] > 0.0){
			OutputData[i] = 1.0;
		}else if(IntegratedData[i] < 0.0){
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