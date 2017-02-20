/*
    src/button.cpp -- [Normal/Toggle/Radio/Popup] Button widget

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/
NAMESPACE_BEGIN(nanogui)

// ctor
Button::Button(Widget * parent, const std::string & caption, int icon)
	: Widget(parent), mCaption(caption), mIcon(icon),
	mIconPosition(IconPosition::LeftCentered), mPushed(false),
	mFlags(NormalButton), mBackgroundColor(Colour(0, 0)),
	mTextColor(Colour(0, 0))
{
}

//// dtor
//Button::~Button()
//{
//}

ivec2 Button::preferredSize(NVGcontext * ctx) const
{
	int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
	nvgFontSize(ctx, fontSize);
	nvgFontFace(ctx, "sans-bold");
	float tw = nvgTextBounds(ctx, 0, 0, mCaption.c_str(), nullptr, nullptr);
	float iw = 0.0f, ih = fontSize;
	if (mIcon)
	{
		if (nvgIsFontIcon(mIcon))
		{
			ih *= 1.5f;
			nvgFontFace(ctx, "icons");
			nvgFontSize(ctx, ih);
			iw = nvgTextBounds(ctx, 0, 0, utf8(mIcon).data(), nullptr, nullptr)
				+ mSize.y * 0.15f;
		}
		else
		{
			int w, h;
			ih *= 0.9f;
			nvgImageSize(ctx, mIcon, &w, &h);
			iw = w * ih / h;
		}
	}
	return ivec2((int)(tw + iw) + 20, fontSize + 10);
}

bool Button::mouseButtonEvent(const ivec2 & p, int button, bool down, int modifiers)
{
	Widget::mouseButtonEvent(p, button, down, modifiers);
	/* Temporarily increase the reference count of the button in case the
	button causes the parent window to be destructed */
	ref<Button> self = this;
	if (button == 0 && mEnabled)
	{
		bool pushedBackup = mPushed;
		if (down)
		{
			if (mFlags & RadioButton)
			{
				if (mButtonGroup.empty())
				{
					for (auto widget : parent()->children())
					{
						Button * b = dynamic_cast<Button *> (widget);
						if (b != this && b && b->flags() & RadioButton)
							b->mPushed = false;
					}
				}
				else
				{
					for (auto b : mButtonGroup)
					{
						if (b != this && b->flags() & RadioButton)
							b->mPushed = false;
					}
				}
			}
			if (mFlags & PopupButton)
			{
				for (auto widget : parent()->children())
				{
					Button * b = dynamic_cast<Button *> (widget);
					if (b != this && b && b->flags() & PopupButton)
						b->mPushed = false;
				}
			}
			if (mFlags & ToggleButton)
				mPushed = !mPushed;
			else
				mPushed = true;
		}
		else
			if (mPushed)
			{
				if (contains(p) && mCallback)
					mCallback();
				if (mFlags & NormalButton)
					mPushed = false;
			}
		if (pushedBackup != mPushed && mChangeCallback)
			mChangeCallback(mPushed);
		return true;
	}
	return false;
}

