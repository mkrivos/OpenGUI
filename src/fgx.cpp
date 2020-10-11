/*
	OpenGUI

	Copyright (C) 1996,2005  Marian Krivos

	This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

	You should have received a copy of the GNU Library General Public
	License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    nezmar@atlas.sk

*/

/* TODO:
	-Text width for editbox
	-window flags
	-selected/disabled ctrl
	-radiogroup
	-checkbox
	-text
	image
	-sliders
	-listbox
	color picker
	font chooser
	file chooser

*/

//---------------------------------------------------------------------------

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#include "fastgl.h"
#include "widgets.h"
#include "fglistbox.h"
#include "fggradient.h"

#include "fgx.h"

#include <tinyxml.h>

#define WIDGET			"widget"
#define TEXT			"text"
#define MENUBAR			"menubar"
#define MENU			"menu"
#define MENUPOPUP		"menupopup"
#define MENUITEM		"menuitem"
#define PUSHBUTTON		"pushbutton"
#define EDITBOX			"editbox"
#define RADIOBUTTON		"radiobutton"
#define RADIOGROUP		"radiogroup"
#define CHECKBOX		"checkbox"
#define SLIDEBAR		"slidebar"
#define LISTBOX			"listbox"

#define ID				"id"
#define LABEL			"label"
#define TITLE			"title"
#define FRAME			"frame"
#define SCRAMBLED		"scrambled"
#define HEXADECIMAL		"hexadecimal"
#define CLOSEONESCAPE	"closeonescape"
#define	MODAL			"modal"
#define	CENTER			"center"
#define	PERSISTENT		"persistent"
#define	RESIZE			"resize"
#define STATUSBAR       "statusbar"
#define GLCONTEXT       "glcontext"
#define STICKY          "sticky"
#define FOCUS         	"focus"
#define PICTOGRAMS    	"pictograms"
#define ONCLICK			"onclick"
#define POPUP			"popup"
#define HOTKEY			"hotkey"
#define DISABLE			"disable"
#define SELECTED		"selected"
#define FOREGROUND		"foreground"
#define BACKGROUND      "background"
#define HANDLER			"handler"
#define HORIZONTAL		"horizontal"
#define SPAN			"span"
#define MINIMUM			"minimum"
#define MAXIMUM			"maximum"
#define STEP			"step"
#define DROPDOWN		"dropdown"

