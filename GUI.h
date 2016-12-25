/// GUI class for SDL projects
/// Copyright James Higgs 2016
#pragma once
#include <SDL2/SDL.h>
#include <vector>
#include <string>

class FontEngine;
class CGUIManager;

/// Draw "context" for controls to use when drawing themselves 
class CGUIDrawContext
{
public:
	CGUIDrawContext(CGUIManager *gm, SDL_Renderer *renderer, FontEngine *font)
		: 	m_gm(gm),
			m_renderer(renderer),
			m_font(font),
			m_drawShadow(false),
			m_dirty(true)
		{
		SetForeColour(255, 255, 255, 0);
		SetTextColour(32, 192, 32, 0);
		SetBackColour(0, 0, 0, 0);
		};

	void SetForeColour(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
		m_foreColour.r = r;
		m_foreColour.g = g;
		m_foreColour.b = b;
		m_foreColour.a = a;
		}

	void SetTextColour(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
		m_textColour.r = r;
		m_textColour.g = g;
		m_textColour.b = b;
		m_textColour.a = a;
		}
		
	void SetBackColour(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
		{
		m_backColour.r = r;
		m_backColour.g = g;
		m_backColour.b = b;
		m_backColour.a = a;
		}
	// Set renderer's current draw colour
	void SetDrawColour(const SDL_Colour &colour) const
		{
		SDL_SetRenderDrawColor(m_renderer, colour.r, colour.g, colour.b, colour.a);
		}
	
	CGUIManager *m_gm;			// The GUIManager that this drawcontext "belongs" to
	SDL_Renderer *m_renderer;
	FontEngine *m_font;			// Text rendering engine
	SDL_Colour m_foreColour;	// Foreground colour
	SDL_Colour m_textColour;	// Text (font) colouring
	SDL_Colour m_backColour;	// Background colour
	bool m_drawShadow;
	bool m_dirty;				// Whether "page" is dirty
};

enum CONTROLTYPE {
	CT_LABEL = 1,
	CT_EDIT,
	CT_BUTTON,
	CT_SLIDER,
	CT_ENVELOPE,
	CT_OPTIONLIST
};

/// Base class for controls
class CGUIControl
{
public:
	CGUIControl(int x, int y, int width, int height, unsigned short type, const char *name, const char *text);

	/// Mouse/touch click handler
	virtual int OnClick(const CGUIDrawContext &drawContext, int x, int y) = 0;
	/// Mouse drag / swipe handler
	virtual int OnSwipe(int x, int y, int dx, int dy) = 0;
	/// Draw this control in its current state
	virtual void Draw(const CGUIDrawContext &drawContext) = 0;		// TODO : state?
	/// Get extents of this control
	SDL_Rect GetRect() const;
	/// Get/set the text value in this control
	char *GetText() const;
	void SetText(const char *text);
	/// control rendering functions
	void DrawBGFill(const CGUIDrawContext &drawContext) const;
	void DrawFrame(const CGUIDrawContext &drawContext) const;
	void DrawLine(const CGUIDrawContext &drawContext, int x1, int y1, int x2, int y2) const;
	void DrawText(const CGUIDrawContext &drawContext, int x, int y, const char *text) const;

	unsigned short m_type;
	char m_name[64];
	SDL_Rect m_rect;
	char m_text[256];
};

/// Text label control
class CGUILabel : public CGUIControl
{
public:
	CGUILabel(int x, int y, int width, int height, const char *name, const char *text);

	int OnClick(const CGUIDrawContext &drawContext, int x, int y) { return 0; };
	int OnSwipe(int x, int y, int dx, int dy) { return 0; };
	void Draw(const CGUIDrawContext &drawContext);		// TODO : state?
};

/// Text edit control
class CGUIEdit : public CGUIControl
{
public:
	CGUIEdit(int x, int y, int width, int height, const char *name, const char *text);

