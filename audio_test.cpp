//*****************************************��Ƶ�ɼ�***************************//
#include "stdafx.h"
#include <stdio.h>  
#include <Windows.h>  
#pragma comment(lib, "winmm.lib") 

#include <string>

#include <cstdint>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>

//����ʾ����̨
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

using namespace std;

HWAVEIN hWaveIn;  //�����豸
WAVEFORMATEX waveform; //�ɼ���Ƶ�ĸ�ʽ���ṹ��
BYTE *pBuffer1;//�ɼ���Ƶʱ�����ݻ���
WAVEHDR wHdr1; //�ɼ���Ƶʱ�������ݻ���Ľṹ��
FILE *pf;

string get_time()
{
	SYSTEMTIME  st, lt;
	//GetSystemTime(&lt);
	GetLocalTime(&lt);

	char szResult[30] = "\0";

	sprintf_s(szResult, 30, "%d-%d-%d-%d-%d-%d-%d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond, lt.wMilliseconds);

	return szResult;
}

int getPcmDB(char *pcmdata, size_t size) {

	int db = 0;
	short int value = 0;
	double sum = 0;
	for (int i = 0; i < size; i += 2)
	{
		memcpy(&value, pcmdata + i, 2); //��ȡ2���ֽڵĴ�С��ֵ��  
		sum += abs(value); //����ֵ���  
	}
	sum = sum / (size / 2); //��ƽ��ֵ��2���ֽڱ�ʾһ������������������Ϊ��size/2����  
	if (sum > 0)
	{
		db = (int)(20.0*log10(sum));
	}
	return db;
}

int main()
{
	char audio_name[100];
	string time;
	time = get_time();
	sprintf_s(audio_name, "%s%s%s", "F:\\testaudio\\audio", time.c_str(), ".pcm");


	HANDLE     wait;
	waveform.wFormatTag = WAVE_FORMAT_PCM;//������ʽΪPCM
	waveform.nSamplesPerSec = 8000;//�����ʣ�16000��/��
	waveform.wBitsPerSample = 16;//�������أ�16bits/��
	waveform.nChannels = 1;//������������2���� // HDMIֻ����1
	waveform.nAvgBytesPerSec = 16000;//ÿ��������ʣ�����ÿ���ܲɼ������ֽڵ�����
	waveform.nBlockAlign = 2;//һ����Ĵ�С������bit���ֽ�������������
	waveform.cbSize = 0;//һ��Ϊ0

	wait = CreateEvent(NULL, 0, 0, NULL);

	//���������豸����Ŀ
	int n = waveInGetNumDevs();

	//ʹ��waveInOpen����������Ƶ�ɼ���	��WAVE_MAPPERΪ-1��ϵͳ���Զ�ѡ��һ����Ҫ����豸
	//-1ΪĬ�ϣ�0Ϊ�Դ���˷磬1Ϊ�ɼ�����˷�	//ע�⣺�������豸�ž͵ý����Դ�������˷磬ʹ�òɼ�����˷磺Live Streaming Audio Device
	waveInOpen(&hWaveIn, 1, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);

	//HDMI��������AV˫����
	//�����������飨������Խ���������飩����������Ƶ����
	DWORD bufsize = 1024 * 100;//ÿ�ο���10k�Ļ���洢¼������
	int i = 5;
	fopen_s(&pf, audio_name, "wb");
	while (i--)//¼��5�����������������Ƶ��������紫������޸�Ϊʵʱ¼�����ŵĻ�����ʵ�ֶԽ�����
	{
		pBuffer1 = new BYTE[bufsize];
		wHdr1.lpData = (LPSTR)pBuffer1;
		wHdr1.dwBufferLength = bufsize;
		wHdr1.dwBytesRecorded = 0;
		wHdr1.dwUser = 0;
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1;
		waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//׼��һ���������ݿ�ͷ����¼��
		waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//ָ���������ݿ�Ϊ¼�����뻺��
		waveInStart(hWaveIn);//��ʼ¼��
		Sleep(1000);//�ȴ�����¼��1s
		waveInReset(hWaveIn);//ֹͣ¼��
		fwrite(pBuffer1, 1, wHdr1.dwBytesRecorded, pf);
		delete pBuffer1;
		printf("%ds  ", i);
	}


	fclose(pf);

	waveInClose(hWaveIn);


	//�ȼ���Ƿ�����Ƶ
	ifstream inFile(audio_name, ios::in | ios::binary | ios::ate);
	long nFileSizeInBytes = inFile.tellg();

	if (nFileSizeInBytes == 0)
	{
		MessageBox(NULL, TEXT("δ��⵽��Ƶ"), TEXT("��Ƶ�����"), MB_DEFBUTTON1 | MB_DEFBUTTON2);
		return -1;
	}


	//��Ƶ����
	//��ȡ�������֮ƽ��ֵ ����db(������ֵ 2 ^ 16 - 1 = 65535 ���ֵ�� 96.32db)
	char* pcmdata = audio_name;
	size_t size = sizeof(WAVEHDR);
	int db = getPcmDB(audio_name, size);
	if(db > 0 && db < 86)
		MessageBox(NULL, TEXT("����"), TEXT("��Ƶ�����"), MB_DEFBUTTON1 | MB_DEFBUTTON2);

	return 0;
}

