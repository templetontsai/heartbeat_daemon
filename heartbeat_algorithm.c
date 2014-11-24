#include <fcntl.h>
#include <errno.h>
#include <math.h>
#include <poll.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <sys/select.h>
#include <cutils/log.h>
#include <utils/Log.h>
#include "heartbeat_algorithm.h"
#define LOG_TAG "HeartBeatAlgo"

int     m_nzValues_RAW[POINT_COUNT];  
int     m_nzValues_x[POINT_COUNT];  
float   m_nzValues_y[POINT_COUNT];  
float   m_nzValues_z[POINT_COUNT];  
float   m_nzValues_a[POINT_COUNT];  
float   m_nzValues_p[POINT_COUNT];  
float   m_nzValues_kg[POINT_COUNT]; 
//int     m_nzValues_Rate[POINT_COUNT];
float     m_nzValues_Rate[RATE_MA_NUM];
//float   HBM_Perist_Buf[POINT_COUNT] = {0};

float   m_uiFilter_B0 = 0.0478; 
float   m_uiFilter_B1 = 0; 
float   m_uiFilter_B2 = -0.0478; 
float   m_uiFilter_A1 = -1.899; 
float   m_uiFilter_A2 = 0.9044; 

int     HBM_NoFinger_Count_Pre = 0;
float   HBM_Rate_Pre       = 0.0;
float   HBM_Rate_Pre_Tmp   = 0.0;
int     HBM_RateCount      = 0;
int     HBM_RateCount_Pre  = 0;
int     HBM_Perist_Count   = 0;
int     HBM_NoFinger_Index = 0;
int     HBM_Flag           = 0;
float   HBM_First_1        = 0.0;
float   HBM_First_2        = 0.0;
float   HBM_First_3        = 0.0;
int     HBM_First_Index    = 0;
int     HBM_Rate_Index     = 0;
float   HBM_Rate_Index_Pre = 0.0;
int     HBM_Rate_Flag      = 0;

void HBM_Init()
{
    HBM_Rate_Pre       = 0.0;
	HBM_RateCount      = 0;
	HBM_RateCount_Pre  = 0;
	HBM_Perist_Count   = 0;
	HBM_NoFinger_Index = 0;
	HBM_Flag           = 0;
	HBM_NoFinger_Count_Pre = 0;
	HBM_Rate_Flag      = 0;

	memset(m_nzValues_x, 0, sizeof(int) * POINT_COUNT); 
	memset(m_nzValues_y, 0, sizeof(int) * POINT_COUNT);  
	memset(m_nzValues_z, 0, sizeof(int) * POINT_COUNT); 
	memset(m_nzValues_a, 0, sizeof(int) * POINT_COUNT); 
	memset(m_nzValues_RAW, 0, sizeof(int) * POINT_COUNT); 
	memset(m_nzValues_Rate, 0, sizeof(m_nzValues_Rate)); 

	HBM_First_Index    = 0;
	HBM_First_1        = 0.0;
	HBM_First_2        = 0.0;
	HBM_First_3        = 0.0;
	HBM_Rate_Index_Pre = 0.0;
	HBM_Rate_Index     = 0;
    //ALOGI("                           HBM Init");

}
void FilterVariable(float TimeFreq)
{
	float Div;
	Div = (4 / pow(TimeFreq,2)) + (25.14 / TimeFreq) + 88.83;

	m_uiFilter_B0 = (25.14 / TimeFreq) / Div; 
	m_uiFilter_B1 = 0; 
	m_uiFilter_B2 = (-25.14 / TimeFreq) / Div; 
	m_uiFilter_A1 = ((-8 / pow(TimeFreq,2)) + 177.66) / Div; 
	m_uiFilter_A2 = ((4 / pow(TimeFreq,2)) - (25.14 / TimeFreq) + 88.83) / Div; 
}

