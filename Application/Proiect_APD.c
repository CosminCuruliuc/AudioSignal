#include <utility.h>
#include <formatio.h>
#include <ansi_c.h>
#include <analysis.h>

#include <cvirte.h>		
#include <userint.h>
#include "Proiect_APD.h"

//==============================================================================
// Constants
#define SAMPLE_RATE		0
#define NPOINTS			1

//==============================================================================
// Global variables
int waveInfo[2]; //waveInfo[0] = sampleRate
				 //waveInfo[1] = number of elements
double sampleRate = 0.0;
int npoints = 0;
double *waveData = 0;
double *waveDataModdified = 0;
int currentSecond = 0;
int const nr_intervale = 20;
double axeHistogram[nr_intervale];
int freq_np = 1024, freq_pos;
int curr_window, number_of_window, fereastra;

static int panelHandle;
static int panelFrecv;

int main (int argc, char *argv[])
{
	if (InitCVIRTE (0, argv, 0) == 0)
		return -1;	/* out of memory */
	if ((panelHandle = LoadPanel (0, "Proiect_APD.uir", PANEL)) < 0)
		return -1;
	if((panelFrecv = LoadPanel(0, "Proiect_APD.uir", PANEL_2)) < 0)
		return -1;
	DisplayPanel (panelHandle);
	RunUserInterface ();
	DiscardPanel (panelHandle);
	free(waveData);
	return 0;
}

