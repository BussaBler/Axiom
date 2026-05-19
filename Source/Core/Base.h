#pragma once

#if defined(AX_REFLECTION_PARSER)
#define AX_COMPONENT() [[clang::annotate("Axiom::Component")]]
#define AX_PROPERTY(...) [[clang::annotate("Axiom::Property|" #__VA_ARGS__)]]
#else
#define AX_COMPONENT()
#define AX_PROPERTY(...)
#endif