float cal(uint32_t uiCounts)
{
    uint32_t uiPos,n;
    int i,j;
    const float QData = 1E-6;//constant value for algo
    const float RData = 0.5;//constant value for algo
    float fHBR = 0.0;
    float DifRatio = 1.0;
    long RawMA = 0;
	float fHBRMA = 0.0;
    if (HBM_Flag == 0){
    	FilterVariable(m_uiTimeFreq);
    	HBM_Flag = 1;
    }
    if ((uiCounts >= m_uiHighThreshold)||(uiCounts <= m_uiLowThreshold)){
    	//ALOGI("not match high-low threshold");
		HBM_RateCount = 0;
    	if (uiCounts > HBM_NoFinger_Count_Pre){
			HBM_NoFinger_Index++;
			if (HBM_NoFinger_Index > 2){
				HBM_Rate_Pre = 0.0;
				HBM_Rate_Pre       = 0.0;
				HBM_RateCount      = 0;
				HBM_RateCount_Pre  = 0;
				HBM_Perist_Count   = 0;
				HBM_NoFinger_Index = 0;
				HBM_Flag           = 0;
				HBM_NoFinger_Count_Pre = 0;
				HBM_Rate_Flag      = 0;

				HBM_First_Index    = 0;
				HBM_First_1        = 0.0;
				HBM_First_2        = 0.0;
				HBM_First_3        = 0.0;
				HBM_Rate_Index_Pre = 0.0;
				HBM_Rate_Index     = 0;
				ALOGI("====================    Re Init");
				return HBM_Rate_Pre;
			}
		}else if (uiCounts < HBM_NoFinger_Count_Pre){
			HBM_NoFinger_Index = 0;			
		}
		HBM_NoFinger_Count_Pre = uiCounts;
    }else{
		HBM_NoFinger_Index = 0;
	    //Note: enhance the performance, use index to move data instead of futile copying
	    for (i=0; i<POINT_COUNT-1; i++) { 
			m_nzValues_x[i]  = m_nzValues_x[i+1]; 
			m_nzValues_y[i]  = m_nzValues_y[i+1]; 
			m_nzValues_z[i]  = m_nzValues_z[i+1]; 
			m_nzValues_p[i]  = m_nzValues_p[i+1];
			m_nzValues_kg[i] = m_nzValues_kg[i+1];
			m_nzValues_a[i]  = m_nzValues_a[i+1];	
			m_nzValues_RAW[i] = m_nzValues_RAW[i+1];
	    }
	    n = POINT_COUNT-1;
	    //x(n)
	    RawMA = 0;
	    m_nzValues_RAW[n] = uiCounts; 
	    for (i=0;i<RAW_MA_NUM;i++){
	    	if (m_nzValues_RAW[n-i]>0)
	        	RawMA += m_nzValues_RAW[n-i];
	        else
	        	break;
	    }
	    RawMA /= i;
		
	    m_nzValues_x[n] = RawMA;

	    //y(n)=0.1287*x(n)-0.1287*x(n-2)+1.829*y(n-1)-0.8345*y(n-2)
	    //m_nzValues_y[n] = (0.1287 * m`_nzValues_x[n]) - (0.1287 * m_nzValues_x[n-2]) + (1.829 * m_nzValues_y[n-1]) - (0.8345 * m_nzValues_y[n-2]); 
	    //y(n)=0.0478*x(n)-0.0478*x(n-2)+1.899*y(n-1)-0.9044*y(n-2)
	    //meaningfully define valuables
	    //m_nzValues_y[n] = (0.0478 * m_nzValues_x[n]) - (0.0478 * m_nzValues_x[n-2]) + (1.899 * m_nzValues_y[n-1]) - (0.9044 * m_nzValues_y[n-2]); 
	    m_nzValues_y[n] = (m_uiFilter_B0 * m_nzValues_x[n]) + (m_uiFilter_B2 * m_nzValues_x[n-2]) - (m_uiFilter_A1 * m_nzValues_y[n-1]) - (m_uiFilter_A2 * m_nzValues_y[n-2]); 
		
	    m_nzValues_kg[n] = (m_nzValues_p[n-1] + QData) / ((m_nzValues_p[n-1] + QData) + RData);
	    m_nzValues_z[n] = m_nzValues_z[n-1] + m_nzValues_kg[n] * (m_nzValues_y[n] - m_nzValues_z[n-1]);
	    m_nzValues_p[n] = (1 - m_nzValues_kg[n]) * (m_nzValues_p[n-1] + QData);

	    //y(n)=0.1287*x(n)-0.1287*x(n-2)+1.829*y(n-1)-0.8345*y(n-2)
	    //m_nzValues_a[n] = (0.1287 * m_nzValues_z[n]) - (0.1287 * m_nzValues_z[n-2]) + (1.829 * m_nzValues_a[n-1]) - (0.8345 * m_nzValues_a[n-2]); 
	    //y(n)=0.0478*x(n)-0.0478*x(n-2)+1.899*y(n-1)-0.9044*y(n-2)
	    //m_nzValues_a[n] = (0.0478 * m_nzValues_z[n]) - (0.0478 * m_nzValues_z[n-2]) + (1.899 * m_nzValues_a[n-1]) - (0.9044 * m_nzValues_a[n-2]); 
	    m_nzValues_a[n] = (m_uiFilter_B0 * m_nzValues_z[n]) + (m_uiFilter_B2 * m_nzValues_z[n-2]) - (m_uiFilter_A1 * m_nzValues_a[n-1]) - (m_uiFilter_A2 * m_nzValues_a[n-2]); 
		
		HBM_RateCount ++;
		
		if ((m_nzValues_a[n-1] < 0)&&(m_nzValues_a[n] > 0)){
			fHBR = (1.0 / (HBM_RateCount * m_uiTimeFreq)) * 60.0;//Condition 2
			HBM_Rate_Index ++;
			//if (HBM_Rate_Pre != 0.0){
			if (HBM_Rate_Flag == 1){
				DifRatio = HBM_Rate_Pre / fHBR;//Note: Condition 1
				if(((DifRatio < m_uiRateRatioMax) && (DifRatio > m_uiRateRatioMin)) && ((fHBR < m_uiRateMax) && (fHBR > m_uiRateMin)))
				{
					HBM_Rate_Index_Pre = HBM_Rate_Index;
					HBM_Rate_Index = 0;
					HBM_Perist_Count ++;
					if (HBM_Perist_Count >= RATE_PERSIST){
					    //Note: use index for shifting
						for (i=0;i<RATE_MA_NUM-1;i++)
							m_nzValues_Rate[i] = m_nzValues_Rate[i+1];
						m_nzValues_Rate[i] = fHBR;
						fHBRMA = 0;
						for (i=0;i<RATE_MA_NUM;i++){
							if (m_nzValues_Rate[RATE_MA_NUM-1-i] > 0){
								fHBRMA += m_nzValues_Rate[RATE_MA_NUM-1-i];	
							}else{
								break;
							}
						}
						fHBRMA /= i;

						HBM_Perist_Count --;
						HBM_Rate_Pre = fHBRMA;	
						//ALOGI("AA %d,%.4f,%.2f,%.2f,%.4f", uiCounts,m_nzValues_a[n],fHBR,HBM_Rate_Pre,DifRatio);
					}
				}
				else
				{
					HBM_Perist_Count = 0;
					HBM_RateCount = 0;
					if (HBM_Rate_Index > 2){
						HBM_Rate_Index = 0;
						HBM_Rate_Flag = 0;
						ALOGI("====================    Re Caculating Measure");
					/*}else{
						HBM_Rate_Pre_Tmp = fHBR;
						ALOGI("3 %d,%.4f,%.2f,%.2f,%.2f", uiCounts,m_nzValues_a[n],fHBR,HBM_Rate_Pre,HBM_Rate_Pre_Tmp);
						return HBM_Rate_Pre_Tmp;*/
					}
				}
			}else{			
				float DiffA,DiffB;
				HBM_First_1      = HBM_First_2;
				HBM_First_2      = HBM_First_3;
				HBM_First_3      = fHBR;
				DiffA = HBM_First_1/HBM_First_2;
				DiffB = HBM_First_2/HBM_First_3;
				if ((DiffA < m_uiRateRatioMax) && (DiffA > m_uiRateRatioMin)){
					HBM_Rate_Pre = (HBM_First_1 + HBM_First_2) / 2;
					HBM_Rate_Flag = 1;
				}else if ((DiffB < m_uiRateRatioMax) && (DiffB > m_uiRateRatioMin)){
					HBM_Rate_Pre = (HBM_First_2 + HBM_First_3) / 2;
					HBM_Rate_Flag = 1;
				/*}else{
					HBM_RateCount = 0;
					HBM_Rate_Pre_Tmp = fHBR;
					ALOGI("2 %d,%.4f,%.2f,%.2f,%.2f", uiCounts,m_nzValues_a[n],fHBR,HBM_Rate_Pre,HBM_Rate_Pre_Tmp);
					return HBM_Rate_Pre_Tmp;*/
				}
			}
			HBM_RateCount = 0;
		//}else{
		//	ALOGI("00 %d,%.4f", uiCounts,m_nzValues_a[n]);
			ALOGI("1 %d,%.4f,%.2f,%.2f", uiCounts,m_nzValues_a[n],fHBR,HBM_Rate_Pre);
		}
		ALOGI("%d,%.4f,%.2f,%.2f", uiCounts,m_nzValues_a[n],fHBR,HBM_Rate_Pre);
	}
	return HBM_Rate_Pre;
}