#ifdef FG_NAMESPACE
namespace fgl {
#endif

// -----------------------------------------------------------------------------

XUIComponent::XUIComponent()
 : shape(0, 0, 0, 0)
{
	init();
}

XUIComponent::XUIComponent(const char* _id, const char* _label, int x, int y, int w, int h)
 : shape(x,y,w,h)
{
	init();
	SetId(_id);
	SetLabel(_label);
}

XUIComponent::XUIComponent(const XUIComponent& old)
 : shape(old.shape)
{
	init();
	shape = old.shape;
	if (old.label) label = strdup(old.label);
	if (old.id) id = strdup(old.id);
	strcpy(onclick, old.onclick);
}

void XUIComponent::init()
{
	label = 0;
	id = 0;
	onclick[0] = 0;
}

XUIComponent::~XUIComponent()
{
	if (id)
	{
		free((void *)id);
		id = 0;
	}
	if (label)
	{
		free((void *)label);
		label = 0;
	}
}

void XUIComponent::SetId(const char* name)
{
	if (id) free((void *)id);
	if (name) id = strdup(name);
	else id = 0;
}

void XUIComponent::SetLabel(const char* name)
{
	if (label) free((void *)label);
	if (name) label = strdup(name);
	else label = 0;
}

void XUIComponent::Save(TiXmlElement* doc)
{
	if (id && *id) doc->SetAttribute( ID, id);
	if (label) doc->SetAttribute( LABEL, label);

	if (shape.x != DEFAULT_VALUE)
	{
		doc->SetAttribute("x", shape.x);
	}
	if (shape.y != DEFAULT_VALUE)
	{
		doc->SetAttribute("y", shape.y);
	}
	if (shape.w != DEFAULT_VALUE)
	{
		doc->SetAttribute("w", shape.w);
	}
	if (shape.h != DEFAULT_VALUE)
	{
		doc->SetAttribute("h", shape.h);
	}
	if (onclick[0]) doc->SetAttribute( ONCLICK, onclick);
}

void XUIComponent::Load(TiXmlElement* doc)
{
	const char* str;

	str = doc->Attribute( ID );
	if (str) SetId(str);

	str = doc->Attribute( LABEL );
	if (str) SetLabel(str);

	doc->QueryIntAttribute("x", &shape.x);
	doc->QueryIntAttribute("y", &shape.y);
	doc->QueryIntAttribute("w", &shape.w);
	doc->QueryIntAttribute("h", &shape.h);

	str = doc->Attribute( ONCLICK );
	if (str) SetSignalName(str);
}

bool XUIComponent::LoadColor(TiXmlElement* doc, const char* name, FGColor& color)
{
	const char* ptr = doc->Attribute(name);
	if (ptr)
	{
		color = FGColor(ptr);
		return true;
	}
	return false;
}

void XUIComponent::SaveColor(TiXmlElement* doc, const char* name, FGColor& color)
{
	char* ptr = color.GetString();
	doc->SetAttribute(name, ptr);
	free(ptr);
}

void XUIComponent::SetSignalName(const char* name)
{
	strncpy(onclick, name, sizeof(onclick));
	onclick[sizeof(onclick)-1] = 0;
}

// -----------------------------------------------------------------------------

XUIComposite::XUIComposite()
{
}

XUIComposite::XUIComposite(const char* _id, const char* _label, int x, int y, int w, int h)
: XUIComponent(_id, _label, x, y, w, h)
{
}

XUIComposite::XUIComposite(const XUIComposite& old)
 : XUIComponent(old)
{
	CComponentIterator i = old.childs.begin();
	CComponentIterator e = old.childs.end();

	for( ; i!=e ; i++ )
	{
		AddComponent( *i );
	}
}

XUIComposite::~XUIComposite()
{
	ComponentIterator start = childs.begin();
	ComponentIterator end = childs.end();

	while(start != end)
	{
		XUIComponent* obj = *start;
		*start = 0;
		delete obj;
		start++;
	}
}

int XUIComposite::GetComponentCount()
{
	int sz=0;
#ifdef __BORLANDC__
	ComponentIterator start = childs.begin();
	ComponentIterator end = childs.end();

	while(start != end)
	{
		sz++;
		start++;
	}
#else
	sz = childs.size();
#endif
	return sz;
}

void XUIComposite::Save(TiXmlElement* doc)
{
	ComponentIterator start = childs.begin();
	ComponentIterator end = childs.end();

	XUIComponent::Save(doc);

	while(start != end)
	{
		XUIComponent* obj = *start;
		obj->Save(doc);
		start++;
	}
}

void XUIComposite::Load(TiXmlElement* doc)
{
	ComponentIterator start = childs.begin();
	ComponentIterator end = childs.end();

	XUIComponent::Load(doc);

	while(start != end)
	{
		XUIComponent* obj = *start;
		obj->Load(doc);
		start++;
	}
}

XUIComponent* XUIComposite::Clone()
{
	return new XUIComposite( * (XUIComposite *) this);
}

void XUIComposite::AddComponent(XUIComponent *component)
{
	XUIComponent* new_component = component->Clone();
	childs.push_back( new_component );
}

void XUIComposite::RemoveComponent(void)
{
	// fixme
}

void XUIComposite::Show(XUIComponent* parent)
{
	// show its buttons too
	ComponentIterator start = childs.begin();
	ComponentIterator end = childs.end();
	while(start != end)
	{
		(*start)->Show(parent);
		start++;
	}
}

XUIComponent* XUIComposite::FindObject(const char* ctrl_id)
{
	ComponentIterator start = childs.begin();
	ComponentIterator end = childs.end();

	while(start != end)
	{
		if (strcmp(ctrl_id, (*start)->GetId())==0)
			return *start;
		start++;
	}
	return 0;
}

// -----------------------------------------------------------------------------

XUIText::XUIText()
{
}

XUIText::XUIText(const char* _label, int x, int y, FGColor& _ink, FGColor& _paper)
 : XUIComponent(0, _label, x,y,0,0)
{
	ink = _ink;
	paper = _paper;
}

XUIText::~XUIText()
{
}

void XUIText::Save(TiXmlElement* doc)
{
	TiXmlElement item( TEXT );

	XUIComponent::Save(&item);
	SaveColor(&item, FOREGROUND, ink);
	SaveColor(&item, BACKGROUND, paper);
	doc->InsertEndChild(item);
}

void XUIText::Load(TiXmlElement* doc)
{
	XUIComponent::Load(doc);
	LoadColor(doc, FOREGROUND, ink);
	LoadColor(doc, BACKGROUND, paper);
}

XUIComponent* XUIText::Clone()
{
	return new XUIText( * (XUIText *) this);
}

void XUIText::Show(XUIComponent* parent)
{
	parent->GetParent()->AddText(shape.x, shape.y, label, ink, paper);
}

// -----------------------------------------------------------------------------

XUIControl::XUIControl()
 : XUIComponent()
{
	init();
}

XUIControl::XUIControl(const char* _id, const char* _label, int key, int x, int y, int w, int h)
 : XUIComponent(_id, _label, x, y, w, h)
{
	init();
	hotkey = key;
}

XUIControl::XUIControl(const XUIControl& old)
 : XUIComponent(old)
{
	hotkey = old.hotkey;
	disable = old.disable;
	selected = old.selected;
	control = 0;
}

void XUIControl::init()
{
	hotkey = 0;
	disable = false;
	selected = false;
	control = 0;
}

void XUIControl::FixControl(FGControl* ctrl)
{
	control = ctrl;

	if (selected)
		ctrl->GetOwner()->SetDefaultControl(ctrl);
	if (disable) ctrl->Disable();
	ctrl->AttachSignalName(onclick);	// assign the name of the event generated on button click
}

void XUIControl::Save(TiXmlElement* doc)
{
	XUIComponent::Save(doc);

	if (hotkey) doc->SetAttribute( HOTKEY, hotkey);
	if (disable) doc->SetAttribute( DISABLE, disable);
	if (selected) doc->SetAttribute( SELECTED, selected);
}

void XUIControl::Load(TiXmlElement* doc)
{
	XUIComponent::Load(doc);

	doc->QueryIntAttribute( SELECTED, &selected );
	doc->QueryIntAttribute( HOTKEY , &hotkey);
	doc->QueryIntAttribute( DISABLE, &disable );
}

// -----------------------------------------------------------------------------

XUIPushButton::XUIPushButton()
{
}

XUIPushButton::XUIPushButton(const char* _id, const char* _label, int key, int x, int y, int w, int h)
 : XUIControl(_id, _label, key, x,y,w,h)
{
}

void XUIPushButton::Save(TiXmlElement* doc)
{
	TiXmlElement item( PUSHBUTTON );
	XUIControl::Save(&item);
	doc->InsertEndChild(item);
}

void XUIPushButton::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);
}

XUIComponent* XUIPushButton::Clone()
{
	return new XUIPushButton( * (XUIPushButton *) this);
}

void XUIPushButton::Show(XUIComponent* parent)
{
	FGControl* ctrl = parent->GetParent()->AddPushButton(shape.x, shape.y, shape.w, shape.h, label, hotkey);
	FixControl(ctrl);
}

// -----------------------------------------------------------------------------

XUIEditBox::XUIEditBox()
{
	init();
}

XUIEditBox::XUIEditBox(const char* _id, const char* _label, int key, int x, int y, int w, int h)
 : XUIControl(_id, _label, key, x,y,w,h)
{
	init();
}

void XUIEditBox::init()
{
	text[0] = 0;
	high_precision = 0;
	integer = 0;
	inputtype = 0;
	scrambled = false;
	hexadecimal = false;
}

