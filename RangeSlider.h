#pragma once

#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif // !WX_PRECOMP

#include <wx/dcbuffer.h>

enum ThumbType
{
	LOW,
	HIGH,
};

class SliderThumb;

class RangeSlider : public wxPanel
{
public:

	explicit RangeSlider(wxWindow* parent,
						 wxWindowID id = wxID_ANY,
						 const wxPoint& pos = wxDefaultPosition,
						 const wxSize& size = wxDefaultSize,
						 long style = wxSL_HORIZONTAL,
						 const wxString& name = wxPanelNameStr);
	
	void PaintEvent(wxPaintEvent& event);
	void Render(wxDC& dc);

	int GetMax() { return max_value; };
	int GetMin() { return min_value; };

	void OnMouseDown(wxMouseEvent& event);
	void OnMouseUp(wxMouseEvent& event);
	void OnMouseLost(wxMouseCaptureLostEvent& event);
	void OnMouseMotion(wxMouseEvent& event);
	void OnMouseEnter(wxMouseEvent& event);
	void OnMouseLeave(wxMouseEvent& event);
	void OnResize(wxSizeEvent& event);

	const int border_width = 8;
	wxVector<SliderThumb> thumbs;

private:

	SliderThumb* selected_thumb = nullptr;

	int min_value = 0;
	int max_value = 10000;

	wxColor slider_background_color	= wxColor(231, 234, 234);
	wxColor slider_outline			= wxColor(14, 14, 14);
	wxColor selected_range_color	= wxColor(0, 120, 215);
	wxColor selected_range_outline	= wxColor(0, 120, 215);
};

class SliderThumb
{
public:
	SliderThumb(RangeSlider& parent, int value, ThumbType type);
	
	bool IsMouseOver(wxPoint& click_pos);
	void Render(wxDC& dc);

	wxPoint GetPosition();
	void SetPosition(wxPoint mouse_position);
	const int GetThumbValue() const { return value_; };
	const ThumbType GetType() const { return type_; };
	const wxSize GetSize() const { return size_; };
	int GetMin();
	int GetMax();

	bool mouse_over = false;
	

private:

	RangeSlider* parent_;
	float value_ = 0.0;
	ThumbType type_;
	wxSize size_;
	

	wxPoint thumb_poly[5] = { wxPoint(0,0), wxPoint(0,13), wxPoint(5,18), wxPoint(10,13), wxPoint(10,0) };
	wxPoint thumb_shadow_poly[4] = { wxPoint(0,14), wxPoint(4,18), wxPoint(6,18), wxPoint(10,14) };

	
	wxColor normal_color		= wxColor(0, 120, 215);
	wxColor normal_shadow		= wxColor(120, 180, 228);
	wxColor dragged_color		= wxColor(204, 204, 204);
	wxColor dragged_shadow		= wxColor(222, 222, 222);
	wxColor mouse_over_color	= wxColor(100, 100, 100);
	wxColor mouse_over_shadow	= wxColor(132, 132, 132);

};