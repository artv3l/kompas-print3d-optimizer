#ifndef BASE_EVENT_HPP
#define BASE_EVENT_HPP

#include <afxwin.h>

class AutomationBaseEvent : protected CCmdTarget {
public:
    AutomationBaseEvent(IUnknown *object, IID iidEvent);
    virtual ~AutomationBaseEvent();

protected:
    int advise(); // Подписаться на получение событий
    void unadvise(); // Отписаться от получения событий
    void disconnect();

    virtual void clear();

    DWORD m_dwCookie; // Идентификатор соединения
    IID m_iidEvent; // IID интерфейса событий
    IConnectionPoint *m_connectionPoint; // Соединение
    IUnknown *m_object; // Источник событий

    BEGIN_INTERFACE_PART(EventHandler, IDispatch)
        INIT_INTERFACE_PART(AutomationBaseEvent, EventHandler)
        STDMETHOD(GetTypeInfoCount)(unsigned int *pctinfo);
        STDMETHOD(GetTypeInfo)(unsigned int itinfo, LCID lcid, ITypeInfo **pptinfo);
        STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR *rgszNames, unsigned int cNames, LCID lcid, DISPID *rgdispid);
        STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid, LCID lcid, unsigned short wFlags, DISPPARAMS *lpDispparams,
            VARIANT *pvarResult, EXCEPINFO *pexcepinfo, unsigned int *puArgErr);
    END_INTERFACE_PART(EventHandler)

    DECLARE_INTERFACE_MAP()
    DECLARE_EVENTSINK_MAP()

};

#endif /* BASE_EVENT_HPP */
