/**************************************************************************/
/* LabWindows/CVI User Interface Resource (UIR) Include File              */
/*                                                                        */
/* WARNING: Do not add to, delete from, or otherwise modify the contents  */
/*          of this include file.                                         */
/**************************************************************************/

#include <userint.h>

#ifdef __cplusplus
    extern "C" {
#endif

     /* Panels and Controls: */

#define  PANEL                            1       /* callback function: OnMainPanel */
#define  PANEL_COMMANDBUTTON_APLICA       2       /* control type: command, callback function: OnApplyButton */
#define  PANEL_BUTTON_ENVELOPE            3       /* control type: command, callback function: OnEnvelopeCB */
#define  PANEL_COMMANDBUTTON_3            4       /* control type: command, callback function: OnLoadButtonCB */
#define  PANEL_COMMANDBUTTON_NEXT         5       /* control type: command, callback function: OnNextButton */
#define  PANEL_COMMANDBUTTON_PREV         6       /* control type: command, callback function: OnPrevButton */
#define  PANEL_NUMERIC_START              7       /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_STOP               8       /* control type: numeric, callback function: (none) */
#define  PANEL_RING_FILTRU                9       /* control type: ring, callback function: OnFilterButton */
#define  PANEL_GRAPH_FILTERED_DATA        10      /* control type: graph, callback function: (none) */
#define  PANEL_GRAPH_HISTOGRAM            11      /* control type: graph, callback function: (none) */
#define  PANEL_GRAPH                      12      /* control type: graph, callback function: (none) */
#define  PANEL_NUMERIC_MAX_VALUE          13      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_MAX_INDEX          14      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_KURTOSIS           15      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_SKEWNESS           16      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_ZERO               17      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_MEDIAN             18      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_MEAN               19      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_DISPERSION         20      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_MIN_INDEX          21      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_MIN_VALUE          22      /* control type: numeric, callback function: (none) */
#define  PANEL_NUMERIC_ALPHA              23      /* control type: numeric, callback function: (none) */
#define  PANEL_TEXTMSG                    24      /* control type: textMsg, callback function: (none) */
#define  PANEL_RING_DIMENSIUNE            25      /* control type: ring, callback function: (none) */
#define  PANEL_TOGGLE_DERIVATIVE          26      /* control type: textButton, callback function: OnDerivativeButton */
#define  PANEL_BINARYSWITCH               27      /* control type: binary, callback function: OnPanelSwitch */

#define  PANEL_2                          2       /* callback function: OnFrecventaPanel */
#define  PANEL_2_BINARYSWITCH             2       /* control type: binary, callback function: OnPanelSwitch */
#define  PANEL_2_GRAPH_SPECTRUM           3       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH_SPEC_SAVITSKY      4       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH_SPEC_NOTCH         5       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH_NOTCH              6       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH_SAVITZKY           7       /* control type: graph, callback function: (none) */
#define  PANEL_2_GRAPH_RAW_DATA           8       /* control type: graph, callback function: (none) */
#define  PANEL_2_RINGWINDOW               9       /* control type: slide, callback function: OnSwitchWindow */
#define  PANEL_2_RINGPOINTS               10      /* control type: ring, callback function: OnNumberOfPoints */
#define  PANEL_2_FREQ_PEAK                11      /* control type: numeric, callback function: (none) */
#define  PANEL_2_POWER_PEAK               12      /* control type: numeric, callback function: (none) */
#define  PANEL_2_NUMERIC_CURR_WIN         13      /* control type: numeric, callback function: (none) */
#define  PANEL_2_NUMERIC_NUM_WIN          14      /* control type: numeric, callback function: (none) */
#define  PANEL_2_TOGGLEBUTTON             15      /* control type: textButton, callback function: OnStartStopButton */
#define  PANEL_2_COMMANDBUTTON            16      /* control type: command, callback function: OnResetButton */
#define  PANEL_2_POLY_SAV                 17      /* control type: numeric, callback function: (none) */
#define  PANEL_2_SIDE_SAV                 18      /* control type: numeric, callback function: (none) */
#define  PANEL_2_TIMER                    19      /* control type: timer, callback function: OnSpectruTimer */
#define  PANEL_2_TEXTMSG                  20      /* control type: textMsg, callback function: (none) */
#define  PANEL_2_NOTCH_PARAM              21      /* control type: numeric, callback function: (none) */
#define  PANEL_2_TEXTMSG_2                22      /* control type: textMsg, callback function: (none) */


     /* Control Arrays: */

#define  CTRLARRAY                        1

     /* Menu Bars, Menus, and Menu Items: */

          /* (no menu bars in the resource file) */


     /* Callback Prototypes: */

int  CVICALLBACK OnApplyButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnDerivativeButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnEnvelopeCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFilterButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnFrecventaPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnLoadButtonCB(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnMainPanel(int panel, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnNextButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnNumberOfPoints(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPanelSwitch(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnPrevButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnResetButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSpectruTimer(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnStartStopButton(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);
int  CVICALLBACK OnSwitchWindow(int panel, int control, int event, void *callbackData, int eventData1, int eventData2);


#ifdef __cplusplus
    }
#endif