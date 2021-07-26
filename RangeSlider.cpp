#include "RangeSlider.h"

#include <wx/dcbuffer.h>

#include <algorithm>


float fraction_to_value(float fraction, int min_value, int max_value)
{
	return (max_value - min_value) * fraction + min_value;
}

float value_to_fraction(float value, int min_value, int max_value)
{
	return float(value - min_value) / (max_value - min_value);
}


RangeSlider::RangeSlider(wxWindow* parent, wxWindowID id,const wxPoint& pos ,const wxSize& size ,long style ,const wxString& name) 
	: wxPanel(parent, id, pos,size, style, name)
{

	SetMinSize(wxSize(std::max({50, size.GetX()}), std::max({26, size.GetY()})));

	SliderThumb low_thumb(*this, min_value, ThumbType::LOW);
	SliderThumb high_thumb(*this, max_value, ThumbType::HIGH);

	thumbs.push_back(low_thumb);
	thumbs.push_back(high_thumb);

	Bind(wxEVT_LEFT_DOWN, &RangeSlider::OnMouseDown, this, wxID_ANY);
	Bind(wxEVT_LEFT_UP, &RangeSlider::OnMouseUp, this, wxID_ANY);
	Bind(wxEVT_MOTION, &RangeSlider::OnMouseMotion, this, wxID_ANY);
	Bind(wxEVT_MOUSE_CAPTURE_LOST, &RangeSlider::OnMouseLost, this, wxID_ANY);
	Bind(wxEVT_ENTER_WINDOW, &RangeSlider::OnMouseEnter, this, wxID_ANY);
	Bind(wxEVT_LEAVE_WINDOW, &RangeSlider::OnMouseLeave, this, wxID_ANY);
	Bind(wxEVT_PAINT, &RangeSlider::PaintEvent, this, wxID_ANY);
	Bind(wxEVT_SIZE, &RangeSlider::OnResize, this, wxID_ANY);
}

void RangeSlider::PaintEvent(wxPaintEvent& event)
{
	// depending on your system you may need to look at double-buffered dcs
	wxBufferedPaintDC dc(this);
	Render(dc);

	event.Skip();
}

void RangeSlider::Render(wxDC& dc)
{
	
	auto background_brush = wxBrush(GetBackgroundColour(), wxBRUSHSTYLE_SOLID);
	dc.SetBackground(background_brush);
	dc.Clear();
	
	int track_height = 12;
	auto w = GetSize().GetX();
	auto h = GetSize().GetY();
	
	dc.SetPen(wxPen(slider_outline, 1, wxPENSTYLE_SOLID));
	dc.SetBrush(wxBrush(slider_background_color, wxBRUSHSTYLE_SOLID));
	dc.DrawRectangle(border_width, (h - track_height) / 2, w - 2 * border_width, track_height);
	
	if (IsEnabled())
	{
		dc.SetPen(wxPen(selected_range_outline, 1, wxPENSTYLE_SOLID));
		dc.SetBrush(wxBrush(selected_range_color, wxBRUSHSTYLE_SOLID));
	}
	else
	{
		dc.SetPen(wxPen(slider_outline, 1, wxPENSTYLE_SOLID));
		dc.SetBrush(wxBrush(slider_outline, wxBRUSHSTYLE_SOLID));
	}

	auto low_thumb_pos = thumbs[0].GetPosition().x;

	auto high_thumb_pos = thumbs[1].GetPosition().x;
	dc.DrawRectangle(low_thumb_pos, h / 2 - track_height / 4, high_thumb_pos - low_thumb_pos, track_height / 2);

	for (auto& thumb : thumbs)
	{
		thumb.Render(dc);
	}

}

void RangeSlider::OnMouseDown(wxMouseEvent& event)
{
	
	if (!IsEnabled())
	{
		return;
	}

	auto click_pos = event.GetPosition();
	for (auto& thumb : thumbs)
	{
		if (thumb.IsMouseOver(click_pos))
		{
			selected_thumb = &thumb;
			break;
		}
	}

	CaptureMouse();
	Refresh();

}

void RangeSlider::OnMouseUp(wxMouseEvent& event)
{
	
	if (!IsEnabled())
	{
		return;
	}
	selected_thumb = nullptr;

	if (HasCapture())
	{
		ReleaseMouse();
	}
}

void RangeSlider::OnMouseLost(wxMouseCaptureLostEvent& event)
{
	event.Skip();
}

