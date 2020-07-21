#include <gluon/api/gln_renderer.h>
#include <gluon/api/gln_interpolate.h>
#include <gluon/api/gln_application.h>

// struct application : public gluon::application
// {
// 	application()
// 	    : gluon::application("Button example")
// 	{
// 		m_ButtonPos  = (gluon::vec2)GetSize();
// 		m_ButtonSize = gluon::vec2(128.f, 54.f);
// 	}

// 	virtual void OnUpdate() override final
// 	{
// 		gluon::color Color = gluon::MakeColorFromRGB32(0x9E5727);

// 		if (m_Pressed)
// 		{
// 			Color = gluon::MakeColorFromRGB32(0x485D8B);
// 		}
// 		else if (m_Hovered)
// 		{
// 			Color = gluon::MakeColorFromRGB32(0x8A6B93);
// 		}

// 		gluon::DrawRectangle(m_ButtonPos.x, m_ButtonPos.y, m_ButtonSize.x, m_ButtonSize.y, Color, 0.0f);

// 		gluon::SetFont("Roboto");
// 		gluon::DrawText(U"Hello, World !", 24, m_ButtonPos.x - 60, m_ButtonPos.y - 5, gluon::MakeColorFromRGB8(0, 0, 0));
// 	}

// 	void OnMouseMove(f32 X, f32 Y) override final
// 	{
// 		m_Hovered                  = false;
// 		const gluon::vec2 HalfSize = m_ButtonSize / 2.0f;
// 		if (X >= m_ButtonPos.x - HalfSize.x && X <= m_ButtonPos.x + HalfSize.x)
// 		{
// 			if (Y >= m_ButtonPos.y - HalfSize.y && Y <= m_ButtonPos.y + HalfSize.y)
// 			{
// 				m_Hovered = true;
// 			}
// 		}
// 	}

// 	virtual void OnMouseEvent(gluon::input_mouse_buttons Button, gluon::input_actions Action, gluon::input_mods Mods) override final
// 	{
// 		if (m_Hovered)
// 		{
// 			if (Button == gluon::MouseButton_Left)
// 			{
// 				if (Action == gluon::Action_Press)
// 				{
// 					m_Pressed = true;
// 				}
// 				else if (Action == gluon::Action_Release)
// 				{
// 					m_Pressed = false;
// 				}
// 			}
// 		}
// 	}

// 	virtual void OnResize(i32 Width, i32 Height) override final { m_ButtonPos = {Width / 2.0f, Height / 2.0f}; }

// 	gluon::vec2 m_ButtonPos;
// 	gluon::vec2 m_ButtonSize;

// 	bool m_Hovered = false;
// 	bool m_Pressed = false;
// };

i32 main()
{
	gluon::application App;
	return App.Run();
}
