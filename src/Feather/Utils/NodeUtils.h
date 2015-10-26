#pragma once

#ifdef __cplusplus
extern "C" {
#endif




StringRef Feather_BackendCode_getCode(const Node* node);
EvalMode Feather_BackendCode_getEvalMode(Node* node);

void Feather_ChangeMode_setChild(Node* node, Node* child);
EvalMode Feather_ChangeMode_getEvalMode(Node* node);

void Feather_Function_addParameter(Node* node, Node* parameter, bool first = false);
void Feather_Function_setResultType(Node* node, Node* resultType);
void Feather_Function_setBody(Node* node, Node* body);
size_t Feather_Function_numParameters(Node* node);
Node* Feather_Function_getParameter(Node* node, size_t idx);
TypeRef Feather_Function_resultType(Node* node);
Node* Feather_Function_body(Node* node);
CallConvention Feather_Function_callConvention(Node* node);

#ifdef __cplusplus
}
#endif

template <typename T>
Node* Feather_mkCtValueT(const Location& loc, TypeRef type, T* dataVal)
{
    const char* p = reinterpret_cast<const char*>(dataVal);
    StringRef dataStr = {p, p+sizeof(*dataVal)};
    return Feather_mkCtValue(loc, type, dataStr);
}