void CalculateAndDisplayStatistics(int panel, double* waveData, int npoints)
{
    double minValue, maxValue, mean, stdDev, median, dispersion;
    int minIndex, maxIndex;
    int zeroCrossings = 0;
    ssize_t histogram[nr_intervale];
    int i;

    // Min, Max and their indices
	MaxMin1D(waveData, npoints, &maxValue, &maxIndex, &minValue, &minIndex);
	
    // Mean and Standard Deviation
	Mean(waveData, npoints, &mean);
	StdDev(waveData, npoints, &mean, &stdDev);

    // Dispersion is the square of standard deviation
    dispersion = stdDev * stdDev;

    // Median
    Median(waveData, npoints, &median);

    // Zero Crossings
    for (i = 1; i < npoints; i++)
    {
        if ((waveData[i] * waveData[i-1]) < 0 || waveData[i] == 0)
            zeroCrossings++;
    }

    // Histogram
    Histogram(waveData, npoints, minValue, maxValue, histogram, axeHistogram, nr_intervale);
	PlotXY(panel, PANEL_GRAPH_HISTOGRAM, axeHistogram, histogram, nr_intervale, VAL_DOUBLE, VAL_SSIZE_T, VAL_VERTICAL_BAR, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
	
	// Display
	SetCtrlVal(panelHandle, PANEL_NUMERIC_MAX_VALUE, maxValue);
	SetCtrlVal(panelHandle, PANEL_NUMERIC_MAX_INDEX, maxIndex);
	SetCtrlVal(panelHandle, PANEL_NUMERIC_MIN_VALUE, minValue);
	SetCtrlVal(panelHandle, PANEL_NUMERIC_MIN_INDEX, minIndex);
	SetCtrlVal(panelHandle, PANEL_NUMERIC_DISPERSION, dispersion);
	SetCtrlVal(panelHandle, PANEL_NUMERIC_MEAN, mean);
	SetCtrlVal(panelHandle, PANEL_NUMERIC_MEDIAN, median);
	SetCtrlVal(panelHandle, PANEL_NUMERIC_ZERO, zeroCrossings);

}

int CVICALLBACK OnLoadButtonCB (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			//executa script python pentru conversia unui fisierului .wav in .txt
			LaunchExecutable("python main.py");
			
			//astept sa fie generate cele doua fisiere (modificati timpul daca este necesar
			Delay(4);
			
			//incarc informatiile privind rata de esantionare si numarul de valori
			FileToArray("wafeInfo.txt", waveInfo, VAL_INTEGER, 2, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			sampleRate = waveInfo[SAMPLE_RATE];
			npoints = waveInfo[NPOINTS];
			
			//alocare memorie pentru numarul de puncte
			waveData = (double *) calloc(npoints, sizeof(double));
			
			//incarcare din fisierul .txt in memorie (vector)
			FileToArray("waveData.txt", waveData, VAL_DOUBLE, npoints, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
			
			// Daca durata este mai mare de 10 secunde
			if (npoints / sampleRate > 10)
			{
				double* trimmedWaveData = (double *) calloc(6 * sampleRate, sizeof(double));
				int i;
				// Pentru primele 6 secunde:
				for (i = 0; i < 6 * sampleRate; i++)
				{
					trimmedWaveData[i] = waveData[i];
				}

				free(waveData);
				waveData = trimmedWaveData;
				npoints = 6 * sampleRate;
			}
			
			// Afisare pe grapf
			PlotY(panel, PANEL_GRAPH, waveData, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			// Afisare rezultate
			CalculateAndDisplayStatistics(panel, waveData, npoints);
			
			// Pornire buton filtrare + Anvelopa
			SetCtrlAttribute(panel, PANEL_RING_FILTRU, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, PANEL_BUTTON_ENVELOPE, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, PANEL_BINARYSWITCH, ATTR_DIMMED, 0);
			break;
	}
	return 0;
}

// Cele doua filtre
double* filterAverage(double* signal, int npoints, int avg_elements)
{
    double* filtered = (double*)calloc(npoints, sizeof(double));
    for(int i = 0; i < npoints; i++)
    {
        double sum = 0;
        for(int j = 0; j < avg_elements && i-j >= 0; j++)
        {
            sum += signal[i-j];
        }
        filtered[i] = sum / avg_elements;
    }
    return filtered;
}

double* filterFirstOrder(double* signal, int npoints, double alpha)
{
    double* filtered = (double*)calloc(npoints, sizeof(double));
    filtered[0] = signal[0];
    for(int i = 1; i < npoints; i++)
    {
        filtered[i] = (1 - alpha) * filtered[i-1] + alpha * signal[i];
    }
    return filtered;
}

// Calcul derivata
void CalculateDerivative(double* signal, double* derivative, int length) {
    for (int i = 1; i < length; i++) {
        derivative[i - 1] = signal[i] - signal[i - 1];
    }
    derivative[length - 1] = 0;
}

// Atributul dimmed la selectarea diferitelor filtre
int CVICALLBACK OnFilterButton (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	int filtru;
	switch (event)
	{
		case EVENT_COMMIT:
			// None
			GetCtrlVal(panel, PANEL_RING_FILTRU, &filtru);
			if(filtru == 0) {
				SetCtrlAttribute(panel, PANEL_RING_DIMENSIUNE, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_NUMERIC_ALPHA, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 1);
			}
			// Mediere
			else if (filtru == 1) {
				SetCtrlAttribute(panel, PANEL_RING_DIMENSIUNE, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_NUMERIC_ALPHA, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 0);
			}
			// Ordin 1
			else if (filtru == 2) {
				SetCtrlAttribute(panel, PANEL_RING_DIMENSIUNE, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_NUMERIC_ALPHA, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 0);
			}
			break;
	}
	return 0;
}

// Functie de update a graficului
void UpdateFilteredGraph(int panel, double* filteredData, int sampleRate, int second) {
    int startIndex = second * sampleRate;
    int endIndex = startIndex + sampleRate;
    if (endIndex > npoints) endIndex = npoints;

    double* subWaveData = (double*)calloc(endIndex - startIndex, sizeof(double));
    for (int i = startIndex; i < endIndex; i++) {
        subWaveData[i - startIndex] = filteredData[i];
    }

    DeleteGraphPlot(panel, PANEL_GRAPH_FILTERED_DATA, -1, VAL_IMMEDIATE_DRAW);
    PlotY(panel, PANEL_GRAPH_FILTERED_DATA, subWaveData, endIndex - startIndex, VAL_DOUBLE, 
          VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);

    if (npoints >= 256) {
        double skewness, kurtosis;
        Moment(subWaveData, 256, 3, &skewness); // Skewness
        Moment(subWaveData, 256, 4, &kurtosis); // Kurtosis

        SetCtrlVal(panelHandle, PANEL_NUMERIC_SKEWNESS, skewness);
        SetCtrlVal(panelHandle, PANEL_NUMERIC_KURTOSIS, kurtosis);
    }
	
	SetCtrlVal(panel, PANEL_NUMERIC_START, currentSecond);
	SetCtrlVal(panel, PANEL_NUMERIC_STOP, currentSecond + 1);
    free(subWaveData); 
}

// Afisare derivata
void PlotDerivative(int panel, double* derivative, int sampleRate, int second) {
	int startIndex = second * sampleRate;
    int endIndex = startIndex + sampleRate;
    if (endIndex > npoints) endIndex = npoints;

    double* subWaveData = (double*)calloc(endIndex - startIndex, sizeof(double));
    for (int i = startIndex; i < endIndex; i++) {
        subWaveData[i - startIndex] = derivative[i];
    }
    DeleteGraphPlot(panel, PANEL_GRAPH_FILTERED_DATA, -1, VAL_IMMEDIATE_DRAW);

    PlotY(panel, PANEL_GRAPH_FILTERED_DATA, subWaveData, endIndex - startIndex, VAL_DOUBLE, 
          VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_BLUE);
}

// La apasarea button-ului de apply
int CVICALLBACK OnApplyButton (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	int filtru;
	int dimensiune;
	double alpha;
	switch (event)
	{
		case EVENT_COMMIT:
			SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_PREV, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_NEXT, ATTR_DIMMED, 0);
			SetCtrlAttribute(panel, PANEL_TOGGLE_DERIVATIVE, ATTR_DIMMED, 0);
			GetCtrlVal(panel, PANEL_RING_FILTRU, &filtru);
			// Mediere
			if(filtru == 1) {
				GetCtrlVal(panel, PANEL_RING_DIMENSIUNE, &dimensiune);
				waveDataModdified = filterAverage(waveData, npoints, dimensiune);	
			}
			// Ordin 1
			else {
				GetCtrlVal(panel, PANEL_NUMERIC_ALPHA, &alpha);
				waveDataModdified = filterFirstOrder(waveData, npoints, alpha);
			}
			
			int showDerivative;
        	GetCtrlVal(panel, PANEL_TOGGLE_DERIVATIVE, &showDerivative);
        
        	if (showDerivative) {
            	double* derivative = (double *)calloc(npoints, sizeof(double));
            	CalculateDerivative(waveDataModdified, derivative, npoints);
            	PlotDerivative(panel, derivative, sampleRate, currentSecond);
				free(derivative);
        	} else {
            	UpdateFilteredGraph(panel, waveDataModdified, sampleRate, currentSecond);
        	}			
			break;
	}
	return 0;
}

// Functie de salvare a unor imagini
int SaveGraphToJPEG(int panel, int control, char* filename) {
	int bitmapId;
	if(GetCtrlDisplayBitmap(panel, control, 1, &bitmapId) < 0)
		return -1;
	
	int result = SaveBitmapToJPEGFile(bitmapId, filename, JPEG_PROGRESSIVE, 100);
	DiscardBitmap(bitmapId);
	return result;
}


// Butonul de next
int CVICALLBACK OnNextButton (int panel, int control, int event,
                              void *callbackData, int eventData1, int eventData2) {
    if (event == EVENT_COMMIT) {
		if(currentSecond < npoints / sampleRate - 1)
		{
	        int showDerivative;
	        GetCtrlVal(panel, PANEL_TOGGLE_DERIVATIVE, &showDerivative);
	        
	        if (showDerivative) {
	            double* derivative = (double *)calloc(npoints, sizeof(double));
	            CalculateDerivative(waveDataModdified, derivative, npoints);
	            PlotDerivative(panel, derivative, sampleRate, currentSecond);
				free(derivative);
	        } else {
				currentSecond++;
	            UpdateFilteredGraph(panel, waveDataModdified, sampleRate, currentSecond);
        }
		
		char fileName[260];
		sprintf(fileName, "e:\\APD_Proiect\\Imagini\\grafic_next_%d.jpg", currentSecond);
		SaveGraphToJPEG(panel, PANEL_GRAPH_FILTERED_DATA, fileName);
		}
    }
    return 0;
}

// Butonul de Prev (secunda anterioara)
int CVICALLBACK OnPrevButton (int panel, int control, int event,
                              void *callbackData, int eventData1, int eventData2) {
    if (event == EVENT_COMMIT) {
        if (currentSecond > 0) {
            int showDerivative;
        	GetCtrlVal(panel, PANEL_TOGGLE_DERIVATIVE, &showDerivative);
        
        	if (showDerivative) {
            	double* derivative = (double *)calloc(npoints, sizeof(double));
            	CalculateDerivative(waveDataModdified, derivative, npoints);
            	PlotDerivative(panel, derivative, sampleRate, currentSecond);
				free(derivative);
        	} else {
				currentSecond--;
            	UpdateFilteredGraph(panel, waveDataModdified, sampleRate, currentSecond);
        	}
			
			char fileName[260];
			sprintf(fileName, "e:\\APD_Proiect\\Imagini\\grafic_next_%d.jpg", currentSecond);
			SaveGraphToJPEG(panel, PANEL_GRAPH_FILTERED_DATA, fileName);
		}
    }
    return 0;
}

// Switch-ul On/Off pentru derivata
int CVICALLBACK OnDerivativeButton (int panel, int control, int event,
									void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int showDerivative;
        	GetCtrlVal(panel, PANEL_TOGGLE_DERIVATIVE, &showDerivative);
        
        	if (showDerivative) {
            	double* derivative = (double *)calloc(npoints, sizeof(double));
            	CalculateDerivative(waveDataModdified, derivative, npoints);
            	PlotDerivative(panel, derivative, sampleRate, currentSecond);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_PREV, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_NEXT, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_RING_FILTRU, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_RING_DIMENSIUNE, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_NUMERIC_ALPHA, ATTR_DIMMED, 1);
        	} else {
				UpdateFilteredGraph(panel, waveDataModdified, sampleRate, currentSecond);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_PREV, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_NEXT, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_RING_FILTRU, ATTR_DIMMED, 0);
				int filtru;
				GetCtrlVal(panel, PANEL_RING_FILTRU, &filtru);
				if(filtru == 0) {
					SetCtrlAttribute(panel, PANEL_RING_DIMENSIUNE, ATTR_DIMMED, 1);
					SetCtrlAttribute(panel, PANEL_NUMERIC_ALPHA, ATTR_DIMMED, 1);
					SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 1);
				}
				// Mediere
				else if (filtru == 1) {
					SetCtrlAttribute(panel, PANEL_RING_DIMENSIUNE, ATTR_DIMMED, 0);
					SetCtrlAttribute(panel, PANEL_NUMERIC_ALPHA, ATTR_DIMMED, 1);
					SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 0);
				}
				// Ordin 1
				else if (filtru == 2) {
					SetCtrlAttribute(panel, PANEL_RING_DIMENSIUNE, ATTR_DIMMED, 1);
					SetCtrlAttribute(panel, PANEL_NUMERIC_ALPHA, ATTR_DIMMED, 0);
					SetCtrlAttribute(panel, PANEL_COMMANDBUTTON_APLICA, ATTR_DIMMED, 0);
				}				
			}
			break;
	}
	return 0;
}