void Button::draw(NVGcontext * ctx)
{
	Widget::draw(ctx);
	NVGcolor gradTop = mTheme->mButtonGradientTopUnfocused;
	NVGcolor gradBot = mTheme->mButtonGradientBotUnfocused;
	if (mPushed)
	{
		gradTop = mTheme->mButtonGradientTopPushed;
		gradBot = mTheme->mButtonGradientBotPushed;
	}
	else
		if (mMouseFocus && mEnabled)
		{
			gradTop = mTheme->mButtonGradientTopFocused;
			gradBot = mTheme->mButtonGradientBotFocused;
		}
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, mPos.x + 1, mPos.y + 1.0f, mSize.x - 2,
		mSize.y - 2, mTheme->mButtonCornerRadius - 1);
	if (mBackgroundColor.a != 0)
	{
		nvgFillColor(ctx, Colour(mBackgroundColor.r, mBackgroundColor.g, mBackgroundColor.b, 1.f));
		nvgFill(ctx);
		if (mPushed)
			gradTop.a = gradBot.a = 0.8f;
		else
		{
			double v = 1 - mBackgroundColor.a;
			gradTop.a = gradBot.a = mEnabled ? v : v * .5f + .5f;
		}
	}
	NVGpaint bg = nvgLinearGradient(ctx, mPos.x, mPos.y, mPos.x,
		mPos.y + mSize.y, gradTop, gradBot);
	nvgFillPaint(ctx, bg);
	nvgFill(ctx);
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, mPos.x + 0.5f, mPos.y + (mPushed ? 0.5f : 1.5f), mSize.x - 1,
		mSize.y - 1 - (mPushed ? 0.0f : 1.0f), mTheme->mButtonCornerRadius);
	nvgStrokeColor(ctx, mTheme->mBorderLight);
	nvgStroke(ctx);
	nvgBeginPath(ctx);
	nvgRoundedRect(ctx, mPos.x + 0.5f, mPos.y + 0.5f, mSize.x - 1,
		mSize.y - 2, mTheme->mButtonCornerRadius);
	nvgStrokeColor(ctx, mTheme->mBorderDark);
	nvgStroke(ctx);
	int fontSize = mFontSize == -1 ? mTheme->mButtonFontSize : mFontSize;
	nvgFontSize(ctx, fontSize);
	nvgFontFace(ctx, "sans-bold");
	float tw = nvgTextBounds(ctx, 0, 0, mCaption.c_str(), nullptr, nullptr);
	vec2 center = vec2((float)mPos.x, (float)mPos.y) + vec2((float)mSize.x, (float)mSize.y) * 0.5f;
	vec2 textPos(center.x - tw * 0.5f, center.y - 1);
	NVGcolor textColor =
		mTextColor.a == 0 ? mTheme->mTextColor : mTextColor;
	if (!mEnabled)
		textColor = mTheme->mDisabledTextColor;
	if (mIcon)
	{
		auto icon = utf8(mIcon);
		float iw, ih = fontSize;
		if (nvgIsFontIcon(mIcon))
		{
			ih *= 1.5f;
			nvgFontSize(ctx, ih);
			nvgFontFace(ctx, "icons");
			iw = nvgTextBounds(ctx, 0, 0, icon.data(), nullptr, nullptr);
		}
		else
		{
			int w, h;
			ih *= 0.9f;
			nvgImageSize(ctx, mIcon, &w, &h);
			iw = w * ih / h;
		}
		if (mCaption != "")
			iw += mSize.y * 0.15f;
		nvgFillColor(ctx, textColor);
		nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
		vec2 iconPos = center;
		iconPos.y -= 1;
		if (mIconPosition == IconPosition::LeftCentered)
		{
			iconPos.x -= (tw + iw) * 0.5f;
			textPos.x += iw * 0.5f;
		}
		else
			if (mIconPosition == IconPosition::RightCentered)
			{
				textPos.x -= iw * 0.5f;
				iconPos.x += tw * 0.5f;
			}
			else
				if (mIconPosition == IconPosition::Left)
					iconPos.x = mPos.x + 8;
				else
					if (mIconPosition == IconPosition::Right)
						iconPos.x = mPos.x + mSize.x - iw - 8;
		if (nvgIsFontIcon(mIcon))
			nvgText(ctx, iconPos.x, iconPos.y + 1, icon.data(), nullptr);
		else
		{
			NVGpaint imgPaint = nvgImagePattern(ctx,
				iconPos.x, iconPos.y - ih / 2, iw, ih, 0, mIcon, mEnabled ? 0.5f : 0.25f);
			nvgFillPaint(ctx, imgPaint);
			nvgFill(ctx);
		}
	}
	nvgFontSize(ctx, fontSize);
	nvgFontFace(ctx, "sans-bold");
	nvgTextAlign(ctx, NVG_ALIGN_LEFT | NVG_ALIGN_MIDDLE);
	nvgFillColor(ctx, mTheme->mTextColorShadow);
	nvgText(ctx, textPos.x, textPos.y, mCaption.c_str(), nullptr);
	nvgFillColor(ctx, textColor);
	nvgText(ctx, textPos.x, textPos.y + 1, mCaption.c_str(), nullptr);
}

NAMESPACE_END(nanogui)



