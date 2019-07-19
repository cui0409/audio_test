//*****************************************音频采集***************************//
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

//不显示控制台
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

using namespace std;

HWAVEIN hWaveIn;  //输入设备
WAVEFORMATEX waveform; //采集音频的格式，结构体
BYTE *pBuffer1;//采集音频时的数据缓存
WAVEHDR wHdr1; //采集音频时包含数据缓存的结构体
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
		memcpy(&value, pcmdata + i, 2); //获取2个字节的大小（值）  
		sum += abs(value); //绝对值求和  
	}
	sum = sum / (size / 2); //求平均值（2个字节表示一个振幅，所以振幅个数为：size/2个）  
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
	waveform.wFormatTag = WAVE_FORMAT_PCM;//声音格式为PCM
	waveform.nSamplesPerSec = 8000;//采样率，16000次/秒
	waveform.wBitsPerSample = 16;//采样比特，16bits/次
	waveform.nChannels = 1;//采样声道数，2声道 // HDMI只能是1
	waveform.nAvgBytesPerSec = 16000;//每秒的数据率，就是每秒能采集多少字节的数据
	waveform.nBlockAlign = 2;//一个块的大小，采样bit的字节数乘以声道数
	waveform.cbSize = 0;//一般为0

	wait = CreateEvent(NULL, 0, 0, NULL);

	//波形输入设备的数目
	int n = waveInGetNumDevs();

	//使用waveInOpen函数开启音频采集，	当WAVE_MAPPER为-1，系统会自动选择一符合要求的设备
	//-1为默认，0为自带麦克风，1为采集卡麦克风	//注意：不设置设备号就得禁用自带输入麦克风，使用采集卡麦克风：Live Streaming Audio Device
	waveInOpen(&hWaveIn, 1, &waveform, (DWORD_PTR)wait, 0L, CALLBACK_EVENT);

	//HDMI单声道，AV双声道
	//建立两个数组（这里可以建立多个数组）用来缓冲音频数据
	DWORD bufsize = 1024 * 100;//每次开辟10k的缓存存储录音数据
	int i = 5;
	fopen_s(&pf, audio_name, "wb");
	while (i--)//录制5秒左右声音，结合音频解码和网络传输可以修改为实时录音播放的机制以实现对讲功能
	{
		pBuffer1 = new BYTE[bufsize];
		wHdr1.lpData = (LPSTR)pBuffer1;
		wHdr1.dwBufferLength = bufsize;
		wHdr1.dwBytesRecorded = 0;
		wHdr1.dwUser = 0;
		wHdr1.dwFlags = 0;
		wHdr1.dwLoops = 1;
		waveInPrepareHeader(hWaveIn, &wHdr1, sizeof(WAVEHDR));//准备一个波形数据块头用于录音
		waveInAddBuffer(hWaveIn, &wHdr1, sizeof(WAVEHDR));//指定波形数据块为录音输入缓存
		waveInStart(hWaveIn);//开始录音
		Sleep(1000);//等待声音录制1s
		waveInReset(hWaveIn);//停止录音
		fwrite(pBuffer1, 1, wHdr1.dwBytesRecorded, pf);
		delete pBuffer1;
		printf("%ds  ", i);
	}


	fclose(pf);

	waveInClose(hWaveIn);


	//先检测是否有音频
	ifstream inFile(audio_name, ios::in | ios::binary | ios::ate);
	long nFileSizeInBytes = inFile.tellg();

	if (nFileSizeInBytes == 0)
	{
		MessageBox(NULL, TEXT("未检测到音频"), TEXT("音频检测结果"), MB_DEFBUTTON1 | MB_DEFBUTTON2);
		return -1;
	}


	//音频处理
	//获取所有振幅之平均值 计算db(振幅最大值 2 ^ 16 - 1 = 65535 最大值是 96.32db)
	char* pcmdata = audio_name;
	size_t size = sizeof(WAVEHDR);
	int db = getPcmDB(audio_name, size);
	if(db > 0 && db < 86)
		MessageBox(NULL, TEXT("正常"), TEXT("音频检测结果"), MB_DEFBUTTON1 | MB_DEFBUTTON2);

	return 0;
}

