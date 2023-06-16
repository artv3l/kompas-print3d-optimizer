#include "stdafx.h"

#include "apiutil/AutomationBaseEvent.hpp"

#include <Unknwn.h>
#include <guiddef.h>
#include <afxwin.h>
#include <cassert>
#include <afxpriv2.h>

CObList &AutomationBaseEvent::eventList_ = *(new CObList());

AutomationBaseEvent::AutomationBaseEvent(IUnknown *object, IID iidEvent) :
        CCmdTarget(),
        dwCookie_(0), object_(object), iidEvent_(iidEvent), connectionPoint_(nullptr) {
    if (object_) {
        object_->AddRef();
    }
    assert(!IsEqualIID(iidEvent_, GUID_NULL));
    eventList_.AddTail(this);
}

AutomationBaseEvent::~AutomationBaseEvent() {
    removeThis();
    if (object_) {
        object_->Release();
        object_ = nullptr;
    }
}

void AutomationBaseEvent::terminateEvents() {
    while (!eventList_.IsEmpty()) {
        AutomationBaseEvent *headEvent = static_cast<AutomationBaseEvent *>(eventList_.RemoveHead());
        headEvent->disconnect();
    }
}

void AutomationBaseEvent::terminateEvents(IID iid) {
    terminateEvents(iid, nullptr);
}

void AutomationBaseEvent::terminateEvents(IID iid, IUnknown *object) {
    INT_PTR count = eventList_.GetCount();
    for (INT_PTR i = 0; i < count; i++) {
        CObject *obj = eventList_.GetAt(eventList_.FindIndex(i));
        AutomationBaseEvent *event = static_cast<AutomationBaseEvent *>(obj);
        if (event && ((!object) || event->object_ == object) &&
            (IsEqualIID(iid, GUID_NULL) || IsEqualIID(iid, event->iidEvent_))) {
            event->disconnect();
        }
    }
}

void AutomationBaseEvent::destroyList() {
    if (&eventList_) {
        delete &eventList_;
    }
}

bool AutomationBaseEvent::findEvent(IID iid, IUnknown *object) {
    INT_PTR count = eventList_.GetCount();
    for (INT_PTR i = 0; i < count; i++) {
        CObject *obj = eventList_.GetAt(eventList_.FindIndex(i));
        AutomationBaseEvent *event = static_cast<AutomationBaseEvent *>(obj);
        if (event && ((!object) || event->object_ == object) &&
            (IsEqualIID(iid, GUID_NULL) || IsEqualIID(iid, event->iidEvent_))) {
            return true;
        }
    }
    return false;
}

int AutomationBaseEvent::advise() {
    assert(dwCookie_ == 0);
    if (object_) {
        IConnectionPointContainer *connectionPointContainer = nullptr;
        if (SUCCEEDED(object_->QueryInterface(IID_IConnectionPointContainer, (LPVOID *)&connectionPointContainer))) {
            if (connectionPointContainer && SUCCEEDED(connectionPointContainer->FindConnectionPoint(iidEvent_, &connectionPoint_))) {
                assert(connectionPoint_ != nullptr);
                connectionPoint_->Advise(&m_xEventHandler, &dwCookie_);
            }
            connectionPointContainer->Release();
        }
    }
    if (!dwCookie_) {
        delete this;
        return 0;
    }
    return dwCookie_;
}

void AutomationBaseEvent::unadvise() {
    if (connectionPoint_) {
        connectionPoint_->Unadvise(dwCookie_);
        connectionPoint_->Release();
        connectionPoint_ = nullptr;
    }
    dwCookie_ = 0;
}

void AutomationBaseEvent::removeThis() {
    POSITION position = eventList_.Find(this);
    if (position) {
        eventList_.RemoveAt(position);
        unadvise();
    }
}

void AutomationBaseEvent::disconnect() {
    unadvise();
    clear();
    ExternalRelease();
}

void AutomationBaseEvent::clear() {
    if (object_) {
        object_->Release();
        object_ = nullptr;
    }
    iidEvent_ = GUID_NULL;
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
    if ((iid == IID_IUnknown) || (iid == IID_IDispatch) || (iid == pThis->iidEvent_)) {
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