void XUIEditBox::Save(TiXmlElement* doc)
{
	TiXmlElement item( EDITBOX );

	XUIControl::Save(&item);

	if (scrambled) item.SetAttribute( SCRAMBLED, scrambled );
	if(hexadecimal) item.SetAttribute( HEXADECIMAL, hexadecimal );
	doc->InsertEndChild(item);
}

void XUIEditBox::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);

	doc->QueryIntAttribute( SCRAMBLED, &scrambled );
	doc->QueryIntAttribute( HEXADECIMAL, &hexadecimal);
}

XUIComponent* XUIEditBox::Clone()
{
	return new XUIEditBox( * (XUIEditBox *) this);
}

void XUIEditBox::Show(XUIComponent* parent)
{
	FGEditBox* ctrl=0;
	int textsize=0;

	if (label)
	{
		textsize = FGFontManager::textwidth(FGFONT_BUTTON, label)+8;
	}

	switch(inputtype)
	{
		case STRING:
			ctrl = parent->GetParent()->AddEditBox(shape.x-textsize, shape.y, textsize, shape.w, label, hotkey, text);
			break;
		case INT:
			ctrl = parent->GetParent()->AddEditBox(shape.x-textsize, shape.y, textsize, shape.w, label, hotkey, &integer);
			break;
		case DOUBLE:
			ctrl = parent->GetParent()->AddEditBox(shape.x-textsize, shape.y, textsize, shape.w, label, hotkey, &high_precision);
			break;
		default:
			assert(!"Unassigned Data & Type with EditBox!");
	}
	FixControl(ctrl);
	if (scrambled) ctrl->PasswdMode(scrambled);
	if (hexadecimal) ctrl->HexMode(hexadecimal);
}

bool XUIEditBox::SetData(const char data[], int size)
{
	inputtype = STRING;
	memset(text, 0, sizeof(text));
	strncpy(text, data, size< int(sizeof(text)-1) ? size : sizeof(text)-1);
	return true;
}

bool XUIEditBox::SetData(const int data)
{
	inputtype = INT;
	integer = data;
	return true;
}

bool XUIEditBox::SetData(const double data)
{
	inputtype = DOUBLE;
	high_precision = data;
	return true;
}

// -----------------------------------------------------------------------------

XUISlideBar::XUISlideBar()
{
	minimum = INT_MIN;
	maximum = INT_MAX;
	step = 1;
	horizontal = true;
	value = 0;
}

XUISlideBar::XUISlideBar(const char* _id, int x, int y, int min, int max, int _step)
 : XUIControl(_id, 0, 0, x, y, 0, 0)
{
	minimum = min;
	maximum = max;
	step = _step;
	horizontal = true;
	value = 0;
}

XUIComponent* XUISlideBar::Clone()
{
	return new XUISlideBar( * (XUISlideBar *) this);
}

void XUISlideBar::Show(XUIComponent* parent)
{
	FGControl* ctrl;

	if (horizontal)
		ctrl = parent->GetParent()->AddSlideBarH(shape.x, shape.y, minimum, maximum, step, &value);
	else
		ctrl = parent->GetParent()->AddSlideBarV(shape.x, shape.y, minimum, maximum, step, &value);

	FixControl(ctrl);
}

bool XUISlideBar::SetData(const int data)
{
	value = data;
	return true;
}

void XUISlideBar::Save(TiXmlElement* doc)
{
	TiXmlElement item( SLIDEBAR );
	XUIControl::Save(&item);
	if (horizontal) item.SetAttribute( HORIZONTAL, horizontal);
	if (step > 1) item.SetAttribute( STEP, step);
	if (minimum) item.SetAttribute( MINIMUM, minimum);
	item.SetAttribute( MAXIMUM, maximum);
	doc->InsertEndChild(item);
}

void XUISlideBar::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);
	doc->QueryIntAttribute( MINIMUM, &minimum );
	doc->QueryIntAttribute( MAXIMUM, &maximum );
	doc->QueryIntAttribute( STEP, &step );
	doc->QueryIntAttribute( HORIZONTAL, &horizontal);
}

// -----------------------------------------------------------------------------

XUICheckBox::XUICheckBox()
{
	integer = 0;
}

XUICheckBox::XUICheckBox(const char* _id, const char* _label, int key, int x, int y)
 : XUIControl(_id, _label, key, x, y, 0, 0)
{
	integer = 0;
}

XUIComponent* XUICheckBox::Clone()
{
	return new XUICheckBox( * (XUICheckBox *) this);
}

void XUICheckBox::Show(XUIComponent* parent)
{
	FGControl* ctrl = parent->GetParent()->AddCheckBox(shape.x, shape.y, label, hotkey, &integer);
	FixControl(ctrl);
}

bool XUICheckBox::SetData(const int data)
{
	integer = data;
	return true;
}

void XUICheckBox::Save(TiXmlElement* doc)
{
	TiXmlElement item( CHECKBOX );

	XUIControl::Save(&item);

	doc->InsertEndChild(item);
}

void XUICheckBox::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);
}

// -----------------------------------------------------------------------------

XUIRadioButton::XUIRadioButton()
{
	integer = 0;
}

XUIRadioButton::~XUIRadioButton()
{
}

XUIRadioButton::XUIRadioButton(const char* _id, const char* _label, int key, int x, int y)
 : XUIControl(_id, _label, key, x, y, 0, 0)
{
	integer = 0;
}

XUIComponent* XUIRadioButton::Clone()
{
	return new XUIRadioButton( * (XUIRadioButton *) this);
}

void XUIRadioButton::Show(XUIComponent* parent)
{
	FGControl* ctrl = parent->GetParent()->AddRadioButton(shape.x, shape.y, label, hotkey, &integer);
	FixControl(ctrl);
}

bool XUIRadioButton::SetData(const int data)
{
	integer = data;
	return true;
}

void XUIRadioButton::Save(TiXmlElement* doc)
{
	TiXmlElement item( RADIOBUTTON );

	XUIControl::Save(&item);
	doc->InsertEndChild(item);
}

void XUIRadioButton::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);
}

// -----------------------------------------------------------------------------

