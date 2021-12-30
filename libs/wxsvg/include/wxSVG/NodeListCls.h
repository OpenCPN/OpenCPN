//////////////////////////////////////////////////////////////////////////////
// Name:        NodeListCls.h
// Purpose:
// Author:      Jonathan Hurtrel
// Created:     2007/06/08
// RCS-ID:      $Id: NodeListCls.h,v 1.2 2007/10/30 21:59:22 etisserant Exp $
// Copyright:   (c) 2005 Jonathan Hurtrel
// Licence:     wxWindows licence
//////////////////////////////////////////////////////////////////////////////

#ifndef wxSVG_NODE_LIST_CLS_H
#define wxSVG_NODE_LIST_CLS_H

#include "SVGElement.h"
#include "NodeList.h"

class wxNodeListCls
{
	protected:
		int list_size;
		wxNodeList my_list;

	public:
		wxNodeListCls(){ list_size=0;}
		wxNodeListCls(wxNodeList m_list){ my_list = m_list; list_size = my_list.size(); }
		~wxNodeListCls();

		wxSVGElement *GetFirstElement(){return my_list[0];}
		wxSVGElement *GetElement(int i){return my_list[i];}
		int GetSize(){return list_size;}
};

#endif //wxSVG_NODE_LIST_CLS_H