// Functia de afisare a anvelopei
void PlotEnvelope(){
	double* envelope = (double *) calloc(npoints, sizeof(double));
	system("python anvelopa.py");
	FileToArray("anvelopa.txt", envelope, VAL_DOUBLE, npoints, 1, VAL_GROUPS_TOGETHER, VAL_GROUPS_AS_COLUMNS, VAL_ASCII);
	DeleteGraphPlot(panelHandle, PANEL_GRAPH_FILTERED_DATA, -1, VAL_IMMEDIATE_DRAW);
	PlotY(panelHandle, PANEL_GRAPH, waveData, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
	PlotY(panelHandle, PANEL_GRAPH, envelope, npoints, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_GREEN);
	free(envelope);
}

// La apasarea butonului de afisare a anvelopei
int CVICALLBACK OnEnvelopeCB (int panel, int control, int event,
							  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			PlotEnvelope();
			break;
	}
	return 0;
}

int CVICALLBACK OnMainPanel (int panel, int event, void *callbackData,
							 int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface(0);
			break;
	}
	return 0;
}

int CVICALLBACK OnPanelSwitch (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			if(panel == panelHandle)
			{
				SetCtrlVal(panelFrecv, PANEL_2_BINARYSWITCH, 1);
				DisplayPanel(panelFrecv);
				HidePanel(panel);
			}
			else 
			{
				SetCtrlVal(panelHandle, PANEL_BINARYSWITCH, 0);
				DisplayPanel(panelHandle);
				HidePanel(panel);
			}
			break;
	}
	return 0;
}

