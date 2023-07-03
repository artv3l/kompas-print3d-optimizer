#include "stdafx.h"

#include "apiutil/AutomationBaseEvent.hpp"

#include <Unknwn.h>
#include <guiddef.h>
#include <afxwin.h>
#include <cassert>
#include <afxpriv2.h>

CObList AutomationBaseEvent::m_eventList;

AutomationBaseEvent::AutomationBaseEvent(IUnknown *object, IID iidEvent) :
        CCmdTarget(),
        m_dwCookie(0), m_object(object), m_iidEvent(iidEvent), m_connectionPoint(nullptr) {
    if (m_object) {
        m_object->AddRef();
    }
    assert(!IsEqualIID(m_iidEvent, GUID_NULL));
    m_eventList.AddTail(this);
}

AutomationBaseEvent::~AutomationBaseEvent() {
    removeThis();
    if (m_object) {
        m_object->Release();
        m_object = nullptr;
    }
}

void AutomationBaseEvent::terminateEvents() {
    while (!m_eventList.IsEmpty()) {
        AutomationBaseEvent *headEvent = static_cast<AutomationBaseEvent *>(m_eventList.RemoveHead());
        headEvent->disconnect();
    }
}

void AutomationBaseEvent::terminateEvents(IID iid) {
    terminateEvents(iid, nullptr);
}

void AutomationBaseEvent::terminateEvents(IID iid, IUnknown *object) {
    INT_PTR count = m_eventList.GetCount();
    for (INT_PTR i = 0; i < count; i++) {
        CObject *obj = m_eventList.GetAt(m_eventList.FindIndex(i));
        AutomationBaseEvent *event = static_cast<AutomationBaseEvent *>(obj);
        if (event && ((!object) || event->m_object == object) &&
            (IsEqualIID(iid, GUID_NULL) || IsEqualIID(iid, event->m_iidEvent))) {
            event->disconnect();
        }
    }
}

bool AutomationBaseEvent::findEvent(IID iid, IUnknown *object) {
    INT_PTR count = m_eventList.GetCount();
    for (INT_PTR i = 0; i < count; i++) {
        CObject *obj = m_eventList.GetAt(m_eventList.FindIndex(i));
        AutomationBaseEvent *event = static_cast<AutomationBaseEvent *>(obj);
        if (event && ((!object) || event->m_object == object) &&
            (IsEqualIID(iid, GUID_NULL) || IsEqualIID(iid, event->m_iidEvent))) {
            return true;
        }
    }
    return false;
}

int AutomationBaseEvent::advise() {
    assert(m_dwCookie == 0);
    if (m_object) {
        IConnectionPointContainer *connectionPointContainer = nullptr;
        if (SUCCEEDED(m_object->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&connectionPointContainer))) {
            if (connectionPointContainer && SUCCEEDED(connectionPointContainer->FindConnectionPoint(m_iidEvent, &m_connectionPoint))) {
                assert(m_connectionPoint != nullptr);
                m_connectionPoint->Advise(&m_xEventHandler, &m_dwCookie);
            }
            connectionPointContainer->Release();
        }
    }
    if (!m_dwCookie) {
        delete this;
        return 0;
    }
    return m_dwCookie;
}

void AutomationBaseEvent::unadvise() {
    if (m_connectionPoint) {
        m_connectionPoint->Unadvise(m_dwCookie);
        m_connectionPoint->Release();
        m_connectionPoint = nullptr;
    }
    m_dwCookie = 0;
}

void AutomationBaseEvent::removeThis() {
    POSITION position = m_eventList.Find(this);
    if (position) {
        m_eventList.RemoveAt(position);
        unadvise();
    }
}

void AutomationBaseEvent::disconnect() {
    unadvise();
    clear();
    ExternalRelease();
}

void AutomationBaseEvent::clear() {
    if (m_object) {
        m_object->Release();
        m_object = nullptr;
    }
    m_iidEvent = GUID_NULL;
}

// Карта интерфейса
BEGIN_INTERFACE_MAP(AutomationBaseEvent, CCmdTarget)
END_INTERFACE_MAP()

// Карта сообщений интерфейса
BEGIN_EVENTSINK_MAP(AutomationBaseEvent, CCmdTarget)
END_EVENTSINK_MAP()

STDMETHODIMP_(ULONG) AutomationBaseEvent::XEventHandler::AddRef() {
    METHOD_PROLOGUE(AutomationBaseEvent, EventHandler);
    return (ULONG)pThis->ExternalAddRef();
}

STDMETHODIMP AutomationBaseEvent::XEventHandler::GetTypeInfoCount(unsigned int *pctinfo) {
    METHOD_PROLOGUE(AutomationBaseEvent, EventHandler);
    *pctinfo = 0;
    return NOERROR;
}

STDMETHODIMP AutomationBaseEvent::XEventHandler::GetTypeInfo(unsigned int itinfo, LCID lcid, ITypeInfo **pptinfo) {
    METHOD_PROLOGUE(AutomationBaseEvent, EventHandler);
    ASSERT_VALID(pThis);
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP AutomationBaseEvent::XEventHandler::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, unsigned int cNames, LCID lcid, DISPID *rgdispid) {
    METHOD_PROLOGUE(AutomationBaseEvent, EventHandler);
    ASSERT_VALID(pThis);
    return ResultFromScode(E_NOTIMPL);
}

STDMETHODIMP AutomationBaseEvent::XEventHandler::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, unsigned short wFlags, DISPPARAMS *lpDispparams,
        VARIANT *pvarResult, EXCEPINFO *pexcepinfo, unsigned int *puArgErr) {
    METHOD_PROLOGUE(AutomationBaseEvent, EventHandler);
    ASSERT_VALID(pThis);

    // Класс параметров сообщения
    AFX_EVENT event(AFX_EVENT::event, dispidMember, lpDispparams, pexcepinfo, puArgErr);
    bool eventHandled = true;
    if (pThis->GetEventSinkEntry(1, &event) != nullptr) {
        eventHandled = pThis->OnEvent(1, &event, nullptr);
    }
    if (pvarResult != nullptr) {
        VariantClear(pvarResult);
        pvarResult->vt = VT_BOOL;
        V_BOOL(pvarResult) = eventHandled;
    }
    return event.m_hResult;
}

STDMETHODIMP AutomationBaseEvent::XEventHandler::QueryInterface(REFIID iid, LPVOID *ppvObj) {
    METHOD_PROLOGUE(AutomationBaseEvent, EventHandler);

    *ppvObj = nullptr;
    if ((iid == IID_IUnknown) || (iid == IID_IDispatch) || (iid == pThis->m_iidEvent)) {
        *ppvObj = this;
    }
    if (*ppvObj) {
        ((IUnknown *)*ppvObj)->AddRef();
        return NOERROR;
    }
    return static_cast<HRESULT>(pThis->ExternalQueryInterface(&iid, ppvObj));
}

STDMETHODIMP_(ULONG) AutomationBaseEvent::XEventHandler::Release() {
    METHOD_PROLOGUE(AutomationBaseEvent, EventHandler);
    return static_cast<ULONG>(pThis->ExternalRelease());
}
