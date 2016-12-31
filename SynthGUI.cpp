/// GUI application for controlling the synthengine process
/// Copyright James Higgs 2016
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <vector>
#include <string>
#include <math.h>
#include <assert.h>
#include <linux/input.h>

#include "common.h"

#define DrawText DrawText		// stupid Windows
#include "fontengine.h"
#include "GUI.h"

#define WIDTH	800
#define HEIGHT	480

// For rendering text
FontEngine *bigFont = NULL;
FontEngine *smallFont = NULL;

// These must be global 
std::string wfNames[] = {
	"NONE",
	"SQUARE",
	"SAW1",
	"SAW2",
	"TRIANGLE",
	"SINE",
	"NOISE"
};

// Curent patch object (for updating main page settings)
CPatch workPatch;
struct globalSynthSettings_t {
	float filterCutoff;				// 0.0 to 1.0
	float reverbDepth;				// 0.0 to 1.0
} globalSynthSettings;

// Touchscreen handler
extern int Touch_Open();
extern void Touch_Update();
extern void Touch_Close();

// mqueue messaging
static void PostMessage(mqd_t mq, char *buffer, int length)
{
	mq_send(mq, buffer, length, 0);
}

// Clear out the specified message queue
static void FlushMessageQueue(mqd_t mq)
{
	char mqbuffer[MSG_MAX_SIZE];
	ssize_t bytes_read = 1;
	while (bytes_read > 0)
		{
		bytes_read = mq_receive(mq, mqbuffer, MSG_MAX_SIZE, NULL);
		}
}

/// Patch Select screen
/// @return -1 if cnacelled, else index of patch selected
int DoPatchSelect(SDL_Renderer *renderer, mqd_t mqEngine, mqd_t mqGUI)
{
	// Remove any lingering messages
	FlushMessageQueue(mqGUI);
	FlushMessageQueue(mqEngine);

	// Send "Request patch names" message to the synth engine
	// Use "undefined" CC message 104 - Data is patch start index and number of patch names wanted
	char outBuffer[MSG_MAX_SIZE] = { 0xB0, 104, 0, 32 };
	PostMessage(mqEngine, outBuffer, 4);

	// Give synthengine some time to formulate and queue a response
	SDL_Delay(100);

	// Check for data from the synthengine process
	std::string names[32];
	char buffer[MSG_MAX_SIZE];
	for (int i = 0; i < 32; i++)
		{
		ssize_t bytes_read = mq_receive(mqGUI, buffer, MSG_MAX_SIZE, NULL);
		if (bytes_read > 0)
			{
			buffer[bytes_read] = 0;
			//printf("msg received: %s\n", buffer);
			// All messages are MIDI messages (sysex)
			char command = buffer[0];
			if (0xF0 == command)
				{
				// Sysex - Synth engine is sending (shortened) patch data
				if (0x7D == buffer[1])
					{
					char sysexCmd = buffer[4];
					char paramAddr = buffer[6];		// param "address"
					char sysexSize = buffer[7];		// size of data (or data requested)
					if (0x12 == sysexCmd)				// "DT1" - recieve patch data
						{
						// read and convert the data
						char *p = buffer + 8;
						CPatch patch;
						patch.UnpackParams(p, paramAddr, sysexSize);
						names[i].assign(patch.GetName());
						//printf("%s\n", patch.GetName());
						}
					}
				}
			}
		}

	// Setup the popup layout
	int parentWidth, parentHeight;
	SDL_RenderGetLogicalSize(renderer, &parentWidth, &parentHeight);

	SDL_Rect gmRect = { 0, 0, parentWidth, parentHeight };
	CGUIManager gm(gmRect);
	gm.m_drawContext.m_renderer = renderer;
	gm.m_drawContext.m_font = smallFont;
	gm.m_drawContext.SetForeColour(255, 255, 255, 0);
	gm.m_drawContext.SetTextColour(255, 255, 255, 0);
	gm.m_drawContext.SetBackColour(0, 64, 0, 255);

	// Add the controls to the layout
	const int COLUMNS = 4;
	const int ROWS = 8;
	const int MARGIN = 4;
	const int CW = (gmRect.w / COLUMNS) - MARGIN;
	const int CH = (gmRect.h / ROWS) - MARGIN;
	for (int i = 0; i < 32; i++)
		{
		gm.AddControl((MARGIN + CW) * (i % 4), (MARGIN + CH) * (i/4), CW, CH, CT_BUTTON,  "PatchName", names[i].c_str());
		}

	int selectedIndex = -1;
	SDL_Event event;
	bool done = false;
	while(!done)
		{
		Touch_Update();
		while (SDL_PollEvent(&event))
			{
			switch (event.type)
				{
				case SDL_KEYDOWN :
					// Check for ESC pressed
					if (SDLK_ESCAPE == event.key.keysym.sym)
						done = true;
					break;
				case SDL_MOUSEBUTTONUP :
					// Get the relevant control from the gui manager
					CGUIControl *control = gm.OnMouseUp(event.button.x, event.button.y);
					if (control)
						{
						// get "local coords"
						int x = control->m_rect.x;
						int y = control->m_rect.y;
						const int ix = x / (MARGIN + CW);
						const int iy = y / (MARGIN + CH);
						selectedIndex  = ix + COLUMNS * iy;
						done = true;
						}
					break;
				}	// end switch
			}	// wend event

		gm.DrawAllControls();
		
		//SDL_Flip(screen);
		SDL_RenderPresent(renderer);
		SDL_Delay(10);
		}

	printf("Selected patch %d.\n", selectedIndex);

	return selectedIndex;
}


