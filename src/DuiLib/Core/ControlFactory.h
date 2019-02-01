#pragma once
#include <map>
namespace DuiLib 
{
	typedef CControlUI* (*CreateClass)();
	typedef std::map<base::String, CreateClass> MAP_DUI_CTRATECLASS;

	class UILIB_API CControlFactory
	{
	public:
		CControlUI* CreateControl(base::String strClassName);
		void RegistControl(base::String strClassName, CreateClass pFunc);

		static CControlFactory* GetInstance();
		static void Release();

	private:	
		CControlFactory();
		virtual ~CControlFactory();

	private:
		MAP_DUI_CTRATECLASS m_mapControl;
        static CControlFactory* m_pThis;
	};

#define DECLARE_DUICONTROL(class_name)\
public:\
	static CControlUI* CreateControl();

#define IMPLEMENT_DUICONTROL(class_name)\
	CControlUI* class_name::CreateControl()\
	{ return new class_name; }

#define REGIST_DUICONTROL(class_name)\
	CControlFactory::GetInstance()->RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);

#define INNER_REGISTER_DUICONTROL(class_name)\
	RegistControl(_T(#class_name), (CreateClass)class_name::CreateControl);
}