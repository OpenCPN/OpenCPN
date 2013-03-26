#ifndef __SELECTITEM_H__
#define __SELECTITEM_H__

#include <wx/list.h>

class SelectItem
{
public:
      SelectItem();
      ~SelectItem();

      int   GetUserData(void);
      void  SetUserData(int data);

      float m_slat;
      float m_slon;
      float m_slat2;
      float m_slon2;
      int   m_seltype;
      bool  m_bIsSelected;
      void  *m_pData1;
      void  *m_pData2;
      void  *m_pData3;
      int   m_Data4;
};

WX_DECLARE_LIST(SelectItem, SelectableItemList);// establish class as list member

#endif