XUIListBox::XUIListBox()
{
	dropdown = 1;
}

XUIListBox::XUIListBox(const XUIListBox& old)
 : XUIControl(old)
{
	dropdown = old.dropdown;
}

XUIListBox::~XUIListBox()
{
}

XUIListBox::XUIListBox(const char* _id, int x, int y, int w, int h, int drop)
 : XUIControl(_id,0,0,x,y,w,h)
{
	dropdown = drop;
}

void XUIListBox::Save(TiXmlElement* doc)
{
	TiXmlElement item( LISTBOX );

	XUIControl::Save(&item);
	item.SetAttribute( DROPDOWN, dropdown);
	doc->InsertEndChild(item);
}

void XUIListBox::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);
	doc->QueryIntAttribute( DROPDOWN, &dropdown );
}

XUIComponent* XUIListBox::Clone()
{
	return new XUIListBox( * (XUIListBox *) this);
}

void XUIListBox::Show(XUIComponent* parent)
{
	FGControl* ctrl = new FGListBoxEx(shape.x, shape.y, shape.w, shape.h, dropdown, parent->GetParent(), 0,0,0);
	FixControl(ctrl);
}

bool XUIListBox::SetData(const char data[], int index)
{
	FGListBoxEx* lbptr = static_cast<FGListBoxEx*>(control);

	if (index == -1)
		lbptr->insert(data);
	else
		lbptr->insert(index, data);
	lbptr->Update();
	return true;
}

bool XUIListBox::SetData(const int data)
{
	FGListBoxEx* lbptr = static_cast<FGListBoxEx*>(control);
	lbptr->SetIndex(data);
	return true;
}

// -----------------------------------------------------------------------------

XUIRadioGroup::XUIRadioGroup()
{
	span = 20;
	horizontal = false;
	value = 0;
	GetTempName(cbname, sizeof(cbname));
}

XUIRadioGroup::XUIRadioGroup(const char* _id, const char* label[], int key[], int x, int y, int _span, bool _horizont)
 : XUIComposite(_id, 0, x, y, 0, 0)
{
	value = 0;
	span = _span;
	horizontal = _horizont;
	GetTempName(cbname, sizeof(cbname));

	while(*label)
	{
		XUIRadioButton radio(0, *label, *key, x, y);

		AddComponent(&radio);
		if (horizontal)
			x += span;
		else
			y += span;
		label++;
		key++;
	}
}

XUIRadioGroup::XUIRadioGroup(const XUIRadioGroup& old)
 : XUIComposite(old), group(old.group)
{
	value = old.value;
	span = old.span;
	horizontal = old.horizontal;
	strcpy(cbname, old.cbname);
}

XUIRadioGroup::~XUIRadioGroup()
{
	DeregisterSignal(cbname, (void *)callback, (void *)this );
}

void XUIRadioGroup::Save(TiXmlElement* doc)
{
	TiXmlElement item( RADIOGROUP );

	XUIComposite::Save(&item);

	if (span!=20) item.SetAttribute( SPAN, span);
	if (horizontal) item.SetAttribute( HORIZONTAL, horizontal);

	doc->InsertEndChild(item);
}

void XUIRadioGroup::Load(TiXmlElement* doc)
{
	XUIComposite::Load(doc);

	doc->QueryIntAttribute( SPAN, &span);
	doc->QueryIntAttribute( HORIZONTAL, &horizontal);

	for(TiXmlElement* element = doc->FirstChildElement(RADIOBUTTON); element; element = element->NextSiblingElement(RADIOBUTTON) )
	{
		XUIRadioButton object;
		object.Load(element);
		AddComponent( &object );
	}
}

XUIComponent* XUIRadioGroup::Clone()
{
	return new XUIRadioGroup( * (XUIRadioGroup *) this);
}

void XUIRadioGroup::Show(XUIComponent* parent)
{
	XUIComposite::Show(parent);

	group.Clear();

	ComponentIterator i = childs.begin();
	ComponentIterator e = childs.end();

	long index = 0;

	for( ; i!=e ; i++ )
	{
		XUIControl* ptr = static_cast<XUIControl*>(*i);
		group.AddToGroup(ptr->GetControl(), index==0);
		ptr->GetControl()->AttachSignalName(cbname);
		ptr->GetControl()->SetParameter((void*) index);
		index++;
	}
	RegisterOnClickSignal(cbname, callback, this );
}

bool XUIRadioGroup::SetData(const int data)
{
	ComponentIterator i = childs.begin();
	ComponentIterator e = childs.end();

	int index = 0;

	for( ; i!=e ; i++ )
	{
		XUIControl* ptr = static_cast<XUIControl*>(*i);
		if (index == data)
		{
			group.RefreshGroup(ptr->GetControl());
			group.RefreshGroup(ptr->GetControl());
			value = data;
			return true;
		}
		index++;
	}
	return false;
}

void XUIRadioGroup::callback(CallBack cb, void* data)
{
	XUIRadioGroup* This = static_cast<XUIRadioGroup*>(data);

	This->value = (long)cb->GetParameter();
	// delegate to handler
	CallClosure(This->onclick, cb, This->value);
}

// -----------------------------------------------------------------------------

XUIMenuItem::XUIMenuItem()
{
}

XUIMenuItem::XUIMenuItem(const char* _id, const char* _label, int key)
 : XUIControl(_id, _label, key, 0, 0, 0, 0)
{
}

XUIMenuItem::XUIMenuItem(const XUIMenuItem& old)
: XUIControl(old)
{
}

void XUIMenuItem::Save(TiXmlElement* doc)
{
	TiXmlElement item( MENUITEM );

	XUIControl::Save(&item);
	doc->InsertEndChild(item);
}

void XUIMenuItem::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);
}

XUIComponent* XUIMenuItem::Clone()
{
	return new XUIMenuItem( * (XUIMenuItem *) this);
}

void XUIMenuItem::Show(XUIComponent* parent)
{
	FGControl* ctrl = ((FGMenuWindow *)parent->GetParent())->AddMenu((char *)label, hotkey);
	FixControl(ctrl);
}