	int EditText(const CGUIDrawContext &drawContext);
	int OnClick(const CGUIDrawContext &drawContext, int x, int y);
	int OnSwipe(int x, int y, int dx, int dy);
	void Draw(const CGUIDrawContext &drawContext);		// TODO : state?
};

/// Button control
class CGUIButton : public CGUIControl
{
public:
	CGUIButton(int x, int y, int width, int height, const char *name, const char *text);

	int OnClick(const CGUIDrawContext &drawContext, int x, int y);
	int OnSwipe(int x, int y, int dx, int dy);
	void Draw(const CGUIDrawContext &drawContext);		// TODO : state?
};

/// Slider control
class CGUISlider : public CGUIControl
{
public:
    CGUISlider(int x, int y, int width, int height, const char *name, float minVal, float maxVal, bool logScale);

    // Set up the slider for linear or log operation
  	void Init(float value, float minVal, float maxVal, bool logScale);
	int EditSlider(const CGUIDrawContext &drawContext);
	int OnClick(const CGUIDrawContext &drawContext, int x, int y);
	int OnSwipe(int x, int y, int dx, int dy);
	void Draw(const CGUIDrawContext &drawContext);		// TODO : state?

    // Get the value of the slider at the current position
    float GetValue(int pos);
    /// Get the position correspondingt o the current intenal m_value
    int GetPosition();
	
	float m_value;
	float m_minVal;
	float m_maxVal;
    bool m_logScale;               // Slider uses logarithmic scaling
};

/// Envelope control
class CGUIEnvelope : public CGUIControl
{
public:
	CGUIEnvelope(int x, int y, int width, int height, const char *name, const char *text);

	int EditADSR(const CGUIDrawContext &drawContext);
	int OnClick(const CGUIDrawContext &drawContext, int x, int y);
	int OnSwipe(int x, int y, int dx, int dy);
	void Draw(const CGUIDrawContext &drawContext);		// TODO : state?
	void SetADSR(float delay, float attack, float peak, float decay, float sustain, float release);
	void GetADSR(float &delay, float &attack, float &peak, float &decay, float &sustain, float &release) const;

	float m_adsr[6];
};

/// Option list control
class CGUIOptionList : public CGUIControl
{
public:
	CGUIOptionList(int x, int y, int width, int height, const char *name, const char *text);

	int OnClick(const CGUIDrawContext &drawContext, int x, int y);
	int OnSwipe(int x, int y, int dx, int dy);
	void Draw(const CGUIDrawContext &drawContext);		// TODO : state?
	void AddOption(const char *option);
	short GetSelectedIndex() const;
	void SetSelectedIndex(short index);

	std::vector<std::string> m_optionArray;
	short m_selectedIndex;
};


/// Control Manager
class CGUIManager
{
public:
	explicit CGUIManager(SDL_Rect rect);

	CGUIControl *AddControl(int x, int y, int w, int h, CONTROLTYPE type, const char *name, const char *text);
	CGUILabel *AddLabel(int x, int y, int w, int h, const char *name, const char *text);
	CGUIButton *AddButton(int x, int y, int w, int h, const char *name, const char *text);
	CGUISlider *AddSlider(int x, int y, int w, int h, const char *name, const char *text);

	CGUIControl *GetControl(const char *name) const;
	CGUIControl *GetControl(int x, int y) const;
	void ArrangeControls(int width, int height);
	void DrawControl(const char *name);
	void DrawAllControls();
	void OnMouseDown(int x, int y);
	CGUIControl *OnMouseMove(int x, int y);
	CGUIControl *OnMouseUp(int x, int y);
	CGUIControl *ProcessClick(int x, int y);
	CGUIControl *ProcessSwipe(int x, int y, int dx, int dy);

	CGUIDrawContext m_drawContext;
	std::vector<CGUIControl *> m_controls;
	SDL_Rect m_rect;
	int m_margin;
	int m_dragStartX;                     // For dragging detection/operation
	int m_dragStartY;
	int m_dragLastX;
	int m_dragLastY;
	bool m_controlChanged;                // set if a control has changed
    CGUIControl *m_highlightedControl;    // For unhighlighting the highlighted control
};