int CVICALLBACK OnFrecventaPanel (int panel, int event, void *callbackData,
								  int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_GOT_FOCUS:

			break;
		case EVENT_LOST_FOCUS:

			break;
		case EVENT_CLOSE:
			QuitUserInterface(0);
			break;
	}
	return 0;
}

int CVICALLBACK OnStartStopButton (int panel, int control, int event,
								   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			int start;
			GetCtrlVal(panel, PANEL_2_TOGGLEBUTTON, &start);
			if(start == 1) {
				SetCtrlAttribute(panel, PANEL_2_RINGWINDOW, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_2_RINGPOINTS, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_2_POLY_SAV, ATTR_DIMMED, 1);
				SetCtrlAttribute(panel, PANEL_2_SIDE_SAV, ATTR_DIMMED, 1);
				
				SetCtrlAttribute (panel, PANEL_2_TIMER, ATTR_ENABLED, 1);
				
				number_of_window = (int) npoints / freq_np + 1;
				curr_window = (int) freq_pos / freq_np + 1;
				freq_pos = freq_np * curr_window;
				SetCtrlVal(panel, PANEL_2_NUMERIC_CURR_WIN, curr_window);
				SetCtrlVal(panel, PANEL_2_NUMERIC_NUM_WIN, number_of_window);
			} else {
				SetCtrlAttribute(panel, PANEL_2_RINGWINDOW, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_2_RINGPOINTS, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_2_POLY_SAV, ATTR_DIMMED, 0);
				SetCtrlAttribute(panel, PANEL_2_SIDE_SAV, ATTR_DIMMED, 0);
		
				SetCtrlAttribute (panel, PANEL_2_TIMER, ATTR_ENABLED, 0);
			}
			break;
	}
	return 0;
}

