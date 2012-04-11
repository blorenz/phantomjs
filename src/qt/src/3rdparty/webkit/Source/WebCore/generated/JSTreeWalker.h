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

#ifndef JSTreeWalker_h
#define JSTreeWalker_h

#include "JSDOMBinding.h"
#include <runtime/JSGlobalObject.h>
#include <runtime/JSObjectWithGlobalObject.h>
#include <runtime/ObjectPrototype.h>

namespace WebCore {

class TreeWalker;

class JSTreeWalker : public JSDOMWrapper {
    typedef JSDOMWrapper Base;
public:
    JSTreeWalker(JSC::Structure*, JSDOMGlobalObject*, PassRefPtr<TreeWalker>);
    static JSC::JSObject* createPrototype(JSC::ExecState*, JSC::JSGlobalObject*);
    virtual bool getOwnPropertySlot(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertySlot&);
    virtual bool getOwnPropertyDescriptor(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::PropertyDescriptor&);
    virtual void put(JSC::ExecState*, const JSC::Identifier& propertyName, JSC::JSValue, JSC::PutPropertySlot&);
    static const JSC::ClassInfo s_info;

    static JSC::Structure* createStructure(JSC::JSGlobalData& globalData, JSC::JSValue prototype)
    {
        return JSC::Structure::create(globalData, prototype, JSC::TypeInfo(JSC::ObjectType, StructureFlags), AnonymousSlotCount, &s_info);
    }

    virtual void visitChildren(JSC::SlotVisitor&);

    static JSC::JSValue getConstructor(JSC::ExecState*, JSC::JSGlobalObject*);
    TreeWalker* impl() const { return m_impl.get(); }

private:
    RefPtr<TreeWalker> m_impl;
protected:
    static const unsigned StructureFlags = JSC::OverridesGetOwnPropertySlot | JSC::OverridesVisitChildren | Base::StructureFlags;
};

JSC::JSValue toJS(JSC::ExecState*, JSDOMGlobalObject*, TreeWalker*);
TreeWalker* toTreeWalker(JSC::JSValue);

class JSTreeWalkerPrototype : public JSC::JSObjectWithGlobalObject {
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
    JSTreeWalkerPrototype(JSC::JSGlobalData& globalData, JSC::JSGlobalObject* globalObject, JSC::Structure* structure) : JSC::JSObjectWithGlobalObject(globalData, globalObject, structure) { }
protected:
    static const unsigned StructureFlags = JSC::OverridesGetOwnPropertySlot | JSC::OverridesVisitChildren | Base::StructureFlags;
};

// Functions

JSC::EncodedJSValue JSC_HOST_CALL jsTreeWalkerPrototypeFunctionParentNode(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsTreeWalkerPrototypeFunctionFirstChild(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsTreeWalkerPrototypeFunctionLastChild(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsTreeWalkerPrototypeFunctionPreviousSibling(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsTreeWalkerPrototypeFunctionNextSibling(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsTreeWalkerPrototypeFunctionPreviousNode(JSC::ExecState*);
JSC::EncodedJSValue JSC_HOST_CALL jsTreeWalkerPrototypeFunctionNextNode(JSC::ExecState*);
// Attributes

JSC::JSValue jsTreeWalkerRoot(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsTreeWalkerWhatToShow(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsTreeWalkerFilter(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsTreeWalkerExpandEntityReferences(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
JSC::JSValue jsTreeWalkerCurrentNode(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);
void setJSTreeWalkerCurrentNode(JSC::ExecState*, JSC::JSObject*, JSC::JSValue);
JSC::JSValue jsTreeWalkerConstructor(JSC::ExecState*, JSC::JSValue, const JSC::Identifier&);

} // namespace WebCore

#endif