//bool reverbOn = true;

/// Set up the main synth GUI screen
static int SetupMainPage(CGUIManager &gm)
{
	const int CH = 28;				// char height
	const int CW = 12;				// char width
	const int MARGIN = CW;
	const int INDENT = CW;
	const int LABELW = 10 * CW;
	const int CNTRLW = 10 * CW;
	const int LCOL1X = MARGIN + INDENT;
	const int CCOL1X = LCOL1X + LABELW;
	const int COLWIDTH = (WIDTH - (MARGIN * 2)) / 3;
	const int LCOL2X = MARGIN + INDENT + COLWIDTH;
	const int CCOL2X = LCOL2X + LABELW;
	const int LCOL3X = MARGIN + INDENT + (2 * COLWIDTH);
	const int CCOL3X = LCOL3X + LABELW;
	const int ROW1Y = MARGIN + 2 * CH;
	const int RDY = CH + (CH/3);	// row spacing
	
	// Patch name edit
	gm.AddControl(MARGIN, MARGIN, LCOL2X, CH, CT_EDIT,    "PatchName", "_patchname");
	gm.AddControl(MARGIN + LCOL2X + MARGIN, MARGIN, 6 * CW, CH, CT_BUTTON,  "SavePatch", "Save");
	gm.AddControl(MARGIN + LCOL2X + 2 * MARGIN + 6 * CW, MARGIN, 8 * CW, CH, CT_BUTTON,  "CopyPatchTo", "Copy To");
	gm.AddControl(WIDTH - MARGIN - 14 * CW, MARGIN, 14 * CW, CH, CT_BUTTON,  "SelectPatch", "Select Patch");

	// OSC 1 parameters
	gm.AddControl(LCOL1X - MARGIN, ROW1Y, LABELW, CH, CT_LABEL,      "OSC1Label", "OSC1");
	gm.AddControl(LCOL1X, ROW1Y + RDY, LABELW, CH,    CT_LABEL,      "WF1Label", "Waveform");
	gm.AddControl(LCOL1X, ROW1Y + RDY*2, LABELW, CH,  CT_LABEL,      "Duty1Label", "Duty %");
	gm.AddControl(LCOL1X, ROW1Y + RDY*3, LABELW, CH,  CT_LABEL,      "Detune1Label", "Detune");
	gm.AddControl(LCOL1X, ROW1Y + RDY*4, LABELW, CH,  CT_LABEL,      "VEnv1Label", "Vol Env");
	gm.AddControl(LCOL1X, ROW1Y + RDY*5, LABELW, CH,  CT_LABEL,      "PEnv1Label", "Pitch Env");
	gm.AddControl(LCOL1X, ROW1Y + RDY*6, LABELW, CH,  CT_LABEL,      "FEnv1Label", "Filter Env");
	gm.AddControl(LCOL1X, ROW1Y + RDY*7, LABELW, CH,  CT_LABEL,      "LFOVEnv1Label", "LFO V Env");
	gm.AddControl(LCOL1X, ROW1Y + RDY*8, LABELW, CH,  CT_LABEL,      "LFOPEnv1Label", "LFO P Env");
	gm.AddControl(LCOL1X, ROW1Y + RDY*9, LABELW, CH,  CT_LABEL,      "LFOFEnv1Label", "LFO F Env");

	gm.AddControl(CCOL1X, ROW1Y + RDY, CNTRLW, CH,   CT_OPTIONLIST, "WF1Option", NULL);
	gm.AddControl(CCOL1X, ROW1Y + RDY*2, CNTRLW, CH, CT_SLIDER,     "Duty1Slider", "Duty");
	gm.AddControl(CCOL1X, ROW1Y + RDY*3, CNTRLW, CH, CT_SLIDER,     "Detune1Slider", "Detune");
	gm.AddControl(CCOL1X, ROW1Y + RDY*4, CNTRLW, CH, CT_ENVELOPE,   "VEnv1Envelope", NULL);
	gm.AddControl(CCOL1X, ROW1Y + RDY*5, CNTRLW, CH, CT_ENVELOPE,   "PEnv1Envelope", NULL);
	gm.AddControl(CCOL1X, ROW1Y + RDY*6, CNTRLW, CH, CT_ENVELOPE,   "FEnv1Envelope", NULL);
	gm.AddControl(CCOL1X, ROW1Y + RDY*7, CNTRLW, CH, CT_ENVELOPE,   "LFOVEnv1Envelope", NULL);
	gm.AddControl(CCOL1X, ROW1Y + RDY*8, CNTRLW, CH, CT_ENVELOPE,   "LFOPEnv1Envelope", NULL);
	gm.AddControl(CCOL1X, ROW1Y + RDY*9, CNTRLW, CH, CT_ENVELOPE,   "LFOFEnv1Envelope", NULL);

	// OSC 2 parameters
	gm.AddControl(LCOL2X - MARGIN, ROW1Y, LABELW, CH,   CT_LABEL,      "OSC2Label", "OSC2");
	gm.AddControl(LCOL2X, ROW1Y + RDY, LABELW, CH,   CT_LABEL,      "WF2Label", "Waveform");
	gm.AddControl(LCOL2X, ROW1Y + RDY*2, LABELW, CH,   CT_LABEL,      "Duty2Label", "Duty %");
	gm.AddControl(LCOL2X, ROW1Y + RDY*3, LABELW, CH,  CT_LABEL,      "Detune2Label", "Detune");
	gm.AddControl(LCOL2X, ROW1Y + RDY*4, LABELW, CH,  CT_LABEL,      "VEnv2Label", "Vol Env");
	gm.AddControl(LCOL2X, ROW1Y + RDY*5, LABELW, CH,  CT_LABEL,      "PEnv2Label", "Pitch Env");
	gm.AddControl(LCOL2X, ROW1Y + RDY*6, LABELW, CH,  CT_LABEL,      "FEnv2Label", "Filter Env");
	gm.AddControl(LCOL2X, ROW1Y + RDY*7, LABELW, CH,  CT_LABEL,      "LFOVEnv2Label", "LFO V Env");
	gm.AddControl(LCOL2X, ROW1Y + RDY*8, LABELW, CH,  CT_LABEL,      "LFOPEnv2Label", "LFO P Env");
	gm.AddControl(LCOL2X, ROW1Y + RDY*9, LABELW, CH,  CT_LABEL,      "LFOFEnv2Label", "LFO F Env");

	gm.AddControl(CCOL2X, ROW1Y + RDY, CNTRLW, CH,  CT_OPTIONLIST, "WF2Option", NULL);
	gm.AddControl(CCOL2X, ROW1Y + RDY*2, CNTRLW, CH,  CT_SLIDER,     "Duty2Slider", "Duty");
	gm.AddControl(CCOL2X, ROW1Y + RDY*3, CNTRLW, CH,  CT_SLIDER,     "Detune2Slider", "Detune");
	gm.AddControl(CCOL2X, ROW1Y + RDY*4, CNTRLW, CH, CT_ENVELOPE,   "VEnv2Envelope", NULL);
	gm.AddControl(CCOL2X, ROW1Y + RDY*5, CNTRLW, CH, CT_ENVELOPE,   "PEnv2Envelope", NULL);
	gm.AddControl(CCOL2X, ROW1Y + RDY*6, CNTRLW, CH, CT_ENVELOPE,   "FEnv2Envelope", NULL);
	gm.AddControl(CCOL2X, ROW1Y + RDY*7, CNTRLW, CH, CT_ENVELOPE,   "LFOVEnv2Envelope", NULL);
	gm.AddControl(CCOL2X, ROW1Y + RDY*8, CNTRLW, CH, CT_ENVELOPE,   "LFOPEnv2Envelope", NULL);
	gm.AddControl(CCOL2X, ROW1Y + RDY*9, CNTRLW, CH, CT_ENVELOPE,   "LFOFEnv2Envelope", NULL);

	// LFO parameters
	gm.AddControl(LCOL3X - MARGIN, ROW1Y, LABELW, CH,  CT_LABEL,      "LFOLabel", "LFO");
	gm.AddControl(LCOL3X, ROW1Y + RDY, LABELW, CH,  CT_LABEL,      "LFOWFLabel", "Waveform");
	gm.AddControl(LCOL3X, ROW1Y + RDY*2, LABELW, CH,  CT_LABEL,      "LFOFreqLabel", "Freq.");
	gm.AddControl(LCOL3X, ROW1Y + RDY*3, LABELW, CH, CT_LABEL,      "LFODepthLabel", "Depth");

	gm.AddControl(CCOL3X, ROW1Y + RDY, CNTRLW, CH,  CT_OPTIONLIST, "LFOWFOption", NULL);
	gm.AddControl(CCOL3X, ROW1Y + RDY*2, CNTRLW, CH,  CT_SLIDER,     "LFOFreqSlider", "LFO Freq.");
	gm.AddControl(CCOL3X, ROW1Y + RDY*3, CNTRLW, CH,  CT_SLIDER,     "LFODepthSlider", "LFO Depth");

	// Reverb parameters
	gm.AddControl(LCOL3X, ROW1Y + RDY*4, LABELW, CH, CT_LABEL,      "RvbDepthLabel", "Rev Depth");
	gm.AddControl(CCOL3X, ROW1Y + RDY*4, CNTRLW, CH, CT_SLIDER,     "RvbDepth", "Reverb Depth");

	// Filter parameters
	gm.AddControl(LCOL3X, ROW1Y + RDY*5, LABELW, CH, CT_LABEL,      "FilterCutoffLabel", "Filt Cutoff");
	gm.AddControl(CCOL3X, ROW1Y + RDY*5, CNTRLW, CH, CT_SLIDER,     "FilterCutoff", "Filter Cutoff");

/*

10   190  40    10   CT_LABEL		"MODLabel"	"MOD"
20   210  70    10   CT_LABEL		"PB Range"	""
20   230  70    10   CT_LABEL		"Mod Target"	""
20   250  70    10   CT_LABEL		"Mod Range"	""
100  205  60    17   CT_SLIDER		"PBRangeSlider"	""
100  225  60    17   CT_OPTIONLIST	"ModTargetOption"	""
100  245  60    17   CT_SLIDER		"ModRangeSlider"	""
*/

	CGUIOptionList *wf1OptList = (CGUIOptionList *)gm.GetControl("WF1Option");
	CGUIOptionList *wf2OptList = (CGUIOptionList *)gm.GetControl("WF2Option");
	CGUIOptionList *lfoWfOptList = (CGUIOptionList *)gm.GetControl("LFOWFOption");
	if (!wf1OptList || !wf2OptList || !lfoWfOptList)
		printf("NULL optlist!\n");

	for (int i = 0; i < 7; i++)
		{
		wf1OptList->m_optionArray.push_back(wfNames[i]);
		wf2OptList->m_optionArray.push_back(wfNames[i]);
		lfoWfOptList->m_optionArray.push_back(wfNames[i]);
		}
	wf1OptList->m_selectedIndex = 0;
	wf2OptList->m_selectedIndex = 0;
	lfoWfOptList->m_selectedIndex = 0;

	return gm.m_controls.size();
}