// -----------------------------------------------------------------------------

XUIMenu::XUIMenu()
{
	popup = new XUIPopupMenu;
}

XUIMenu::XUIMenu(const char* _id, const char* _label, int key)
 : XUIControl(_id, _label, key, 0, 0, 0, 0)
{
	popup = new XUIPopupMenu;
}

XUIMenu::XUIMenu(const XUIMenu& old)
: XUIControl(old)
{
	popup = old.popup->Clone();
}

XUIMenu::~XUIMenu()
{
	if (popup->GetId())
		DeregisterSignal(popup->GetId(), (void *)PullDownMenuActivator, (void *)this );
	delete popup;
	popup = 0;
}

void XUIMenu::Save(TiXmlElement* doc)
{
	TiXmlElement item( MENU );

	XUIControl::Save(&item);
	if (popup->GetId())
		item.SetAttribute( POPUP, popup->GetId());
	popup->Save(&item);		// for menu items
	doc->InsertEndChild(item);
}

void XUIMenu::Load(TiXmlElement* doc)
{
	XUIControl::Load(doc);
	doc = doc->FirstChildElement(MENUPOPUP);
	if (doc) popup->Load(doc);
}

void XUIMenu::AddComponent(XUIComponent *childmenu)
{
	delete popup;
	popup = childmenu->Clone();
}

XUIComponent* XUIMenu::Clone()
{
	return new XUIMenu( * (XUIMenu *) this);
}

void XUIMenu::PullDownMenuActivator(CallBack cb, void *ptr)
{
	XUIMenu* This = static_cast<XUIMenu*>(ptr);

	if (This->popup->GetId())
	{
		This->popup->Show(This);
	}
}

void XUIMenu::Show(XUIComponent* parent)
{
	FGControl* ctrl = parent->GetParent()->AddBaseMenu(label, hotkey);
	FixControl(ctrl);
	// reassign if pulldown is attached
	if (popup->GetId())
	{
		ctrl->AttachSignalName(popup->GetId());
		RegisterOnClickSignal(popup->GetId(), PullDownMenuActivator, this );
	}
}

// -----------------------------------------------------------------------------

XUIPopupMenu::XUIPopupMenu()
{
	parent = 0;
}

XUIPopupMenu::XUIPopupMenu(const char* _id, int x, int y)
 : XUIComposite(_id, 0, x, y, 0, 0)
{
	parent = 0;
}

XUIPopupMenu::XUIPopupMenu(const XUIPopupMenu& old)
 : XUIComposite(old)
{
	parent = 0;
}

XUIPopupMenu::~XUIPopupMenu()
{
	parent = 0;
}

void XUIPopupMenu::Save(TiXmlElement* doc)
{
	TiXmlElement item( MENUPOPUP );
	XUIComposite::Save(&item);
	doc->InsertEndChild(item);
}

void XUIPopupMenu::Load(TiXmlElement* doc)
{
	XUIComposite::Load(doc);

	for(TiXmlElement* element = doc->FirstChildElement(MENUITEM); element; element = element->NextSiblingElement(MENUITEM) )
	{
		XUIMenuItem object;
		object.Load(element);
		AddComponent( &object );
		// compute size of pulldown meny
		shape.h += 22;
		if (object.GetLabel())
		{
			int textlen = FGFontManager::textwidth(FGFONT_BUTTON, object.GetLabel())+16;
			if (textlen > shape.w)
				shape.w = textlen;
		}
	}
	if (shape.h) shape.h += 8;
}

XUIComponent* XUIPopupMenu::Clone()
{
	return new XUIPopupMenu( * (XUIPopupMenu *) this);
}

void XUIPopupMenu::Show(XUIComponent* )
{
	if (shape.x && shape.y)
		parent = new FGMenuWindow(shape.x, shape.y, shape.w, shape.h);
	else
		parent = new FGMenuWindow(shape.w, shape.h);
	XUIComposite::Show(this);
}

// -----------------------------------------------------------------------------

XUIMenuBar::XUIMenuBar()
{
}

XUIMenuBar::XUIMenuBar(const char* _id)
 : XUIComposite(_id, 0, 0, 0, 0, 0)
{
}

XUIMenuBar::~XUIMenuBar()
{
}

void XUIMenuBar::Save(TiXmlElement* doc)
{
	TiXmlElement item( MENUBAR );
	XUIComposite::Save(&item);
	doc->InsertEndChild(item);
}

void XUIMenuBar::Load(TiXmlElement* doc)
{
	XUIComposite::Load(doc);

	for(TiXmlElement* element = doc->FirstChildElement(MENU); element; element = element->NextSiblingElement(MENU) )
	{
		XUIMenu object;
		object.Load(element);
		AddComponent( &object );
	}
}

XUIComponent* XUIMenuBar::Clone()
{
	return new XUIMenuBar( * (XUIMenuBar *) this);
}

void XUIMenuBar::Show(XUIComponent* parent)
{
	XUIComposite::Show(parent);
}

// -----------------------------------------------------------------------------

XUIWindow::XUIWindow()
{
	init();
}

XUIWindow::XUIWindow(const char* _id, const char* _label, int x, int y, int w, int h)
 : XUIComposite(_id, _label, x,y,w,h)
{
	init();
}

XUIWindow::XUIWindow(const XUIWindow& old)
 : XUIComposite(old)
{
	init();

	parent = 0;
	foreground = old.foreground;
	background = old.background;
	strcpy(handler, old.handler);
	title = old.title;
	frame = old.frame;
	modal = old.modal;
	close_on_escape = old.close_on_escape;
	center = old.center;
	persistent = old.persistent;
	resize = old.resize;
	statusbar = old.statusbar;
	glcontext = old.glcontext;
	sticky = old.sticky;
	focus = old.focus;
	pictograms = old.pictograms;
	selectmode = old.selectmode;
	withmenu = old.withmenu;
}

