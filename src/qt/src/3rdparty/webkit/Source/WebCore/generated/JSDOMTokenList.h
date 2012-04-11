/*
    This file is part of the WebKit open source project.
    This file has been generated by generate-bindings.pl. DO NOT MODIFY!

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef JSDOMTokenList_h
#define JSDOMTokenList_h

#include "JSDOMBinding.h"
#include <runtime/JSGlobalObject.h>
#include <runtime/JSObjectWithGlobalObject.h>
#include <runtime/ObjectPrototype.h>

namespace WebCore {

class DOMTokenList;

class JSDOMTokenList : public JSDOMWrapper {
    typedef JSDOMWrapper Base;
public:
    JSDOMTokenList(JSC::Structure*, JSDOMGlobalObject*, PassRefPtr<DOMTokenList>);
    static JSC::JSObject* createPrototype(JSC::ExecState*, JSC::JSGlobalObject*);
    virtual bool getOwnPropertySlot(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertySlot&);
    virtual bool getOwnPropertyDescriptor(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertyDescriptor&);
    virtual bool getOwnPropertySlot(JSC::ExecState*, unsigned propertyName, JSC::PropertySlot&);
    static const JSC::ClassInfo s_info;

    static JSC::Structure* createStructure(JSC::JSGlobalData& globalData, JSC::JSValue prototype)
    {
        return JSC::Structure::create(globalData, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), AnonymousSlotCount, &s_info);
    }

    virtual void getOwnPropertyNames(JSC::ExecState*, JSC::PropertyNameArray&, JSC::EnumerationMode mode = JSC::ExcludeDontEnumProperties);
    static JSC::JSValue getConstructor(JSC::ExecState*, JSC::JSGlobalObject*);
    DOMTokenList* impl() const { return m_impl.get(); }

private:
    RefPtr<DOMTokenList> m_impl;
protected:
    static const unsigned StructureFlags = JSC::OverridesGetPropertyNames | JSC::OverridesGetOwnPropertySlot | Base::StructureFlags;
    static JSC::JSValue indexGetter(JSC::ExecState*, JSC::JSValue, unsigned);
};

class JSDOMTokenListOwner : public JSC::WeakHandleOwner {
    virtual bool isReachableFromOpaqueRoots(JSC::Handle<JSC::Unknown>, void* context, JSC::SlotVisitor&);
    virtual void finalize(JSC::Handle<JSC::Unknown>, void* context);
};

inline JSC::WeakHandleOwner* wrapperOwner(DOMWrapperWorld*, DOMTokenList*)
{
    DEFINE_STATIC_LOCAL(JSDOMTokenListOwner, jsDOMTokenListOwner, ());
    return &jsDOMTokenListOwner;
}

inline void* wrapperContext(DOMWrapperWorld* world, DOMTokenList*)
{
    return world;
}

JSC::JSValue toJS(JSC::ExecState*, JSDOMGlobalObject*, DOMTokenList*);
DOMTokenList* toDOMTokenList(JSC::JSValue);

class JSDOMTokenListPrototype : public JSC::JSObjectWithGlobalObject {
    typedef JSC::JSObjectWithGlobalObject Base;
public:
    static JSC::JSObject* self(JSC::ExecState*, JSC::JSGlobalObject*);
    static const JSC::ClassInfo s_info;
    virtual bool getOwnPropertySlot(JSC::ExecState*, const JSC::Identifier&, JSC::PropertySlot&);
    virtual bool getOwnPropertyDescriptor(JSC::ExecState*, const JSC::Identifier&, JSC::PropertyDescriptor&);
    static JSC::Structure* createStructure(JSC::JSGlobalData& globalData, JSC::JSValue prototype)
    {
        return JSC::Structure::create(globalData, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), AnonymousSlotCount, &s_info);
    }
    JSDOMTokenListPrototype(JSC::JSGlobalData& globalData, JSC::JSGlobalObject* globalObject, JSC::Structure* structure) : JSC::JSObjectWithGlobalObject(globalData, globalObject, structure) { }
protected:
    static const unsigned StructureFlags = JSC::OverridesGetOwnPropertySlot | Base::StructureFlags;
};

// Functions

JSC::EncodedJSValue JSC_HOST_CALL jsDOMTokenListPrototypeFunctionItem(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsDOMTokenListPrototypeFunctionContains(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsDOMTokenListPrototypeFunctionAdd(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsDOMTokenListPrototypeFunctionRemove(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsDOMTokenListPrototypeFunctionToggle(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsDOMTokenListPrototypeFunctionToString(JSC::ExecState*);
// Attributes

JSC::JSValue jsDOMTokenListLength(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsDOMTokenListConstructor(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);

} // namespace WebCore

#endif