/// Setup ADSR envelope control from patch envelope data
static void SetEnvelopeControlData(CGUIEnvelope *control, const CEnvelope *envData)
{
	if (control && envData)
		control->SetADSR(envData->m_delay, envData->m_attack, envData->m_peak, envData->m_decay, envData->m_sustain, envData->m_release);
}

/// Set patch envelope data from ADSR envelope control
static void GetEnvelopeDataFromControl(CGUIEnvelope *control, CEnvelope *envData)
{
	if (control && envData)
		control->GetADSR(envData->m_delay, envData->m_attack, envData->m_peak, envData->m_decay, envData->m_sustain, envData->m_release);
}

/// Refresh the main page from the specified patch
static void UpdateMainPageFromPatch(CGUIManager &gm, const CPatch &patch)
{
	CGUIControl *control = gm.GetControl("PatchName");
	control->SetText(patch.GetName());
	
	// OSC1
	control = gm.GetControl("WF1Option");
	((CGUIOptionList *)control)->SetSelectedIndex(patch.m_osc1.m_waveType);

	control = gm.GetControl("Duty1Slider");
	((CGUISlider *)control)->Init(patch.m_osc1.m_duty, 0.0f, 1.0f, false);

	control = gm.GetControl("Detune1Slider");
	((CGUISlider *)control)->Init(patch.m_osc1.m_detune, -1.0f, 1.0f, false);

	control = gm.GetControl("VEnv1Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc1.m_envelope[ET_VOLUME]);
	control = gm.GetControl("PEnv1Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc1.m_envelope[ET_PITCH]);
	control = gm.GetControl("FEnv1Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc1.m_envelope[ET_FILTER]);

	control = gm.GetControl("LFOVEnv1Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc1.m_LFOEnvelope[ET_VOLUME]);
	control = gm.GetControl("LFOPEnv1Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc1.m_LFOEnvelope[ET_PITCH]);
	control = gm.GetControl("LFOFEnv1Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc1.m_LFOEnvelope[ET_FILTER]);

	// OSC2
	control = gm.GetControl("WF2Option");
	((CGUIOptionList *)control)->SetSelectedIndex(patch.m_osc2.m_waveType);

	control = gm.GetControl("Duty2Slider");
	((CGUISlider *)control)->Init(patch.m_osc2.m_duty, 0.0f, 1.0f, false);

	control = gm.GetControl("Detune2Slider");
	((CGUISlider *)control)->Init(patch.m_osc2.m_detune, -1.0f, 1.0f, false);

	control = gm.GetControl("VEnv2Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc2.m_envelope[ET_VOLUME]);
	control = gm.GetControl("PEnv2Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc2.m_envelope[ET_PITCH]);
	control = gm.GetControl("FEnv2Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc2.m_envelope[ET_FILTER]);

	control = gm.GetControl("LFOVEnv2Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc2.m_LFOEnvelope[ET_VOLUME]);
	control = gm.GetControl("LFOPEnv2Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc2.m_LFOEnvelope[ET_PITCH]);
	control = gm.GetControl("LFOFEnv2Envelope");
	SetEnvelopeControlData((CGUIEnvelope *)control, &patch.m_osc2.m_LFOEnvelope[ET_FILTER]);

	// LFO
	control = gm.GetControl("LFOWFOption");
	((CGUIOptionList *)control)->SetSelectedIndex(patch.m_LFOWaveform);

	control = gm.GetControl("LFOFreqSlider");
	((CGUISlider *)control)->Init(patch.m_LFOFreq, 0.1f, LFO_MAX_FREQ, false);

	control = gm.GetControl("LFODepthSlider");
	((CGUISlider *)control)->Init(patch.m_LFODepth, 0.0f, 1.0f, false);

	// Global settings
	control = gm.GetControl("FilterCutoff");
	((CGUISlider *)control)->Init(globalSynthSettings.filterCutoff, 0.0f, 1.0f, false);

	control = gm.GetControl("RvbDepth");
	((CGUISlider *)control)->Init(globalSynthSettings.reverbDepth, 0.0f, 1.0f, false);
}

/// Update the patch data from the controls
static void UpdatePatchFromMainPage(CGUIManager &gm, CPatch &patch)
{
	CGUIControl *control = gm.GetControl("PatchName");
	patch.SetName(control->GetText());
	
	// OSC1
	control = gm.GetControl("WF1Option");
	patch.m_osc1.m_waveType = (WAVETYPE)((CGUIOptionList *)control)->GetSelectedIndex();

	control = gm.GetControl("Duty1Slider");
	patch.m_osc1.m_duty = ((CGUISlider *)control)->m_value;

	control = gm.GetControl("Detune1Slider");
	patch.m_osc1.m_detune = ((CGUISlider *)control)->m_value;

	control = gm.GetControl("VEnv1Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc1.m_envelope[ET_VOLUME]);
	control = gm.GetControl("PEnv1Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc1.m_envelope[ET_PITCH]);
	control = gm.GetControl("FEnv1Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc1.m_envelope[ET_FILTER]);

	control = gm.GetControl("LFOVEnv1Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc1.m_LFOEnvelope[ET_VOLUME]);
	control = gm.GetControl("LFOPEnv1Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc1.m_LFOEnvelope[ET_PITCH]);
	control = gm.GetControl("LFOFEnv1Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc1.m_LFOEnvelope[ET_FILTER]);

	// OSC2
	control = gm.GetControl("WF2Option");
	patch.m_osc2.m_waveType = (WAVETYPE)((CGUIOptionList *)control)->GetSelectedIndex();

	control = gm.GetControl("Duty2Slider");
	patch.m_osc2.m_duty = ((CGUISlider *)control)->m_value;

	control = gm.GetControl("Detune2Slider");
	patch.m_osc2.m_detune = ((CGUISlider *)control)->m_value;

	control = gm.GetControl("VEnv2Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc2.m_envelope[ET_VOLUME]);
	control = gm.GetControl("PEnv2Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc2.m_envelope[ET_PITCH]);
	control = gm.GetControl("FEnv2Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc2.m_envelope[ET_FILTER]);

	control = gm.GetControl("LFOVEnv2Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc2.m_LFOEnvelope[ET_VOLUME]);
	control = gm.GetControl("LFOPEnv2Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc2.m_LFOEnvelope[ET_PITCH]);
	control = gm.GetControl("LFOFEnv2Envelope");
	GetEnvelopeDataFromControl((CGUIEnvelope *)control, &patch.m_osc2.m_LFOEnvelope[ET_FILTER]);
	
	// LFO
	control = gm.GetControl("LFOWFOption");
	patch.m_LFOWaveform = (WAVETYPE)((CGUIOptionList *)control)->GetSelectedIndex();

	control = gm.GetControl("LFOFreqSlider");
	patch.m_LFOFreq = ((CGUISlider *)control)->m_value;
	
	control = gm.GetControl("LFODepthSlider");
	patch.m_LFODepth = ((CGUISlider *)control)->m_value;
}

/// Update the working GUI patch info from a CC message
static void UpdatePatchFromCC(CPatch &patch, char cc, char value)
{
	// Mapping of CC numbers to patch parameters
	// Osc 1 waveform		Knob 1		CC 14
	// Osc 1 duty			Knob 2		CC 15
	// Osc 1 detune			Knob 3		CC 16
	// Osc 2 waveform		Knob 5		CC 17
	// Osc 2 duty			Knob 6		CC 18
	// Osc 2 detune			Knob 7		CC 19
	// LFO waveform			Knob 4		CC 20
	// LFO speed 			Knob 8		CC 21
	// Envelope delay		Slider 1	CC 22
	// Envelope attack		Slider 2	CC 23
	// Envelope peak		Slider 3	CC 24
	// Envelope decay		Slider 4	CC 25
	// Envelope sustain		Slider 5	CC 26
	// Envelope release		Slider 6	CC 27
	// Cutoff freq			Slider 7	CC 74
	// Resonance			Slider 8	CC 71

	switch(cc)
		{
		case 14 :
			patch.m_osc1.m_waveType = (WAVETYPE)(value / (128 / WT_MAX));
			break;
		case 15 :
			patch.m_osc1.m_duty = UnpackValue(value, 0x7F);
			break;
		case 16 :
			patch.m_osc1.m_detune = UnpackValue(value, 0x7F) * 2.0f - 1.0f;
			break;
		case 17 :
			patch.m_osc2.m_waveType = (WAVETYPE)(value / (128 / WT_MAX));
			break;
		case 18 :
			patch.m_osc2.m_duty = UnpackValue(value, 0x7F);
			break;
		case 19 :
			patch.m_osc2.m_detune = UnpackValue(value, 0x7F) * 2.0f - 1.0f;
			break;
		case 20 :
			patch.m_LFOWaveform = (WAVETYPE)(value / (128 / WT_MAX));
			break;
		case 21 :
			patch.m_LFOFreq = UnpackValue(value, 0x7F) * LFO_MAX_FREQ;
			break;
		case 22 :	
			// TODO : Which envelope?
			//patch.m_delay = UnpackValue(buffer[0], 0x7F) * ENV_MAX_ATTACK;
			//m_attack = UnpackValue(buffer[1], 0x7F) * ENV_MAX_ATTACK;
			//m_peak = UnpackValue(buffer[2], 0x7F);
			//m_decay = UnpackValue(buffer[3], 0x7F) * ENV_MAX_ATTACK;
			//m_sustain = UnpackValue(buffer[4], 0x7F);
			//m_release = UnpackValue(buffer[5], 0x7F) * ENV_MAX_ATTACK;

			break;
		case 23 :
			break;
		case 24 :
			break;
		case 25 :
			break;
		case 26 :
			break;
		case 27 :
			break;
		case 71 :		// Resonance
			break;
		case 74 :		// Cutoff
			globalSynthSettings.filterCutoff = UnpackValue(value, 0x7F);
			break;
		case 91 :		// Reverb Depth
			globalSynthSettings.reverbDepth = UnpackValue(value, 0x7F);
			break;
		}	// end switch
			
}

/// Process messages recieved from the synth engine
static void ProcessMessage(char *buffer, int length)
{
	// parse buffer and do something (see ref WaveSynth::ProcessMessage())
	// TODO : Handle patch info recieve
	if (length < 1)
		return;
		
	// All messages are MIDI messages
	char command = buffer[0];
	switch (command & 0xF0)
		{
		printf("ProcessMessage(%X %X %X)\n", buffer[0], buffer[1], buffer[2]);
		case 0xB0 :			// Controller change
			{
			//char cc = buffer[1];
			// Update the working patch info
			UpdatePatchFromCC(workPatch, buffer[1], buffer[2]);
			}
			break;
		case 0xC0 :			// Patch change
			{
			//char patchIndex = buffer[1];
			//m_currentPatch = patchIndex;
			}
			break;
		case 0xE0 :			// Pitch bend
			{
			//char bend = buffer[1];
			}
			break;
		case 0XF0 :			// System / sysex
			{
			if (0xF0 == command)
				{
				// Sysex - Synth engine is sending patch data
				if (0x7D == buffer[1])
					{
					char sysexCmd = buffer[4];
					char paramAddr = buffer[6];		// param "address"
					char sysexSize = buffer[7];		// size of data (or data requested)
					if (0x12 == sysexCmd)				// "DT1" - recieve patch data
						{
						// read and convert the data
						char *p = buffer + 8;
						// DEBUG
						//printf("Sysex data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
						//        p[20], p[21], p[22], p[23], p[24], p[25], p[26], p[27], p[28], p[29]);

						workPatch.UnpackParams(p, paramAddr, sysexSize);
						printf("Recieved patch '%s' data.\n", workPatch.GetName());
						//workPatch.Dump();
						}
					}
				}
			}
			break;
		}	// end switch
}

/// Request current (working) patch data from the synth engine
static void RequestPatchData(mqd_t mq)
{
	printf("SG: Requesting patch data\n"); 
	// create MIDI sysex RQ1 message to request the patch data
	char buffer[] = { 0xF0, 0x7D, 0x01, 0x01, 0x11, 0x00, 0x00, PADDR_END, 0x00, 0xF7 } ;
	PostMessage(mq, buffer, 10);
	
	// Synth engine will send DT1 sysex data
}


int main(int argc, char *argv[])
{
	// IPC message queue stuff
	char mqbuffer[MSG_MAX_SIZE];
	
	// Open queue for sending messages to the synth engine
	printf("Opening message queue...\n");
	mqd_t mqEngine = mq_open(ENGINE_QUEUE_NAME, O_WRONLY | O_NONBLOCK);
	//assert(mqEngine != (mqd_t)-1);
	if (mqEngine == (mqd_t)-1)
		{
		printf("Could not open mqEngine message queue.\nMake sure synth engine is running.\n");
		return -1;
		}

	// Open queue for reading message from the synth engine
	mqd_t mqGUI = mq_open(GUI_QUEUE_NAME, O_RDONLY | O_NONBLOCK);
	//assert(mqGUI != (mqd_t)-1);
	if (mqEngine == (mqd_t)-1)
		{
		printf("Could not open mqGUI message queue.\n");
		return -1;
		}
		
	// Flush the incoming message queue
	printf("SG: Flushing incoming message queue\n");
	ssize_t bytes_read = 1;
	while (bytes_read > 0)
		{
		bytes_read = mq_receive(mqGUI, mqbuffer, MSG_MAX_SIZE, NULL);
		}


 	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS))
		printf("SDL_Init() failed: %s\n", SDL_GetError());

	//// Use OpenGLES2
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);

	SDL_Window *window = SDL_CreateWindow("SynthGUI", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	//SDL_Window *window = SDL_CreateWindow("SynthGUI", 100, 100, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
	//SDL_Window *window = SDL_CreateWindow("SynthGUI", 0, 0, WIDTH, HEIGHT, SDL_WINDOW_FULLSCREEN);
	if (!window)
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (!renderer)
		printf("SDL_CreateRenderer failed.\n");

	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

	bigFont = new FontEngine(renderer, "font_16x32.bmp", 16, 32);
	//smallFont = new FontEngine(renderer, "font_8x16.bmp", 8, 16);
	smallFont = new FontEngine(renderer, "font_12x24.bmp", 12, 24);

	// Create the GUI control manager and set up our layout
	int guiWidth, guiHeight;
	SDL_GetWindowSize(window, &guiWidth, &guiHeight);
	// This needed so controls can determine parent window size
	SDL_RenderSetLogicalSize(renderer, guiWidth, guiHeight);

	SDL_Rect gmRect = { 0, 0, guiWidth, guiHeight };
	CGUIManager gm(gmRect);
	gm.m_drawContext.m_renderer = renderer;
	gm.m_drawContext.m_font = smallFont;
	gm.m_drawContext.SetForeColour(192, 192, 128, 0);
	gm.m_drawContext.SetTextColour(32, 192, 32, 0);
	gm.m_drawContext.SetBackColour(0, 0, 0, 255);
	SetupMainPage(gm);

	// Ask the synth engine for current patch info
	RequestPatchData(mqEngine);

	// Init global synth settings
	globalSynthSettings.filterCutoff = 64;
	globalSynthSettings.reverbDepth = 0;

    // Init touch screen input
    Touch_Open();

	SDL_Event event;
	bool done = false;
	while(!done)
		{
		bool redraw = false;
        Touch_Update();
			
		while (SDL_PollEvent(&event))
			{
			switch (event.type)
				{
				case SDL_QUIT :
					done = true;
					break;
				case SDL_KEYDOWN :
					if (SDLK_END == event.key.keysym.sym)
						{
						done = true;	
						}
					else
						{
						// Trigger note from keyboard
						mqbuffer[0] = 0x80;
						mqbuffer[1] = (char)event.key.keysym.sym;
						mqbuffer[2] = 0x7F;
						mqbuffer[4] = 0;
						printf("Posting NoteOn message...\n");
						PostMessage(mqEngine, mqbuffer, 4);
						}
					break;
				case SDL_KEYUP :
					// Note up
					mqbuffer[0] = 0x90;
					mqbuffer[1] = (char)event.key.keysym.sym;
					mqbuffer[2] = 0;
					printf("Posting NoteOff message...\n");
					PostMessage(mqEngine, mqbuffer, 3);
					break;
				case SDL_JOYBUTTONDOWN :
					break;
				case SDL_JOYBUTTONUP :
					break;
				case SDL_MOUSEMOTION :
					{
					// Trap dragging of sliders etc
					// Let gui manager handle the event first
					CGUIControl *control = gm.OnMouseMove(event.button.x, event.button.y);
					if (control)
						{
						if (0 == strcmp(control->m_name, "FilterCutoff"))
							{
							// Use "filter cutoff" CC message 74
							globalSynthSettings.filterCutoff = ((CGUISlider *)control)->m_value;
							unsigned char value = (unsigned char)(globalSynthSettings.filterCutoff * 0x7F);
							char outBuffer[4] = { 0xB0, 74, value };
							PostMessage(mqEngine, outBuffer, 3);
							gm.m_controlChanged = false;
							redraw = true;
							}
						else if (0 == strcmp(control->m_name, "RvbDepth"))
							{
							// Use "Reverb Send" CC message 91
							globalSynthSettings.reverbDepth = ((CGUISlider *)control)->m_value;
							unsigned char value = (unsigned char)(globalSynthSettings.reverbDepth * 0x7F);
							char outBuffer[4] = { 0xB0, 91, value };
							PostMessage(mqEngine, outBuffer, 3);
							gm.m_controlChanged = false;
							redraw = true;
							}
						}
					}
					break;
				case SDL_MOUSEBUTTONDOWN :
					gm.OnMouseDown(event.button.x, event.button.y);
					break;
				case SDL_MOUSEBUTTONUP :
					{
					// Let the gui manager handle the mouse up first
					CGUIControl *control = gm.OnMouseUp(event.button.x, event.button.y);
					// Now we see if we need to handle it
					//HandleControlClick(control, mqEngine);
					if (control)
						{
						printf("Button clicked: %s\n", control->m_name);
						if (0 == strcmp(control->m_name, "SavePatch"))
							{
							// Send "Save" message to the synth engine
							// Use "undefined" CC message 102, data = 1, 0
							char outBuffer[MSG_MAX_SIZE] = { 0xB0, 102, 1, 0 };
							PostMessage(mqEngine, outBuffer, 4);
							// Give synthengine some time to formulate and queue a response
							SDL_Delay(50);
							gm.m_controlChanged = false;
							}
						else if (0 == strcmp(control->m_name, "CopyPatchTo"))
							{
							// Selet patch to copy to
							int selectedPatch = DoPatchSelect(renderer, mqEngine, mqGUI);
							if (selectedPatch > -1)
								{
								// Send "CopyTo" message to the synth engine
								// Use "undefined" CC message 102, data = 2
								char outBuffer[MSG_MAX_SIZE] = { 0xB0, 102, 2, (char)selectedPatch };
								PostMessage(mqEngine, outBuffer, 4);
								// Give synthengine some time to formulate and queue a response
								SDL_Delay(50);
								// TODO : Change to the destination patch?
								gm.m_controlChanged = false;
								}
							}
						else if (0 == strcmp(control->m_name, "FilterCutoff"))
							{
							// Use "filter cutoff" CC message 74
							unsigned char value = (unsigned char)(((CGUISlider *)control)->m_value * 0x7F);
							char outBuffer[MSG_MAX_SIZE] = { 0xB0, 74, value };
							PostMessage(mqEngine, outBuffer, 3);
							gm.m_controlChanged = false;
							}
						else if (0 == strcmp(control->m_name, "SelectPatch"))
							{
							// Show Patch Select screen
							int selectedPatch = DoPatchSelect(renderer, mqEngine, mqGUI);
							// Send patch select message to the engine
							mqbuffer[0] = 0xC0;
							mqbuffer[1] = (char)selectedPatch;
							mqbuffer[2] = 0;
							printf("Posting Patch %d Select message...\n", selectedPatch);
							PostMessage(mqEngine, mqbuffer, 3);
							// Give synthengine some time to formulate and queue a response
							SDL_Delay(50);
							// Ask the synth engine for current patch info
							// (Not neccessary - synth engine sends selected patch data anyway)
							//RequestPatchData(mqEngine);
							// Avoid sending patch data to the engine before we get it back!
							gm.m_controlChanged = false;
							}
						else if (0 == strcmp(control->m_name, "RvbDepth"))
							{
							// Use "Reverb Send" CC message 91
							unsigned char value = (unsigned char)(((CGUISlider *)control)->m_value * 0x7F);
							char outBuffer[MSG_MAX_SIZE] = { 0xB0, 91, value };
							PostMessage(mqEngine, outBuffer, 3);
							gm.m_controlChanged = false;
							}
						// trigger redraw
						redraw = true;
						}
					}
					break;
				//case SDL_FINGERDOWN :
					//printf("Finger down: %d, %d\n", event.tfinger.x, event.tfinger.y);
					//gm.OnMouseDown(event.tfinger.x, event.tfinger.y);
					//break;
				}	// end switch
			}	// wend event

		// If a control has changed, then send patch data to synth
		if (gm.m_controlChanged)
			{
			printf("Control changed!\n");
			UpdatePatchFromMainPage(gm, workPatch);
			// Use Patch::PackParams() to create data for message
			//        to send
			char outBuffer[MSG_MAX_SIZE] = {
				0xF0, 0x7D, 0x01, 0x01, 0x12, 0x00, 0x00, PADDR_END, 0x00, 0xF7
				} ;
			char *p = outBuffer + 8; 
			int packedBytes = workPatch.PackParams(p);
			PostMessage(mqEngine, outBuffer, packedBytes + 10);
			// reset changed flag
			gm.m_controlChanged = false;
			// trigger redraw
			redraw = true;
			}
			
		// Check for data from the synthengine process
		bytes_read = mq_receive(mqGUI, mqbuffer, MSG_MAX_SIZE, NULL);
		if (bytes_read > 0)
			{
			mqbuffer[bytes_read] = 0;
			//printf("msg received: %s\n", mqbuffer);
			ProcessMessage(mqbuffer, bytes_read);
			UpdateMainPageFromPatch(gm, workPatch);
			redraw = true;
			}

		// Redraw if neccessary
		if (redraw)
			{
			gm.DrawAllControls();
			//SDL_Flip(screen);
			SDL_RenderPresent(renderer);
			}
			
		SDL_Delay(10);
		}

	printf("GUI finished.\n");
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	// Tidy up mqueue stuff
	mq_close(mqEngine);
	mq_close(mqGUI);
	//mq_unlink(MQUEUE_NAME);    // only server (creator) has to do this

	Touch_Close();
	
	return 0;
}