int CVICALLBACK OnSwitchWindow (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, PANEL_2_RINGWINDOW, &fereastra);
			break;
	}
	return 0;
}

int CVICALLBACK OnNumberOfPoints (int panel, int control, int event,
								  void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			GetCtrlVal(panel, PANEL_2_RINGPOINTS, &freq_np);
			break;
	}
	return 0;
}

int CVICALLBACK OnResetButton (int panel, int control, int event,
							   void *callbackData, int eventData1, int eventData2)
{
	switch (event)
	{
		case EVENT_COMMIT:
			SetCtrlVal(panel, PANEL_2_RINGWINDOW, 1);
			SetCtrlAttribute(panel, PANEL_2_RINGWINDOW, ATTR_DIMMED, 0);
			
			SetCtrlVal(panel, PANEL_2_RINGPOINTS, 1024);
			SetCtrlAttribute(panel, PANEL_2_RINGPOINTS, ATTR_DIMMED, 0);
			
			SetCtrlVal(panel, PANEL_2_POLY_SAV, 1);
			SetCtrlAttribute(panel, PANEL_2_POLY_SAV, ATTR_DIMMED, 0);
			
			SetCtrlVal(panel, PANEL_2_SIDE_SAV, 1);
			SetCtrlAttribute(panel, PANEL_2_SIDE_SAV, ATTR_DIMMED, 0);
			
			SetCtrlAttribute (panel, PANEL_2_TIMER, ATTR_ENABLED, 0);
			number_of_window = 0;
			curr_window = 0;
			SetCtrlVal(panel, PANEL_2_NUMERIC_CURR_WIN, curr_window);
			SetCtrlVal(panel, PANEL_2_NUMERIC_NUM_WIN, number_of_window);
			freq_pos = 0;
			break;
	}
	return 0;
}