void XUIWindow::init()
{
	foreground = (unsigned)FGColor::black;
	background = (unsigned)FGColor::white;
	handler[0] = 0;
	parent = 0;
	title = true;
	frame = true;
	focus = true;
	pictograms = true;
	close_on_escape = false;
	modal = false;
	center = false;
	persistent = false;
	resize = false;
	statusbar = false;
	glcontext = false;
	sticky = false;
	selectmode = false;
	withmenu = false;
}

XUIWindow::~XUIWindow()
{
	parent = 0;
}

void XUIWindow::Save(TiXmlElement* doc)
{
	TiXmlElement item( WIDGET );

	XUIComponent::Save(&item);

	SaveColor(&item, FOREGROUND, foreground);
	SaveColor(&item, BACKGROUND, background);

	if (handler[0]) item.SetAttribute( HANDLER, handler );

	if (title == false) item.SetAttribute( TITLE, title );
	if (frame == false) item.SetAttribute( FRAME, frame );
	if (focus == false) item.SetAttribute( FOCUS, focus );
	if (pictograms == false) item.SetAttribute( PICTOGRAMS, pictograms );

	if (modal == true) item.SetAttribute( MODAL, modal );
	if (close_on_escape == true) item.SetAttribute( CLOSEONESCAPE, close_on_escape );
	if (center == true) item.SetAttribute( CENTER, center );
	if (persistent == true) item.SetAttribute( PERSISTENT, persistent );
	if (resize == true) item.SetAttribute( RESIZE, resize );
	if (statusbar == true) item.SetAttribute( STATUSBAR, statusbar );
	if (glcontext == true) item.SetAttribute( GLCONTEXT, glcontext );
	if (sticky == true) item.SetAttribute( STICKY, sticky );

	XUIComposite::Save(&item);
	doc->InsertEndChild(item);
}

void XUIWindow::Load(TiXmlElement* doc)
{
	const char* ptr;

	XUIComponent::Load(doc);

	LoadColor(doc, FOREGROUND, foreground);
	LoadColor(doc, BACKGROUND, background);

	ptr = doc->Attribute(HANDLER);
	if (ptr)
	{
		strcpy(handler, ptr);
	}

	doc->QueryIntAttribute( TITLE, &title );
	doc->QueryIntAttribute( FRAME, &frame );
	doc->QueryIntAttribute( MODAL, &modal );
	doc->QueryIntAttribute( CLOSEONESCAPE, &close_on_escape );
	doc->QueryIntAttribute( CENTER, &center );
	doc->QueryIntAttribute( PERSISTENT, &persistent );
	doc->QueryIntAttribute( RESIZE, &resize );
	doc->QueryIntAttribute( STATUSBAR, &statusbar );
	doc->QueryIntAttribute( GLCONTEXT, &glcontext );
	doc->QueryIntAttribute( STICKY, &sticky );
	doc->QueryIntAttribute( FOCUS, &focus );
	doc->QueryIntAttribute( PICTOGRAMS, &pictograms );

	LoadChilds(doc);
}

void XUIWindow::LoadChilds(TiXmlElement* doc)
{
	for(TiXmlElement* element = doc->FirstChildElement(); element; element = element->NextSiblingElement() )
	{
		XUIComponent* object = 0;

		if ( strcmp(element->Value(), TEXT) == 0)
		{
			object = new XUIText;
		}
		else if ( strcmp(element->Value(), PUSHBUTTON) == 0)
		{
			object = new XUIPushButton;
		}
		else if ( strcmp(element->Value(), EDITBOX) == 0)
		{
			object = new XUIEditBox;
		}
		else if ( strcmp(element->Value(), CHECKBOX) == 0)
		{
			object = new XUICheckBox;
		}
		else if ( strcmp(element->Value(), RADIOBUTTON) == 0)
		{
			object = new XUIRadioButton;
		}
		else if ( strcmp(element->Value(), RADIOGROUP) == 0)
		{
			object = new XUIRadioGroup;
		}
		else if ( strcmp(element->Value(), MENUBAR) == 0)
		{
			object = new XUIMenuBar;
			withmenu = true;
		}
		else if ( strcmp(element->Value(), MENUPOPUP) == 0)
		{
			object = new XUIPopupMenu;
		}
		else if ( strcmp(element->Value(), SLIDEBAR) == 0)
		{
			object = new XUISlideBar;
		}
		else if ( strcmp(element->Value(), LISTBOX) == 0)
		{
			object = new XUIListBox;
		}
		else
			assert(!"Nedefinovany typ!!!");

		object->Load(element);
		selectmode |= object->Selected();
		AddComponent( object );
		delete object;
	}
}

void XUIWindow::SetHandlerName(const char* name)
{
	strncpy(handler, name, sizeof(handler));
	handler[sizeof(handler)-1] = 0;
}

void XUIWindow::Show(XUIComponent*)
{
	const char* name = label ? label : id;

	if (parent == 0)
	{
		int flag = 0;

		if (title) flag |= WTITLED;
		if (frame) flag |= WFRAMED;
		if (modal) flag |= WMODAL;
		if (close_on_escape) flag |= WESCAPE;
		if (center) flag |= WCENTRED;
		if (persistent) flag |= WUSELAST;
		if (resize) flag |= WSIZEABLE;
		if (statusbar) flag |= WSTATUSBAR;
		if (glcontext) flag |= WGLCONTEXT;
		if (sticky) flag |= WUNMOVED;
		if (!focus) flag |= WLASTFOCUS;
		if (!pictograms) flag |= WNOPICTO;
		if (selectmode) flag |= WUSESELECTEDCONTROL;
		if (withmenu) flag |= WMENU;

		parent = new FGWindow(&parent, shape.x, shape.y, shape.w, shape.h, name, GlobalHandler, foreground, background, flag, handler);

		XUIComposite::Show(this);
	}
}

void XUIWindow::GlobalHandler(FGEvent *event)
{
	const char* handler_name = (const char*)event->wnd->GetUserData();

//	if (withlistbox)
	{

	}
	CallClosure(handler_name, event);
}

// -----------------------------------------------------------------------------

