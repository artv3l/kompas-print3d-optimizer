#ifndef BASE_EVENT_HPP
#define BASE_EVENT_HPP

#include "stdafx.h"

#include <afxwin.h>

class AutomationBaseEvent : public CCmdTarget {
public:
    AutomationBaseEvent(IUnknown *object, IID iidEvent);
    virtual ~AutomationBaseEvent();

    static void terminateEvents(); // Отписать все события
    static void terminateEvents(IID iid);
    static void terminateEvents(IID iid, IUnknown *object);
    static void destroyList();
    static bool findEvent(IID iid, IUnknown *object);

    int advise(); // Подписаться на получение событий
    void unadvise(); // Отписаться от получения событий
    void removeThis();
    void disconnect();

    virtual void clear();

protected:
    static CObList &eventList_; // Список обработчиков событий
    DWORD dwCookie_; // Идентификатор соединения
    IID iidEvent_; // IID интерфейса событий
    IConnectionPoint *connectionPoint_; // Соединение
    IUnknown *object_; // Источник событий

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