int CVICALLBACK OnSpectruTimer (int panel, int control, int event,
								void *callbackData, int eventData1, int eventData2)
{
	FIRCoefStruct coefs;
	switch (event)
	{
		case EVENT_TIMER_TICK:
			
			WindowConst windowConst;
			int windowType;
			if(fereastra == 1) // Dreptunghiulara
			{
				windowType = 0;
			} else { // BlackMan
				windowType = 5;
			}
			
			
			//Semnal pe interval
			if(freq_pos == npoints)
			{
				freq_pos = 0;
				curr_window = 0;
			}
			if(freq_pos + freq_np > npoints)
			{
				freq_pos = npoints - freq_np;
			}
			double sample[freq_np];
			Copy1D(waveData+freq_pos, freq_np, sample);
			
			DeleteGraphPlot(panel, PANEL_2_GRAPH_RAW_DATA, -1, VAL_IMMEDIATE_DRAW);
			PlotY(panel, PANEL_2_GRAPH_RAW_DATA, sample, freq_np, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			//Spectrum
			double df;
			double autoSpectrum[freq_np/2];
			double convertedSpectrum[freq_np/2];
			char unit[32] = "V";
			double freqPeak, powerPeak;
			
			ScaledWindowEx(sample, freq_np, windowType, 0, &windowConst);
			
			AutoPowerSpectrum(sample, freq_np, 1/sampleRate, autoSpectrum, &df);
			
			PowerFrequencyEstimate(autoSpectrum, freq_np/2, -1.0, windowConst, df, 7, &freqPeak, &powerPeak);
			
			SetCtrlVal(panel, PANEL_2_FREQ_PEAK, freqPeak);
			SetCtrlVal(panel, PANEL_2_POWER_PEAK, powerPeak);
			
			SpectrumUnitConversion(autoSpectrum, freq_np/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df, windowConst, convertedSpectrum, unit);
			
			DeleteGraphPlot(panel, PANEL_2_GRAPH_SPECTRUM, -1, VAL_IMMEDIATE_DRAW);
			PlotWaveform(panel, PANEL_2_GRAPH_SPECTRUM, convertedSpectrum, freq_np/2, VAL_DOUBLE, 1.0, 0.0, 0.0, 2*df, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			//SavitzkyGolay
			Copy1D(waveData+freq_pos, freq_np, sample);
			int poly, nSidePoints;
			GetCtrlVal(panel, PANEL_2_POLY_SAV, &poly);
			GetCtrlVal(panel, PANEL_2_SIDE_SAV, &nSidePoints);
			SavitzkyGolayFiltering (sample, freq_np, poly, nSidePoints, NULL, sample);
			
			DeleteGraphPlot(panel, PANEL_2_GRAPH_SAVITZKY, -1, VAL_IMMEDIATE_DRAW);
			PlotY(panel, PANEL_2_GRAPH_SAVITZKY, sample, freq_np, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			//SavitzkyGolay Spectrum
			AutoPowerSpectrum(sample, freq_np, 1/sampleRate, autoSpectrum, &df);
			
			SpectrumUnitConversion(autoSpectrum, freq_np/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df, windowConst, convertedSpectrum, unit);
			
			DeleteGraphPlot(panel, PANEL_2_GRAPH_SPEC_SAVITSKY, -1, VAL_IMMEDIATE_DRAW);
			PlotWaveform(panel, PANEL_2_GRAPH_SPEC_SAVITSKY, convertedSpectrum, freq_np/2, VAL_DOUBLE, 1.0, 0.0, 0.0, 2*df, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			//Notch
			double Denominator[3], Numerator[3];
			double w0 = 2 * 3.14 * freqPeak / sampleRate, r;// pulsatia corespunzatoare frecventei de taiare (ffreq)
			//normalizate (impatuta la frecventa de esantionare)
			GetCtrlVal(panel, PANEL_2_NOTCH_PARAM, &r);
			//coeficientii calculati conform formulelor
			Denominator[0] = 1;
			Denominator[1] = -2 * r*cos(w0);
			Denominator[2] = r*r;
			Numerator[1] = -2 * cos(w0);
			Numerator[2] = 1;//2 * q;
			Numerator[0]=1;
			double wfm3[freq_np];
			//initializarea primelor doua valori filtrate
			wfm3[0]=sample[0]; wfm3[1]=sample[1];
			//calcularea valorilor marimii filtrate (wfm3) utilizand ecuatia cu diferente;
			//wfm este semnalul nefiltrat
			for (int i=2;i<freq_np;i++)
				wfm3[i] = Numerator[0]*sample[i] + Numerator[1]*sample[i-1] + Numerator[2]*sample[i-2] - Denominator[1]*wfm3[i-1] - Denominator[2]*sample[i-2];
			
			DeleteGraphPlot(panel, PANEL_2_GRAPH_NOTCH, -1, VAL_IMMEDIATE_DRAW);
			PlotY(panel, PANEL_2_GRAPH_NOTCH, wfm3, freq_np, VAL_DOUBLE, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			//Notch Spectrum
			AutoPowerSpectrum(wfm3, freq_np, 1/sampleRate, autoSpectrum, &df);
			
			SpectrumUnitConversion(autoSpectrum, freq_np/2, SPECTRUM_POWER, SCALING_MODE_LINEAR, DISPLAY_UNIT_VRMS, df, windowConst, convertedSpectrum, unit);
			
			DeleteGraphPlot(panel, PANEL_2_GRAPH_SPEC_NOTCH, -1, VAL_IMMEDIATE_DRAW);
			PlotWaveform(panel, PANEL_2_GRAPH_SPEC_NOTCH, convertedSpectrum, freq_np/2, VAL_DOUBLE, 1.0, 0.0, 0.0, 2*df, VAL_THIN_LINE, VAL_EMPTY_SQUARE, VAL_SOLID, VAL_CONNECTED_POINTS, VAL_RED);
			
			curr_window++;
			SetCtrlVal(panel, PANEL_2_NUMERIC_CURR_WIN, curr_window);
			freq_pos += freq_np;
			
			char fileName[70];
            int bitmapId;
            char * time = TimeStr();
            time[2] = '.';
            time[5] = '.';
            char * date = DateStr();

            sprintf(fileName, "./grafic_frecv_%s_%s_semnal.jpg", time, date);
            GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_RAW_DATA, 1, &bitmapId);
            SaveBitmapToJPEGFile(bitmapId, fileName, JPEG_PROGRESSIVE, 100);
            DiscardBitmap(bitmapId);
            
            sprintf(fileName, "./grafic_frecv_%s_%s_semnal_savitzky.jpg", time, date);
            GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_SAVITZKY, 1, &bitmapId);
            SaveBitmapToJPEGFile(bitmapId, fileName, JPEG_PROGRESSIVE, 100);
            DiscardBitmap(bitmapId);
            
            sprintf(fileName, "./grafic_frecv_%s_%s_semnal_notch.jpg", time, date);
            GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_NOTCH, 1, &bitmapId);
            SaveBitmapToJPEGFile(bitmapId, fileName, JPEG_PROGRESSIVE, 100);
            DiscardBitmap(bitmapId);
            
            sprintf(fileName, "./grafic_frecv_%s_%s_spectru.jpg", time, date);
            GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_SPECTRUM, 1, &bitmapId);
            SaveBitmapToJPEGFile(bitmapId, fileName, JPEG_PROGRESSIVE, 100);
            DiscardBitmap(bitmapId);
			
			sprintf(fileName, "./grafic_frecv_%s_%s_spectru_savitzky.jpg", time, date);
            GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_SPEC_SAVITSKY, 1, &bitmapId);
            SaveBitmapToJPEGFile(bitmapId, fileName, JPEG_PROGRESSIVE, 100);
            DiscardBitmap(bitmapId);
            
            sprintf(fileName, "./grafic_frecv_%s_%s_spectru_notch.jpg", time, date);
            GetCtrlDisplayBitmap(panel, PANEL_2_GRAPH_SPEC_NOTCH, 1, &bitmapId);
            SaveBitmapToJPEGFile(bitmapId, fileName, JPEG_PROGRESSIVE, 100);
            DiscardBitmap(bitmapId);
			
			break;
	}
	return 0;
}