void RangeSlider::OnMouseMotion(wxMouseEvent& event)
{
	
	if (!IsEnabled())
	{
		return;
	}

	bool refresh_needed = false;
	wxPoint mouse_position = event.GetPosition();

	if(selected_thumb)
	{

		if (selected_thumb->GetType() == ThumbType::LOW)
		{
			if(mouse_position.x + selected_thumb->GetSize().x > thumbs[1].GetPosition().x)
			{
				// Then Mouse position equals the last known good thumb pos.
				mouse_position = selected_thumb->GetPosition();
			}
		}
		else
		{
			if (mouse_position.x - selected_thumb->GetSize().x < thumbs[0].GetPosition().x)
			{
				// Then Mouse position equals the last known good thumb pos.
				mouse_position = selected_thumb->GetPosition();
			}
		}
		
		selected_thumb->SetPosition(mouse_position);
		refresh_needed = true;
	}
	else
	{
		
		for (auto& thumb : thumbs)
		{
			bool old_mouse_over = thumb.mouse_over;
			thumb.mouse_over = thumb.IsMouseOver(mouse_position);
			if (old_mouse_over != thumb.mouse_over)
			{
				refresh_needed = true;
			}
		}
	}
	
	if (refresh_needed)
	{
		Refresh();
	}

}

void RangeSlider::OnMouseEnter(wxMouseEvent& event)
{
	event.Skip();
}

void RangeSlider::OnMouseLeave(wxMouseEvent& event)
{
	if (!IsEnabled())
	{
		return;
	}

	for (auto& thumb : thumbs)
	{
		thumb.mouse_over = false;
	}
	
	Refresh();
	
}

void RangeSlider::OnResize(wxSizeEvent& event)
{
	Refresh();
}



SliderThumb::SliderThumb(RangeSlider& parent, int value, ThumbType type)
{
	parent_ = &parent;
	value_ = value;
	type_ = type;
	size_ = wxSize(10, 18);
}

void SliderThumb::Render(wxDC& dc)
{
	wxColor thumb_color;
	wxColor thumb_shadow_color;
	if (!parent_->IsEnabled())
	{
		thumb_color = dragged_color;
		thumb_shadow_color = dragged_shadow;
	}
	else if (mouse_over)
	{
		thumb_color = mouse_over_color;
		thumb_shadow_color = mouse_over_shadow;
	}
	else
	{
		thumb_color = normal_color;
		thumb_shadow_color = normal_shadow;
	}

	auto thumb_pos = GetPosition();

	dc.SetBrush(wxBrush(thumb_shadow_color, wxBRUSHSTYLE_SOLID));
	dc.SetPen(wxPen(thumb_shadow_color, 1,  wxPENSTYLE_SOLID));
	dc.DrawPolygon(WXSIZEOF(thumb_shadow_poly), thumb_shadow_poly, thumb_pos.x - size_.x / 2, thumb_pos.y - size_.y / 2);
	
	dc.SetBrush(wxBrush(thumb_color, wxBRUSHSTYLE_SOLID));
	dc.SetPen(wxPen(thumb_color, 1, wxPENSTYLE_SOLID));
	dc.DrawPolygon(WXSIZEOF(thumb_poly), thumb_poly, thumb_pos.x - size_.x / 2, thumb_pos.y - size_.y / 2);
}

wxPoint SliderThumb::GetPosition()
{
	auto min_x = GetMin();
	auto max_x = GetMax();
	auto parent_size = parent_->GetSize();
	auto min_value = parent_->GetMin();
	auto max_value = parent_->GetMax();
	auto fraction = value_to_fraction(value_, min_value, max_value);
	wxPoint pos(fraction_to_value(fraction, min_x, max_x), parent_size.GetY() / 2 + 1);

	return pos;
}

bool SliderThumb::IsMouseOver(wxPoint& click_pos)
{
	auto pos = GetPosition();

	auto boundary_low = pos.x - size_.x / 2;
	auto boundary_high = pos.x + size_.x / 2;
	mouse_over =  (click_pos.x >= boundary_low && click_pos.x <= boundary_high);

	return mouse_over;

}
void SliderThumb::SetPosition(wxPoint mouse_position)
{
	auto pos_x = mouse_position.x;

	pos_x = std::min(std::max(pos_x, GetMin()), GetMax());
	auto fraction = value_to_fraction(pos_x, GetMin(), GetMax());
	value_ = fraction_to_value(fraction, parent_->GetMin(), parent_->GetMax());
	
}

int SliderThumb::GetMin()
{
	return parent_->border_width + size_.x / 2;
}

int SliderThumb::GetMax()
{
	auto parent_size = parent_->GetSize();
	return (parent_size.GetX() - parent_->border_width - size_.x / 2);
}