/**
	Creates an object with empty list of widgets.
	You must create this one prior to work with XUI files.
	There is no reason to build more than one object of this type at the time.
	This object registers some built-in signal handlers when invoked first time.
	- "__CloseWindow" - you can use this signal name to close window itself.
	- "__CloseApplication" - you can use this signal name to terminate application. 
*/
XUIBuilder::XUIBuilder()
{
	static bool firstcall = true;

	if (firstcall)
	{
		firstcall = false;
		RegisterOnClickSignal("__CloseWindow", CloseWindow);
		RegisterOnClickSignal("__CloseApplication", CloseApplication);
	}
}

/**
	Loads the widget from XUI document (or its part only) into XUIBuilder object.
	If object already contains some data, the new one are merged at the end.
	Load & Save XUI document are fully reversible.
	@param fname file name
	@param widget if nonzero, loads widget with this ID from XUI file only
	@return true if success
*/
bool XUIBuilder::LoadGUI(const char* fname, const char* widget)
{
	TiXmlDocument* document = new TiXmlDocument( fname );
	bool loadOkay = document->LoadFile();

	if ( !loadOkay || document->RootElement() == 0)
	{
		delete document;
		return false;
	}

	// load and translate
	TiXmlElement* root = document->RootElement();
	bool retval = false;

	// translate & save
	for(TiXmlElement* element = root->FirstChildElement( WIDGET );
		element;
		element = element->NextSiblingElement( WIDGET ) )
	{
		XUIWindow w;
		w.Load(element);
		if (widget==0 || strcmp(widget, w.GetId())==0 )
		{
			retval = true;
			AddWidget(w);
		}
	}
	if (widget) printf("XUIBuilder::LoadGui - unknown widget '%s'", widget);
	delete document;
	return retval;
}

/**
	Saves the whole object as XUI document.
	Load & Save XUI document are fully reversible.
	@param fname file name
	@return true if success
*/
bool XUIBuilder::SaveGUI(const char* fname)
{
	// Create an empty XUI file
	const char* demoStart =
		"<?xml version=\"1.0\" standalone='yes' >\n"
		"<xui>\n"
		"</xui>";

	TiXmlDocument tmpdoc( fname );
	tmpdoc.Parse( demoStart );

	if ( tmpdoc.Error() )
	{
		return false;
	}

	TiXmlElement* root = tmpdoc.RootElement();

	// translate & save
	for (WidgetIterator i = widgets.begin(); i != widgets.end(); i++)
	{
		i->Save(root);
	}

	return tmpdoc.SaveFile();
}

/**
	Shows one or all widgets.
	@param widget show widget with this ID only
*/
void XUIBuilder::Show(const char* widget)
{
	bool ok = false;
	for (WidgetIterator i = widgets.begin(); i != widgets.end(); i++)
	{
		XUIWindow &w = *i;
		if (widget==0 || strcmp(widget, w.GetId())==0)
		{
			w.Show(&w);
			ok = true;
		}
	}
	if (widget && ok==false) printf("XUIBuilder::Show - unknown widget '%s'", widget);
}

/**
	Find Widget with this ID.
	@param id unique id of Widget to find.
	@return Widget pointer (aka FGWindow pointer) if succes or 0.
*/
FGWindow* XUIBuilder::GetWidget(const char* id)
{
	for (WidgetIterator i = widgets.begin(); i != widgets.end(); i++)
	{
		XUIWindow &w = *i;
		if (w.GetId() && strcmp(id, w.GetId())==0)
			return w.parent;
	}
	return 0;
}

/**
	Insert a copy of Widget at the end of list of objects.
	@note There are not any test to multiple adding.

*/
void XUIBuilder::AddWidget(const XUIWindow& widget)
{
	widgets.push_back(widget);
}

/**
	Removes copy of Widget with this ID from the list of object.
	@note if Widget is shown then will be closed first (WindowClose() is called).
*/
bool XUIBuilder::DeleteWidget(const char* id)
{
	for (WidgetIterator i = widgets.begin(); i != widgets.end(); i++)
	{
		if (strcmp(i->GetId(), id)==0)
		{
			// close window if it is open
			if (i->parent)
				i->parent->WindowClose();
			widgets.erase(i);
			return true;
		}
	}
	printf("XUIBuilder::DeleteWidget - unknown '%s'", id);
	return false;
}

/**
	Find concrete Component (e.g. button) in concrete Widget.
	@param widget_id ID of Widget to contains Component
	@param component_id ID of Component to find
	@return 0 or component if found
*/
XUIComponent* XUIBuilder::FindObject(const char* widget_id, const char* component_id)
{
	for (WidgetIterator i = widgets.begin(); i != widgets.end(); i++)
	{
		if (strcmp(i->GetId(), widget_id)==0)
		{
			return i->FindObject(component_id);
		}
	}
	printf("XUIBuilder::FindObject - unknown %s:%s", widget_id, component_id);
	return 0;
}

/**
	Set some data to the concrete Component (e.g. string to EditBox).
	@param widget_id ID of Widget to contains Component
	@param component_id ID of Component to set.
	@param data string to set
	@param size size of string
	@return true if succes
*/
bool XUIBuilder::SetData(const char* widget_id, const char* component_id, const char data[], int size)
{
	XUIComponent* item = FindObject(widget_id, component_id);

	if (item)
		return item->SetData(data, size);

	return false;
}

/**
	Set some data to the concrete Component (e.g. integer to Slider).
	@param widget_id ID of Widget to contains Component
	@param component_id ID of Component to set.
	@param value value to set
	@return true if succes
*/
bool XUIBuilder::SetData(const char* widget_id, const char* component_id, const int val)
{
	XUIComponent* item = FindObject(widget_id, component_id);

	if (item)
		return item->SetData(val);

	return false;
}

/**
	Set some data to the concrete Component.
	@param widget_id ID of Widget to contains Component
	@param component_id ID of Component to set.
	@param value value to set
	@return true if succes
*/
bool XUIBuilder::SetData(const char* widget_id, const char* component_id, const double val)
{
	XUIComponent* item = FindObject(widget_id, component_id);

	if (item)
		return item->SetData(val);

	return false;
}

#ifdef FG_NAMESPACE
}

using namespace fgl;

#endif

