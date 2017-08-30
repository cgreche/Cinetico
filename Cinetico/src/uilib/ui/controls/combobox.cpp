
//Combo Box
// File name: combobox.cpp
// Last Edit: 20/12/2013 14:17 (UTC-3)
// Author: CGR

#include "..\\uibase.h"



ComboBox::ComboBox():Control(*new OSDComboBox(*this))
{
	m_onSelect = NULL;
	m_selection = 0xffffffff;

	osdRef().create();
	setDefaultFont();
}


Size ComboBox::getAutoSize()
{
	return osdRef().getAutoSize();
}

u32 ComboBox::fastinsertItem(const string &text)
{
	return insertItem(*new ComboBoxItem(text,Color(0,0,0),FontDesc("Arial",10,FONT_BOLD),NULL));
}

u32 ComboBox::insertItem(ComboBoxItem &item)
{
	m_items.push_back(&item);
	return osdRef().insertItem(item);
}

void ComboBox::removeItem(u32 index)
{
	if(index >= m_items.size())
		return;

	delete m_items[index];
	m_items.erase(m_items.begin()+index);
	osdRef().removeItem(index);
}

void ComboBox::removeItem(ComboBoxItem& item)
{
	u32 i;
	for(i = 0; i < m_items.size(); ++i) {
		if(m_items[i] == &item) {
			removeItem(i);
			return;
		}
	}
}

void ComboBox::reset()
{
	if(m_items.size() > 0) {
		std::vector<ComboBoxItem*>::iterator it;
		it = m_items.begin();
		while(it != m_items.end()) {
			delete *it;
			++it;
		}
		m_items.clear();
	}
	osdRef().reset();
}

void ComboBox::sort()
{
}

u32 ComboBox::setSelection(u32 row)
{
	m_selection = row;
	osdRef().setSelection(row);
	return m_selection;
}

u32 ComboBox::selection()
{
	return m_selection;
}

ComboBoxItem *ComboBox::item()
{
	return item(selection());
}

ComboBoxItem *ComboBox::item(u32 row)
{
	if(row+1 > m_items.size())
		return NULL;
	return m_items[row];
}