// -----------------------------------------------------------------------------

#ifdef __REGRESS__

void NewWindow(CallBack, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	builder->Show("id1");
}

void RemoveWindow(CallBack, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
		another_window->WindowClose();
}

void MyTextEntry(CallBack, const char* text, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
	{
		another_window->printf(20,20,"Text input: '%s'", text);
	}
}

void MyIntegerEntry(CallBack, const int i, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
	{
		another_window->printf(20,60,"Integer input: '%d'", i);
	}
}

void MyDoubleEntry(CallBack, const double d, void* userdata)
{
	XUIBuilder* builder = static_cast<XUIBuilder*>(userdata);
	FGWindow* another_window = builder->GetWidget("id1");
	if (another_window)
	{
		another_window->printf(20,100,"Double input: '%f'", d);
	}
}

void MyWindowHandler(FGEvent*e, void* userdata)
{
	printf("MyHandler %d %s\n", e->GetType(), (char *)userdata);
}

static void GenerateXUI()
{
	XUIBuilder builder;
	XUIWindow wnd1;

	XUIWindow wnd2("id1", "Yet another window", 400,0,400,600);
		FGColor w((unsigned)FGColor::white);
		FGColor b((unsigned)FGColor::black);
		XUIText text("A text label", 32,32, b,w);
		wnd2.AddComponent(&text);
		wnd2.resize = true;
		wnd2.persistent = true;

	wnd1.SetId("id0");
	wnd1.shape = FGRect(0,0,400,600);
	wnd1.frame = 0;
	wnd1.SetLabel("This is my Window!");
	wnd1.SetHandlerName("MyHandler");

	XUIPushButton button("id3", "New Window", 'N');
		button.shape = FGRect(32,30,128,25);
		button.SetSignalName("CreateWindow");
		wnd1.AddComponent(&button);
	XUIPushButton button2("id4", "Close Window", 'C');
		button2.shape = FGRect(32,70,128,25);
		button2.SetSignalName("CloseWindow");
		wnd1.AddComponent(&button2);
	XUIPushButton button3("id5", "Close Application", 'A');
		button3.shape = FGRect(32,120,128,25);
		button3.SetSignalName("__CloseApplication");
		button3.selected = true;
		wnd1.AddComponent(&button3);

	// edit
	XUIEditBox entry1("id7", "Text Entry :", 'T');
		entry1.shape = FGRect(120,220,128,25);
		entry1.SetSignalName("MyTextEntry");
		entry1.scrambled = true;
		wnd1.AddComponent(&entry1);
	XUIEditBox entry2("id8", "Integer Entry :", 'T');
		entry2.shape = FGRect(120,250,128,25);
		entry2.SetSignalName("MyIntegerEntry");
		entry2.hexadecimal = true;
		wnd1.AddComponent(&entry2);
	XUIEditBox entry3("id9", "Double Entry :", 'T');
		entry3.shape = FGRect(120,280,128,25);
		entry3.SetSignalName("MyDoubleEntry");
		wnd1.AddComponent(&entry3);

	XUICheckBox chb1("id10", "CheckBox", 'C', 32,320);
		chb1.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&chb1);

	XUISlideBar slider("id15", 32, 350, -100, 100, 10);
		slider.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&slider);

	XUIListBox lb("id16", 32, 380, 100, 20, 5);
		lb.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&lb);

	const char* _label[]= { "prvy", "druhy", "treti", "stvrty", "piaty", 0 };
	int key[] = { 'p', 'd', 0, 0, 0 };

	XUIRadioGroup grp0("id12", _label, key, 200, 20, 40);
		grp0.SetSignalName("MyIntegerEntry");
		wnd1.AddComponent(&grp0);

	XUIMenuBar menu;
		XUIMenu file("id13", "File", 'F');
			XUIPopupMenu popup1("id13_1");
			popup1.shape.w = 200;
			XUIMenuItem item1("id13_1_1", "Item 1", 1);
			XUIMenuItem item2("id13_1_2", "Item 2", 2);
			XUIMenuItem item3("id13_1_3", "Quit", 'q');
			item3.selected = true;
			item3.SetSignalName("__CloseApplication");
			popup1.AddComponent(&item1);
			popup1.AddComponent(&item2);
			popup1.AddComponent(&item3);
			file.AddComponent(&popup1);
		menu.AddComponent(&file);
		XUIMenu edit("id14", "Edit", 'E');
			edit.SetSignalName("__CloseApplication");
		menu.AddComponent(&edit);
		wnd1.AddComponent(&menu);


	builder.AddWidget(wnd1);
	builder.AddWidget(wnd2);
	builder.SaveGUI("test1.xui");
}

/*
int main(int argc, char **argv)
{
	FGApp MyApp(3,argc,argv,0,APP_ENABLEALTX | APP_CFG);
	// -------------------------------------------------------------------------

	GenerateXUI();

	XUIBuilder* builder2 = new XUIBuilder;
	builder2->LoadGUI("test1.xui");
	builder2->SaveGUI("test2.xui");
	char test_string[20]="DS";
	int ii = 4231;
	double dd = 423.423;

	// I'm using an userdata ptr to export builder instance in this case.
	RegisterOnClickSignal("CreateWindow", NewWindow, builder2 );
	RegisterOnClickSignal("CloseWindow", RemoveWindow, builder2 );

	RegisterOnEnterSignal("MyTextEntry", MyTextEntry, builder2);
	builder2->SetData("id0", "id7", test_string, 10 );
	RegisterOnEnterSignal("MyIntegerEntry", MyIntegerEntry, builder2);
	builder2->SetData("id0", "id8", ii);
	RegisterOnEnterSignal("MyDoubleEntry", MyDoubleEntry, builder2);
	builder2->SetData("id0", "id9", dd);


	builder2->Show("id0");
	builder2->SetData("id0", "id12", 1 );
	builder2->SetData("id0", "id16", "One", 0 );
	builder2->SetData("id0", "id16", "Two", 1 );

	MyApp.Run();

	delete builder2;

	CallClosureDebug();

	return 0;
}
*/
#endif

